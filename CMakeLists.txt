cmake_minimum_required(VERSION 3.20)

project(ChessIt3D
    VERSION 0.1.0
    DESCRIPTION "ChessIt 3D built with Irrlicht 1.7.2, IrrAI and SoLoud"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CHESSIT_COPY_MEDIA "Copy media assets next to the built executable" OFF)

set(CHESSIT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CHESSIT_INCLUDE_DIR "${CHESSIT_ROOT_DIR}/include")
set(CHESSIT_LIB_DIR "${CHESSIT_ROOT_DIR}/lib")
set(CHESSIT_BIN_DIR "${CHESSIT_ROOT_DIR}/bin")
set(CHESSIT_MEDIA_DIR "${CHESSIT_ROOT_DIR}/media")
set(CHESSIT_RUNTIME_MEDIA_DIR "../media")
file(TO_CMAKE_PATH "${CHESSIT_RUNTIME_MEDIA_DIR}" CHESSIT_RUNTIME_MEDIA_DIR_CMAKE)

set(CHESSIT_SOURCES
    src/main.cpp
    src/Core/Engine.cpp
    src/Core/Logger.cpp
    src/Game/BoardSquare.cpp
    src/Game/ChessPiece.cpp
    src/Managers/AIManager.cpp
    src/Managers/BillboardManager.cpp
    src/Managers/BoardManager.cpp
    src/Managers/ChessSceneManager.cpp
    src/Managers/InputManager.cpp
    src/Managers/ONNXAIManager.cpp
    src/Managers/PieceManager.cpp
    src/Managers/SaveReplaySystem.cpp
    src/Managers/SoundManager.cpp
    src/Managers/UIAudioMenu.cpp
    src/Managers/UIManager.cpp
    src/Parsing/PositionParser.cpp
    src/Rules/ChessRules.cpp
)

add_executable(chessit_3d)
target_sources(chessit_3d PRIVATE ${CHESSIT_SOURCES})

target_include_directories(chessit_3d PRIVATE
    "${CHESSIT_ROOT_DIR}/src"
    "${CHESSIT_INCLUDE_DIR}/irrlicht"
    "${CHESSIT_INCLUDE_DIR}/irrai"
    "${CHESSIT_INCLUDE_DIR}/soloud"
    "${CHESSIT_INCLUDE_DIR}/onnx"
)

target_compile_definitions(chessit_3d PRIVATE
    CHESSIT_MEDIA_DIR="${CHESSIT_RUNTIME_MEDIA_DIR_CMAKE}"
)

set_target_properties(chessit_3d PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CHESSIT_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CHESSIT_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CHESSIT_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CHESSIT_BIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CHESSIT_BIN_DIR}"
)

function(chessit_import_static_library target_name library_path)
    if(NOT EXISTS "${library_path}")
        message(FATAL_ERROR "Required static library not found: ${library_path}")
    endif()
    add_library(${target_name} STATIC IMPORTED GLOBAL)
    set_target_properties(${target_name} PROPERTIES IMPORTED_LOCATION "${library_path}")
endfunction()

chessit_import_static_library(chessit_irrai "${CHESSIT_LIB_DIR}/IrrAI.a")
chessit_import_static_library(chessit_soloud "${CHESSIT_LIB_DIR}/libsoloud.a")
chessit_import_static_library(chessit_irrlicht "${CHESSIT_LIB_DIR}/libIrrlicht.a")

target_link_libraries(chessit_3d PRIVATE
    chessit_irrai
    chessit_soloud
    chessit_irrlicht
)

if(WIN32)
    if(NOT EXISTS "${CHESSIT_LIB_DIR}/onnxruntime.lib")
        message(FATAL_ERROR "Required ONNX Runtime import library not found: ${CHESSIT_LIB_DIR}/onnxruntime.lib")
    endif()
    if(NOT EXISTS "${CHESSIT_BIN_DIR}/onnxruntime.dll")
        message(FATAL_ERROR "Required ONNX Runtime DLL not found: ${CHESSIT_BIN_DIR}/onnxruntime.dll")
    endif()
    add_library(chessit_onnxruntime SHARED IMPORTED GLOBAL)
    set_target_properties(chessit_onnxruntime PROPERTIES
        IMPORTED_IMPLIB "${CHESSIT_LIB_DIR}/onnxruntime.lib"
        IMPORTED_LOCATION "${CHESSIT_BIN_DIR}/onnxruntime.dll"
    )
    target_compile_definitions(chessit_3d PRIVATE CHESSIT_ENABLE_ONNX_RUNTIME ORT_DLL_IMPORT)
    target_link_libraries(chessit_3d PRIVATE chessit_onnxruntime opengl32 gdi32 winmm)
else()
    find_package(OpenGL QUIET)
    find_package(X11 QUIET)

    if(OpenGL_FOUND)
        target_link_libraries(chessit_3d PRIVATE OpenGL::GL)
    else()
        target_link_libraries(chessit_3d PRIVATE GL)
    endif()

    if(X11_FOUND)
        target_link_libraries(chessit_3d PRIVATE X11::X11)
        if(TARGET X11::Xxf86vm)
            target_link_libraries(chessit_3d PRIVATE X11::Xxf86vm)
        else()
            target_link_libraries(chessit_3d PRIVATE Xxf86vm)
        endif()
    else()
        target_link_libraries(chessit_3d PRIVATE X11 Xxf86vm)
    endif()

    target_link_libraries(chessit_3d PRIVATE pthread dl)
endif()

if(CHESSIT_COPY_MEDIA)
    add_custom_command(TARGET chessit_3d POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CHESSIT_MEDIA_DIR}" "$<TARGET_FILE_DIR:chessit_3d>/media"
        COMMENT "Copying media assets"
    )
endif()
