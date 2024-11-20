cmake_minimum_required(VERSION 3.7)

project(vulkan)
set(TARGET_NAME exec)
set(CMAKE_CXX_STANDARD 17)


include_directories(./)

set(SOURCE_ALL
        ./source/main.cpp
        ./source/logger.cpp
        ./source/window.cpp
        ./source/app.cpp
        ./source/vulkan_pipeline.cpp
        ./source/vulkan_device.cpp
        ./source/vulkan_swapchain.cpp
        ./source/vulkan_model.cpp
        ./source/vulkan_buffer.cpp
        ./source/vulkan_descriptors.cpp
        ./source/vulkan_texture.cpp
)


if (UNIX)
  add_definitions(-DLINUX)

  # debug
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG  "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g" )

  # release
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -fomit-frame-pointer -fno-strict-aliasing -fvisibility=hidden -fvisibility-inlines-hidden -fno-exceptions -fno-rtti -Wall")

  # link options
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s -Wl,--gc-sections -static-libstdc++ -static-libgcc -lXi -lstdc++")

  add_executable(${TARGET_NAME} ${SOURCE_ALL})

  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "./out/")
  set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_NAME "exec")

  
  find_package(Threads REQUIRED)
  find_package(OpenAL REQUIRED)
  find_package(Vulkan REQUIRED)
  find_package(GLEW REQUIRED)
  find_package(glfw3 3.3 REQUIRED)
  find_package(Freetype REQUIRED)
  find_package(SDL2 REQUIRED)
  
  # look for SDL2
  if(APPLE)
    include_directories("/opt/local/include")
    include_directories("/opt/homebrew/include")
    include_directories("/opt/homebrew/Cellar/freetype/2.13.0_1/include/freetype2/")
  endif()

  include_directories("/usr/include/freetype2")

  set(SDL2_LIBRARY SDL2)

  set(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  set(CMAKE_HAVE_THREADS_LIBRARY 1)
  set(CMAKE_USE_WIN32_THREADS_INIT 0)
  set(CMAKE_USE_PTHREADS_INIT 1)
  set(THREADS_PREFER_PTHREAD_FLAG ON)

  target_link_libraries(${TARGET_NAME} ${CMAKE_DL_LIBS} SDL2::SDL2 Vulkan::Vulkan ${FREETYPE_LIBRARIES} GLEW::GLEW glfw)



  file(GLOB_RECURSE GLSL_SOURCE_FILES "./resources/shaders/*.frag" "./resources/shaders/*.vert")

  foreach(GLSL ${GLSL_SOURCE_FILES})
    message("Found Shader: ${GLSL}")
    set(SPIRV "${GLSL}.spv")
    add_custom_command(
      OUTPUT ${SPIRV}
      COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
      DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
  endforeach(GLSL)

  add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

  add_dependencies(${TARGET_NAME} Shaders)
endif()
