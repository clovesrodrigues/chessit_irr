#!/usr/bin/env python3
"""Train and export the ChessIt neural chess model to ONNX.

This script trains a compact PyTorch model from PGN games and exports it with
an interface that the C++ ONNXAIManager can consume later:

    input  name: board        shape: [batch, 13, 8, 8] float32
    output name: move_logits  shape: [batch, 4096] float32
    output name: value        shape: [batch, 1] float32

The 4096 move policy uses a simple from-to mapping:

    move_index = from_square * 64 + to_square

where a1=0, b1=1, ..., h8=63. Promotion choice is intentionally simplified in
this first model contract; the game should filter the returned policy by legal
moves and can promote to queen for the initial integration.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Iterator

import chess
import chess.pgn
import numpy as np
import onnx
import onnxruntime as ort
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import DataLoader, Dataset
from tqdm import tqdm

BOARD_CHANNELS = 13
BOARD_SIZE = 8
MOVE_POLICY_SIZE = 64 * 64
DEFAULT_MAX_POSITIONS = 50_000
DEFAULT_BATCH_SIZE = 64
DEFAULT_EPOCHS = 5
DEFAULT_LEARNING_RATE = 1e-3

PIECE_TO_PLANE = {
    (chess.PAWN, chess.WHITE): 0,
    (chess.KNIGHT, chess.WHITE): 1,
    (chess.BISHOP, chess.WHITE): 2,
    (chess.ROOK, chess.WHITE): 3,
    (chess.QUEEN, chess.WHITE): 4,
    (chess.KING, chess.WHITE): 5,
    (chess.PAWN, chess.BLACK): 6,
    (chess.KNIGHT, chess.BLACK): 7,
    (chess.BISHOP, chess.BLACK): 8,
    (chess.ROOK, chess.BLACK): 9,
    (chess.QUEEN, chess.BLACK): 10,
    (chess.KING, chess.BLACK): 11,
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Train ChessIt's PyTorch chess model from PGN files and export chessit_ai.onnx."
    )
    parser.add_argument(
        "--pgn",
        default="training/data/games.pgn",
        help="Path to a PGN file containing training games.",
    )
    parser.add_argument(
        "--output",
        default="models/chessit_ai.onnx",
        help="Output path for the exported ONNX model.",
    )
    parser.add_argument(
        "--checkpoint",
        default="models/chessit_ai.pt",
        help="Output path for the PyTorch checkpoint.",
    )
    parser.add_argument(
        "--max-positions",
        type=int,
        default=DEFAULT_MAX_POSITIONS,
        help="Maximum board positions to read from the PGN file.",
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=DEFAULT_BATCH_SIZE,
        help="Training batch size.",
    )
    parser.add_argument(
        "--epochs",
        type=int,
        default=DEFAULT_EPOCHS,
        help="Number of training epochs.",
    )
    parser.add_argument(
        "--learning-rate",
        type=float,
        default=DEFAULT_LEARNING_RATE,
        help="Adam optimizer learning rate.",
    )
    parser.add_argument(
        "--device",
        choices=("auto", "cpu", "cuda"),
        default="auto",
        help="Training device. Use auto to select CUDA when available.",
    )
    parser.add_argument(
        "--skip-training",
        action="store_true",
        help="Export an initialized model without training. Useful for validating the ONNX pipeline.",
    )
    return parser.parse_args()


def result_to_white_value(result: str) -> float:
    if result == "1-0":
        return 1.0
    if result == "0-1":
        return -1.0
    return 0.0


def select_device(device_arg: str) -> torch.device:
    if device_arg == "auto":
        return torch.device("cuda" if torch.cuda.is_available() else "cpu")
    if device_arg == "cuda" and not torch.cuda.is_available():
        raise RuntimeError("CUDA was requested, but torch.cuda.is_available() is false.")
    return torch.device(device_arg)


def encode_board(board: chess.Board) -> np.ndarray:
    """Encode a python-chess board as [13, 8, 8] float32 planes.

    Planes 0-5 are white pawn/knight/bishop/rook/queen/king.
    Planes 6-11 are black pawn/knight/bishop/rook/queen/king.
    Plane 12 is the side to move: +1 for white, -1 for black.
    """
    tensor = np.zeros((BOARD_CHANNELS, BOARD_SIZE, BOARD_SIZE), dtype=np.float32)

    for square, piece in board.piece_map().items():
        plane = PIECE_TO_PLANE[(piece.piece_type, piece.color)]
        row = chess.square_rank(square)
        col = chess.square_file(square)
        tensor[plane, row, col] = 1.0

    tensor[12, :, :] = 1.0 if board.turn == chess.WHITE else -1.0
    return tensor


def move_to_index(move: chess.Move) -> int:
    return move.from_square * 64 + move.to_square


def index_to_move(index: int) -> chess.Move:
    from_square = index // 64
    to_square = index % 64
    return chess.Move(from_square, to_square)


def iter_training_samples(pgn_path: Path, max_positions: int) -> Iterator[tuple[np.ndarray, int, np.float32]]:
    with pgn_path.open("r", encoding="utf-8", errors="ignore") as pgn_file:
        produced = 0
        pbar = tqdm(total=max_positions, desc="Reading PGN positions")

        while produced < max_positions:
            game = chess.pgn.read_game(pgn_file)
            if game is None:
                break

            result = game.headers.get("Result", "*")
            if result not in {"1-0", "0-1", "1/2-1/2"}:
                continue

            white_value = result_to_white_value(result)
            board = game.board()

            for move in game.mainline_moves():
                if produced >= max_positions:
                    break

                if move not in board.legal_moves:
                    break

                x = encode_board(board)
                y_policy = move_to_index(move)
                y_value = np.float32(white_value if board.turn == chess.WHITE else -white_value)
                yield x, y_policy, y_value

                board.push(move)
                produced += 1
                pbar.update(1)

        pbar.close()


class ChessPGNDataset(Dataset):
    def __init__(self, pgn_path: str | Path, max_positions: int):
        self.pgn_path = Path(pgn_path)
        if not self.pgn_path.exists():
            raise FileNotFoundError(f"PGN file not found: {self.pgn_path}")

        self.samples = list(iter_training_samples(self.pgn_path, max_positions))
        if not self.samples:
            raise RuntimeError(f"No usable training positions found in: {self.pgn_path}")
        print(f"Loaded {len(self.samples)} training positions from {self.pgn_path}")

    def __len__(self) -> int:
        return len(self.samples)

    def __getitem__(self, index: int) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
        board_tensor, policy_target, value_target = self.samples[index]
        return (
            torch.from_numpy(board_tensor),
            torch.tensor(policy_target, dtype=torch.long),
            torch.tensor([value_target], dtype=torch.float32),
        )


class ChessItNet(nn.Module):
    def __init__(self) -> None:
        super().__init__()
        self.body = nn.Sequential(
            nn.Conv2d(BOARD_CHANNELS, 64, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.Conv2d(64, 64, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.Conv2d(64, 64, kernel_size=3, padding=1),
            nn.ReLU(),
            nn.Conv2d(64, 64, kernel_size=3, padding=1),
            nn.ReLU(),
        )
        self.policy_head = nn.Sequential(
            nn.Flatten(),
            nn.Linear(64 * BOARD_SIZE * BOARD_SIZE, MOVE_POLICY_SIZE),
        )
        self.value_head = nn.Sequential(
            nn.Flatten(),
            nn.Linear(64 * BOARD_SIZE * BOARD_SIZE, 128),
            nn.ReLU(),
            nn.Linear(128, 1),
            nn.Tanh(),
        )

    def forward(self, board: torch.Tensor) -> tuple[torch.Tensor, torch.Tensor]:
        features = self.body(board)
        move_logits = self.policy_head(features)
        value = self.value_head(features)
        return move_logits, value


def train_model(args: argparse.Namespace, device: torch.device) -> ChessItNet:
    model = ChessItNet().to(device)
    if args.skip_training:
        print("Skipping training and exporting an initialized model.")
        return model

    dataset = ChessPGNDataset(args.pgn, args.max_positions)
    loader = DataLoader(dataset, batch_size=args.batch_size, shuffle=True)
    optimizer = torch.optim.Adam(model.parameters(), lr=args.learning_rate)

    for epoch in range(args.epochs):
        model.train()
        total_loss = 0.0
        total_policy_loss = 0.0
        total_value_loss = 0.0
        pbar = tqdm(loader, desc=f"Epoch {epoch + 1}/{args.epochs}")

        for board_tensor, policy_target, value_target in pbar:
            board_tensor = board_tensor.to(device)
            policy_target = policy_target.to(device)
            value_target = value_target.to(device)

            optimizer.zero_grad()
            move_logits, value = model(board_tensor)
            policy_loss = F.cross_entropy(move_logits, policy_target)
            value_loss = F.mse_loss(value, value_target)
            loss = policy_loss + 0.25 * value_loss
            loss.backward()
            optimizer.step()

            total_loss += loss.item()
            total_policy_loss += policy_loss.item()
            total_value_loss += value_loss.item()
            pbar.set_postfix(
                loss=f"{loss.item():.4f}",
                policy=f"{policy_loss.item():.4f}",
                value=f"{value_loss.item():.4f}",
            )

        batches = max(1, len(loader))
        print(
            f"Epoch {epoch + 1}: "
            f"loss={total_loss / batches:.4f}, "
            f"policy={total_policy_loss / batches:.4f}, "
            f"value={total_value_loss / batches:.4f}"
        )

    return model


def export_onnx(model: ChessItNet, output_path: str | Path, device: torch.device) -> None:
    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    model.eval()
    dummy_input = torch.zeros((1, BOARD_CHANNELS, BOARD_SIZE, BOARD_SIZE), dtype=torch.float32, device=device)

    torch.onnx.export(
        model,
        dummy_input,
        output_path.as_posix(),
        input_names=["board"],
        output_names=["move_logits", "value"],
        opset_version=17,
        dynamic_axes={
            "board": {0: "batch"},
            "move_logits": {0: "batch"},
            "value": {0: "batch"},
        },
    )
    onnx.checker.check_model(output_path.as_posix())
    print(f"ONNX model exported to: {output_path}")


def test_onnx(output_path: str | Path) -> None:
    output_path = Path(output_path)
    session = ort.InferenceSession(output_path.as_posix(), providers=["CPUExecutionProvider"])
    dummy_input = np.zeros((1, BOARD_CHANNELS, BOARD_SIZE, BOARD_SIZE), dtype=np.float32)
    move_logits, value = session.run(None, {"board": dummy_input})

    if move_logits.shape != (1, MOVE_POLICY_SIZE):
        raise RuntimeError(f"Unexpected move_logits shape: {move_logits.shape}")
    if value.shape != (1, 1):
        raise RuntimeError(f"Unexpected value shape: {value.shape}")

    print("ONNX Runtime smoke test OK")
    print(f"move_logits shape: {move_logits.shape}")
    print(f"value shape: {value.shape}")


def main() -> None:
    args = parse_args()
    device = select_device(args.device)
    print(f"Using device: {device}")

    checkpoint_path = Path(args.checkpoint)
    checkpoint_path.parent.mkdir(parents=True, exist_ok=True)

    model = train_model(args, device)
    torch.save(model.state_dict(), checkpoint_path)
    print(f"PyTorch checkpoint saved to: {checkpoint_path}")

    export_onnx(model, args.output, device)
    test_onnx(args.output)


if __name__ == "__main__":
    main()
