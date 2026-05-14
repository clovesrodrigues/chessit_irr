# GNU Make build for ChessIt 3D.
# Override from command line when needed, e.g.:
#   make CXX=x86_64-w64-mingw32-g++ PLATFORM=windows

CXX ?= g++
PLATFORM ?= linux
BUILD_DIR ?= build_make
TARGET ?= $(BUILD_DIR)/chessit_3d
ROOT_DIR := $(CURDIR)
MEDIA_DIR := $(ROOT_DIR)/media

CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
CPPFLAGS := -Isrc -isystem include/irrlicht -isystem include/irrai -isystem include/soloud -isystem include/onnx -DCHESSIT_MEDIA_DIR=\"$(MEDIA_DIR)\"

SOURCES := \
  src/main.cpp \
  src/Core/Engine.cpp \
  src/Core/Logger.cpp \
  src/Game/ChessPiece.cpp \
  src/Managers/AIManager.cpp \
  src/Managers/BillboardManager.cpp \
  src/Managers/BoardManager.cpp \
  src/Managers/ChessSceneManager.cpp \
  src/Managers/InputManager.cpp \
  src/Managers/ONNXAIManager.cpp \
  src/Managers/PieceManager.cpp \
  src/Managers/SaveReplaySystem.cpp \
  src/Managers/SoundManager.cpp \
  src/Managers/UIAudioMenu.cpp \
  src/Managers/UIManager.cpp \
  src/Parsing/PositionParser.cpp \
  src/Rules/ChessRules.cpp

OBJECTS := $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

STATIC_LIBS := lib/IrrAI.a lib/libsoloud.a lib/libIrrlicht.a

ifeq ($(PLATFORM),windows)
  TARGET := $(BUILD_DIR)/chessit_3d.exe
  LDLIBS := $(STATIC_LIBS) -lopengl32 -lgdi32 -lwinmm
else
  LDLIBS := $(STATIC_LIBS) -lGL -lX11 -lXxf86vm -lpthread -ldl
endif

.PHONY: all clean run copy-media

all: $(TARGET) copy-media

$(TARGET): $(OBJECTS) $(STATIC_LIBS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

copy-media:
	@mkdir -p $(BUILD_DIR)/media
	@cp -R media/. $(BUILD_DIR)/media/ 2>/dev/null || true

run: all
	$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
