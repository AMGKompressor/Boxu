cmake_minimum_required(VERSION 3.16)
project(COMP710_GPFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Course SDKs live next to source: Boxu/lib/
set(GP_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib")

set(SOURCES
  game.cpp
  logmanager.cpp
  main.cpp
  matrix4.cpp
  renderer.cpp
  shader.cpp
  sprite.cpp
  texture.cpp
  texturemanager.cpp
  vertexarray.cpp
)

add_executable(gpframework ${SOURCES})

target_include_directories(gpframework PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

if(WIN32)
  # Visual Studio + Win32/x86 libs from Studio5Resources layout (no vcpkg required)
  set(GP_INCLUDES
    "${GP_LIB}/SDL2-2.0.22/include"
    "${GP_LIB}/SDL2_image-2.6.1/include"
    "${GP_LIB}/glew-2.1.0/include"
  )
  target_include_directories(gpframework BEFORE PRIVATE ${GP_INCLUDES})
  target_link_directories(gpframework PRIVATE
    "${GP_LIB}/SDL2-2.0.22/lib/x86"
    "${GP_LIB}/SDL2_image-2.6.1/lib/x86"
    "${GP_LIB}/glew-2.1.0/lib/x86"
  )
  target_link_libraries(gpframework PRIVATE
    SDL2
    SDL2main
    SDL2_image
    glew32
    opengl32
  )
else()
  if(APPLE)
    execute_process(
      COMMAND brew --prefix
      OUTPUT_VARIABLE HOMEBREW_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(HOMEBREW_PREFIX)
      list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
      list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/sdl2")
      list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/sdl2_image")
      list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/glew")
    endif()
  endif()

  find_package(OpenGL REQUIRED)
  find_package(SDL2 CONFIG REQUIRED)

  set(_sdl2_image_linked FALSE)
  find_package(SDL2_image CONFIG QUIET)
  if(TARGET SDL2_image::SDL2_image)
    set(_sdl2_image_linked TRUE)
  endif()

  if(NOT _sdl2_image_linked)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_IMAGE_PC REQUIRED IMPORTED_TARGET SDL2_image)
  endif()

  find_package(GLEW REQUIRED)

  target_link_libraries(gpframework PRIVATE SDL2::SDL2)

  if(TARGET SDL2_image::SDL2_image)
    target_link_libraries(gpframework PRIVATE SDL2_image::SDL2_image)
  else()
    target_link_libraries(gpframework PRIVATE PkgConfig::SDL2_image)
  endif()

  if(TARGET GLEW::GLEW)
    target_link_libraries(gpframework PRIVATE GLEW::GLEW)
  else()
    target_include_directories(gpframework PRIVATE ${GLEW_INCLUDE_DIRS})
    target_link_libraries(gpframework PRIVATE ${GLEW_LIBRARIES})
  endif()

  target_link_libraries(gpframework PRIVATE OpenGL::GL)
endif()

if(MSVC)
  target_compile_options(gpframework PRIVATE /W3)
endif()

add_custom_command(
  TARGET gpframework POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
          "$<TARGET_FILE_DIR:gpframework>/shaders"
  COMMENT "Copying shaders next to executable"
)
add_custom_command(
  TARGET gpframework POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/../game/sounds"
          "$<TARGET_FILE_DIR:gpframework>/sounds"
  COMMENT "Copying sounds next to executable"
)
