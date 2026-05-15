# GNU Make build for ChessIt 3D - Windows Priority
# Override from command line when needed, e.g.:
#   make CXX=x86_64-w64-mingw32-g++ PLATFORM=windows
#   make PLATFORM=linux
#   make install INSTALL_DIR=C:/Games/ChessIt3D PLATFORM=windows

#========================================================================================
# CONFIGURATION - Edit these values to customize your build
#========================================================================================

# Choose platform: windows (default) or linux
PLATFORM ?= windows

# Choose your C++ compiler
CXX ?= g++

# Build directory
# Default to bin so the executable is produced beside the runtime DLLs and chessit_ai.onnx model.
BUILD_DIR ?= bin

# Installation directory - CHANGE THIS TO YOUR PREFERRED PATH
# Examples:
#   INSTALL_DIR ?= C:/Program Files/ChessIt3D
#   INSTALL_DIR ?= D:/Games/ChessIt3D
#   INSTALL_DIR ?= /usr/local/games/chessit
INSTALL_DIR ?= D:/MING64/chessit_irr-main

#========================================================================================
# INTERNAL VARIABLES - Do not modify unless you know what you're doing
#========================================================================================

ROOT_DIR := $(CURDIR)
MEDIA_DIR := $(ROOT_DIR)/media
TARGET_BASE := chessit_3d

ifeq ($(PLATFORM),windows)
  TARGET := $(BUILD_DIR)/$(TARGET_BASE).exe
  SUFFIX := .exe
  LDLIBS := lib/IrrAI.a lib/libsoloud.a lib/libIrrlicht.a lib/onnxruntime.lib -lopengl32 -lgdi32 -lwinmm
  # No Windows, usamos o comando shell do CMD para criar diretórios recursivamente
  MKDIR_P = if not exist $(subst /,\,$(dir $@)) mkdir $(subst /,\,$(dir $@))
  RM_CMD := rmdir /s /q
else
  TARGET := $(BUILD_DIR)/$(TARGET_BASE)
  SUFFIX :=
  LDLIBS := lib/IrrAI.a lib/libsoloud.a lib/libIrrlicht.a -lGL -lX11 -lXxf86vm -lpthread -ldl
  MKDIR_P = mkdir -p $(dir $@)
  RM_CMD := rm -rf
endif

CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
CPPFLAGS := -Isrc -isystem include/irrlicht -isystem include/irrai -isystem include/soloud -isystem include/onnx -DCHESSIT_MEDIA_DIR=\"$(MEDIA_DIR)\"

ifeq ($(PLATFORM),windows)
  CPPFLAGS += -DCHESSIT_ENABLE_ONNX_RUNTIME -DORT_DLL_IMPORT
endif

SOURCES := \
  src/main.cpp \
  src/Core/Engine.cpp \
  src/Core/Logger.cpp \
  src/Game/BoardSquare.cpp \
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

#========================================================================================
# PHONY TARGETS
#========================================================================================

.PHONY: all clean run install copy-media info help

#========================================================================================
# BUILD TARGETS
#========================================================================================

all: $(TARGET) copy-media
	@echo ✓ Build complete! Target: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo [LINK] Linking executable...
	@$(MKDIR_P)
	$(CXX) $(OBJECTS) -o $@ $(LDLIBS)
	@echo ✓ Executable created: $@

$(BUILD_DIR)/%.o: %.cpp
	@$(MKDIR_P)
	@echo [CXX] Compiling $<
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

copy-media:
	@echo [COPY] Copying media files to build directory...
	@mkdir -p $(BUILD_DIR)/media
ifeq ($(PLATFORM),windows)
	@xcopy /E /I /Y media $(BUILD_DIR)/media >nul 2>&1 || echo Warning: Media copy had issues
else
	@cp -R media/. $(BUILD_DIR)/media/ 2>/dev/null || true
endif
	@echo ✓ Media files copied

#========================================================================================
# UTILITY TARGETS
#========================================================================================

run: all
	@echo [RUN] Executing application...
	@$(TARGET)

install: all
	@echo [INSTALL] Installing to: $(INSTALL_DIR)
ifeq ($(PLATFORM),windows)
	@if not exist "$(INSTALL_DIR)" mkdir "$(INSTALL_DIR)"
	@xcopy /E /I /Y $(BUILD_DIR) "$(INSTALL_DIR)" >nul 2>&1
else
	@mkdir -p "$(INSTALL_DIR)"
	@cp -R $(BUILD_DIR)/* "$(INSTALL_DIR)/"
endif
	@echo ✓ Installation complete! Location: $(INSTALL_DIR)
	@echo To run: $(INSTALL_DIR)/$(TARGET_BASE)$(SUFFIX)

clean:
ifeq ($(BUILD_DIR),bin)
	@echo [CLEAN] Removing generated build artifacts from bin without deleting runtime DLLs or ONNX models...
	@rm -f $(TARGET) $(OBJECTS) $(DEPS) 2>/dev/null || true
	@rm -rf $(BUILD_DIR)/src $(BUILD_DIR)/media 2>/dev/null || true
else
	@echo [CLEAN] Removing build directory...
	@$(RM_CMD) $(BUILD_DIR) 2>/dev/null || true
endif
	@echo ✓ Clean complete

info:
	@echo ==========================================
	@echo ChessIt 3D - Build Configuration
	@echo ==========================================
	@echo Platform:         $(PLATFORM)
	@echo Compiler:         $(CXX)
	@echo Build Directory:  $(BUILD_DIR)
	@echo Target:           $(TARGET)
	@echo Media Directory:  $(MEDIA_DIR)
	@echo Install Path:     $(INSTALL_DIR)
	@echo ==========================================

help:
	@echo ==========================================
	@echo ChessIt 3D - Makefile Help
	@echo ==========================================
	@echo
	@echo Usage: make [target] [VARIABLE=value]
	@echo
	@echo Targets:
	@echo   all          - Build everything (default)
	@echo   run          - Build and run the application
	@echo   install      - Build and install to INSTALL_DIR
	@echo   clean        - Remove build files
	@echo   info         - Show build configuration
	@echo   help         - Show this help message
	@echo
	@echo Variables:
	@echo   PLATFORM     - Target platform (windows or linux, default: windows)
	@echo   CXX          - C++ compiler (default: g++)
	@echo   BUILD_DIR    - Build directory (default: bin)
	@echo   INSTALL_DIR  - Installation directory
	@echo
	@echo Examples:
	@echo   make PLATFORM=windows
	@echo   make run PLATFORM=windows
	@echo   make PLATFORM=linux
	@echo   make install INSTALL_DIR=C:/Games/ChessIt3D PLATFORM=windows
	@echo   make CXX=x86_64-w64-mingw32-g++ PLATFORM=windows
	@echo   make clean
	@echo
	@echo ==========================================

#========================================================================================
# DEPENDENCY HANDLING
#========================================================================================

-include $(DEPS)

