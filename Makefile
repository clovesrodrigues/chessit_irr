# ChessIt 3D - GNU Make build
# Default platform is Windows/MinGW as requested.
# The executable is emitted directly into ./bin so it runs beside required DLLs.
#
# Usage examples:
#   mingw32-make
#   mingw32-make CXX=g++ CONFIG=debug
#   make PLATFORM=linux

empty :=
space := $(empty) $(empty)
slash = $(subst \,/,$(strip $(1)))

PLATFORM ?= windows
CXX ?= g++
CONFIG ?= release

ROOT_DIR_RAW := $(CURDIR)
ROOT_DIR := $(call slash,$(ROOT_DIR_RAW))
SRC_DIR := src
INCLUDE_DIR := include
LIB_DIR := lib
MEDIA_DIR := ../media
OUTPUT_DIR := bin
BIN_DIR := bin

ifeq ($(PLATFORM),windows)
  BUILD_DIR ?= build/mingw
  TARGET_NAME := chessit_3d.exe
  PLATFORM_LIBS := -lopengl32 -lgdi32 -lwinmm
else ifeq ($(PLATFORM),linux)
  BUILD_DIR ?= build/linux
  TARGET_NAME := chessit_3d
  PLATFORM_LIBS := -lGL -lX11 -lXxf86vm -lpthread -ldl
else
  $(error Unsupported PLATFORM '$(PLATFORM)'. Use PLATFORM=windows or PLATFORM=linux)
endif

BUILD_DIR := $(call slash,$(BUILD_DIR))
OUTPUT_DIR := $(call slash,$(OUTPUT_DIR))
OBJ_DIR := $(BUILD_DIR)/obj
TARGET := $(OUTPUT_DIR)/$(TARGET_NAME)

# GNU Make parses ':' in target names as a rule separator. Keep generated target
# paths relative to avoid "multiple target patterns" on Windows drive letters.
ifneq ($(findstring :,$(BUILD_DIR)$(OUTPUT_DIR)),)
  $(error BUILD_DIR and OUTPUT_DIR must be relative. Use e.g. BUILD_DIR=build/mingw OUTPUT_DIR=bin)
endif

CXXSTD := -std=c++17
WARNINGS := -Wall -Wextra
ifeq ($(CONFIG),debug)
  OPTFLAGS := -O0 -g
else
  OPTFLAGS := -O2
endif

CPPFLAGS += \
  -I$(SRC_DIR) \
  -isystem $(INCLUDE_DIR)/irrlicht \
  -isystem $(INCLUDE_DIR)/irrai \
  -isystem $(INCLUDE_DIR)/soloud \
  -isystem $(INCLUDE_DIR)/onnx \
  -DCHESSIT_MEDIA_DIR=\"$(MEDIA_DIR)\"

CXXFLAGS += $(CXXSTD) $(WARNINGS) $(OPTFLAGS)

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

OBJECTS := $(addprefix $(OBJ_DIR)/,$(SOURCES:.cpp=.o))
DEPS := $(OBJECTS:.o=.d)

IRRAI_LIB := $(LIB_DIR)/IrrAI.a
IRRLICHT_LIB := $(LIB_DIR)/libIrrlicht.a
SOLOUD_LIB := $(LIB_DIR)/libsoloud.a
STATIC_LIBS := $(IRRAI_LIB) $(SOLOUD_LIB) $(IRRLICHT_LIB)
LDLIBS := $(STATIC_LIBS) $(PLATFORM_LIBS)

.PHONY: all clean run print-config verify-media

all: $(TARGET) verify-media

$(TARGET): $(OBJECTS) $(STATIC_LIBS)
	@mkdir -p "$(@D)"
	$(CXX) $(OBJECTS) -o "$@" $(LDLIBS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p "$(@D)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"

verify-media:
	@test -f "media/env.irr" || echo "Warning: media/env.irr not found"
	@test -f "media/CHESSIT_POSITIONS.txt" || echo "Warning: media/CHESSIT_POSITIONS.txt not found"
	@test -f "media/BOARDER.obj" || echo "Warning: media/BOARDER.obj not found"

run: all
	cd "$(OUTPUT_DIR)" && ./$(TARGET_NAME)

print-config:
	@echo PLATFORM=$(PLATFORM)
	@echo CXX=$(CXX)
	@echo CONFIG=$(CONFIG)
	@echo ROOT_DIR=$(ROOT_DIR)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo OUTPUT_DIR=$(OUTPUT_DIR)
	@echo TARGET=$(TARGET)
	@echo MEDIA_DIR_RUNTIME=$(MEDIA_DIR)

clean:
	@rm -rf "$(BUILD_DIR)"
	@rm -f "$(TARGET)"

-include $(DEPS)
