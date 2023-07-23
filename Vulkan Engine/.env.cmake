set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}) #for shared libs
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

set(GLFW_PATH extern/glfw)
set(GLM_PATH extern/glm)
set(VULKAN_SDK_PATH C:/SDK/VulkanSDK/1.3.243.0)

# Set MINGW_PATH if using mingwBuild.bat and not VisualStudio20XX
# set(MINGW_PATH "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64")

# Optional set TINYOBJ_PATH to target specific version,
# Otherwise defaults to external/tinyobjloader
set(TINYOBJ_PATH extern/tiny_obj_loader)