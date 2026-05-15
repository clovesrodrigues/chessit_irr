#!/usr/bin/env python3
"""Inline ONNX external tensor data into one .onnx file.

Use this when ONNX Runtime reports errors such as:

    External Data Path
    ValidateExternalDataPath

The repair only works when the external tensor data files still exist next to the
input model. If only the .onnx file was copied and the external files were lost,
re-export the model with training/train_chessit_model.py instead.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import onnx


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Inline ONNX external data into a single model file.")
    parser.add_argument("input", help="Input ONNX model that may reference external tensor files.")
    parser.add_argument("output", help="Output ONNX model with tensors embedded in the .onnx file.")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    model = onnx.load(input_path.as_posix(), load_external_data=True)
    onnx.save_model(model, output_path.as_posix(), save_as_external_data=False)
    onnx.checker.check_model(output_path.as_posix())
    print(f"Saved single-file ONNX model to: {output_path}")


if __name__ == "__main__":
    main()
