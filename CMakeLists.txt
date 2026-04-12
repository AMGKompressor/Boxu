cmake_minimum_required(VERSION 3.16)
project(ShadowBox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# COMP710 Studio 5 layout: third-party under lib/ (copied from Studio5Resources)
set(GP_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(GP_INCLUDES
  "${GP_LIB}/SDL2-2.0.22/include"
  "${GP_LIB}/SDL2_image-2.6.1/include"
  "${GP_LIB}/glew-2.1.0/include"
)

# --- macOS/Linux: discover packages once (Windows uses lib/x86 .lib) ---
if(NOT WIN32)
  if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
  endif()
  find_package(SDL2 REQUIRED CONFIG)
  find_package(SDL2_image REQUIRED CONFIG)
  find_package(OpenGL REQUIRED)
  find_package(GLEW REQUIRED)
endif()

function(gp_apply_sdl_target targetName)
  if(WIN32)
    # Visual Studio / Win32: course layout — all headers from lib/
    target_include_directories(${targetName} BEFORE PRIVATE ${GP_INCLUDES})
    target_link_directories(${targetName} PRIVATE
      "${GP_LIB}/SDL2-2.0.22/lib/x86"
      "${GP_LIB}/SDL2_image-2.6.1/lib/x86"
      "${GP_LIB}/glew-2.1.0/lib/x86"
    )
    target_link_libraries(${targetName} PRIVATE
      SDL2
      SDL2main
      SDL2_image
      glew32
      opengl32
    )
  else()
    # macOS/Linux: SDL headers must come from Homebrew (linked libs), not lib/SDL2-2.0.22.
    # Old course SDL + Apple Clang 17 hits missing memset_pattern4 in SDL_stdinc.h.
    target_include_directories(${targetName} BEFORE PRIVATE
      "${GP_LIB}/glew-2.1.0/include"
    )
    target_link_libraries(${targetName} PRIVATE
      SDL2::SDL2
      SDL2_image::SDL2_image
      OpenGL::GL
      GLEW::GLEW
    )
  endif()
endfunction()

# Minimal SDK-only executable (no Shadow Box / no GP framework sources)
add_executable(studio5_smoketest studio5_smoketest/main.cpp)
gp_apply_sdl_target(studio5_smoketest)

# Full GP-style skeleton + future game
add_executable(shadowbox
  source/main.cpp
  source/logmanager.cpp
  source/game.cpp
  source/renderer.cpp
)
target_include_directories(shadowbox PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source")
gp_apply_sdl_target(shadowbox)

if(MSVC)
  target_compile_options(shadowbox PRIVATE /W3)
  target_compile_options(studio5_smoketest PRIVATE /W3)
else()
  target_compile_options(shadowbox PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(studio5_smoketest PRIVATE -Wall -Wextra -Wpedantic)
endif()
