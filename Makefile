# ChessIt 3D - GNU Make build
# Default platform is Windows/MinGW as requested.
#
# Usage examples:
#   mingw32-make
#   mingw32-make CXX=g++ BUILD_DIR=build/mingw-release
#   make PLATFORM=linux
#
# Important: if you pass paths manually, prefer forward slashes:
#   C:/dev/chessit_irr
# Backslashes are normalized automatically for path variables used here.

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
MEDIA_DIR := $(ROOT_DIR)/media
BIN_DIR := bin

ifeq ($(PLATFORM),windows)
  DEFAULT_BUILD_DIR := build/mingw
  TARGET_NAME := chessit_3d.exe
  PLATFORM_LIBS := -lopengl32 -lgdi32 -lwinmm
  COPY_DLLS := yes
else ifeq ($(PLATFORM),linux)
  DEFAULT_BUILD_DIR := build/linux
  TARGET_NAME := chessit_3d
  PLATFORM_LIBS := -lGL -lX11 -lXxf86vm -lpthread -ldl
  COPY_DLLS := no
else
  $(error Unsupported PLATFORM '$(PLATFORM)'. Use PLATFORM=windows or PLATFORM=linux)
endif

BUILD_DIR ?= $(DEFAULT_BUILD_DIR)
BUILD_DIR := $(call slash,$(BUILD_DIR))
OBJ_DIR := $(BUILD_DIR)/obj
TARGET := $(BUILD_DIR)/$(TARGET_NAME)

# Keep build output relative. GNU Make treats ':' as a rule separator on many
# hosts, so absolute drive-letter paths such as C:/build in targets can produce
# "multiple target patterns" before recipes run. Use a relative BUILD_DIR and
# use ROOT_DIR/MEDIA_DIR for absolute runtime paths.
ifneq ($(findstring :,$(BUILD_DIR)),)
  $(error BUILD_DIR must be relative for GNU Make targets. Received '$(BUILD_DIR)'. Use e.g. BUILD_DIR=build/mingw)
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

.PHONY: all clean run copy-media copy-dlls print-config

all: $(TARGET) copy-media copy-dlls

$(TARGET): $(OBJECTS) $(STATIC_LIBS)
	@mkdir -p "$(@D)"
	$(CXX) $(OBJECTS) -o "$@" $(LDLIBS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p "$(@D)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c "$<" -o "$@"

copy-media:
	@mkdir -p "$(BUILD_DIR)/media"
	@cp -R "media/." "$(BUILD_DIR)/media/" 2>/dev/null || true

copy-dlls:
ifeq ($(COPY_DLLS),yes)
	@cp "$(BIN_DIR)"/*.dll "$(BUILD_DIR)/" 2>/dev/null || true
endif

run: all
	"$(TARGET)"

print-config:
	@echo PLATFORM=$(PLATFORM)
	@echo CXX=$(CXX)
	@echo CONFIG=$(CONFIG)
	@echo ROOT_DIR=$(ROOT_DIR)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo TARGET=$(TARGET)

clean:
	@rm -rf "$(BUILD_DIR)"

-include $(DEPS)
