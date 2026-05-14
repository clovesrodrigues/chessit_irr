cmake_minimum_required(VERSION 3.16)
project(ChessIt3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CHESSIT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CHESSIT_INCLUDE_DIR ${CHESSIT_ROOT_DIR}/include)
set(CHESSIT_LIB_DIR ${CHESSIT_ROOT_DIR}/lib)
set(CHESSIT_BIN_DIR ${CHESSIT_ROOT_DIR}/bin)

add_executable(chessit_3d
    src/main.cpp
    src/Core/Engine.cpp
    src/Core/Logger.cpp
    src/Managers/BillboardManager.cpp
    src/Game/ChessPiece.cpp
    src/Managers/AIManager.cpp
    src/Managers/BoardManager.cpp
    src/Managers/ChessSceneManager.cpp
    src/Managers/InputManager.cpp
    src/Managers/ONNXAIManager.cpp
    src/Managers/PieceManager.cpp
    src/Managers/SaveReplaySystem.cpp
    src/Managers/SoundManager.cpp
    src/Managers/UIManager.cpp
    src/Managers/UIAudioMenu.cpp
    src/Parsing/PositionParser.cpp
    src/Rules/ChessRules.cpp
)

target_include_directories(chessit_3d PRIVATE
    ${CHESSIT_ROOT_DIR}/src
    ${CHESSIT_INCLUDE_DIR}/irrlicht
    ${CHESSIT_INCLUDE_DIR}/irrai
    ${CHESSIT_INCLUDE_DIR}/soloud
    ${CHESSIT_INCLUDE_DIR}/onnx
)

target_compile_definitions(chessit_3d PRIVATE
    CHESSIT_MEDIA_DIR="${CHESSIT_ROOT_DIR}/media"
)

find_library(IRRLICHT_LIBRARY NAMES Irrlicht libIrrlicht irrlicht libirrlicht PATHS ${CHESSIT_LIB_DIR} NO_DEFAULT_PATH)
find_library(SOLOUD_LIBRARY NAMES soloud libsoloud PATHS ${CHESSIT_LIB_DIR} NO_DEFAULT_PATH)

set(IRRAI_LIBRARY ${CHESSIT_LIB_DIR}/IrrAI.a)
if(NOT EXISTS ${IRRAI_LIBRARY})
    find_library(IRRAI_LIBRARY NAMES IrrAI libIrrAI irrai libirrai PATHS ${CHESSIT_LIB_DIR} NO_DEFAULT_PATH)
endif()

if(NOT IRRLICHT_LIBRARY)
    message(FATAL_ERROR "Irrlicht static library was not found in ${CHESSIT_LIB_DIR}")
endif()
if(NOT IRRAI_LIBRARY)
    message(FATAL_ERROR "IrrAI static library was not found in ${CHESSIT_LIB_DIR}")
endif()
if(NOT SOLOUD_LIBRARY)
    message(FATAL_ERROR "Soloud static library was not found in ${CHESSIT_LIB_DIR}")
endif()

target_link_libraries(chessit_3d PRIVATE ${IRRAI_LIBRARY} ${SOLOUD_LIBRARY} ${IRRLICHT_LIBRARY})

if(WIN32)
    target_link_libraries(chessit_3d PRIVATE opengl32 gdi32 winmm)
    add_custom_command(TARGET chessit_3d POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CHESSIT_BIN_DIR} $<TARGET_FILE_DIR:chessit_3d>
    )
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

add_custom_command(TARGET chessit_3d POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CHESSIT_ROOT_DIR}/media $<TARGET_FILE_DIR:chessit_3d>/media
)
