echo "Compiling shaders..."
"C:\SDK\VulkanSDK\1.3.243.0\Bin\glslc.exe" shaders/simple_shader.vert -o shaders/simple_shader.vert.spv
"C:\SDK\VulkanSDK\1.3.243.0\Bin\glslc.exe" shaders/simple_shader.frag -o shaders/simple_shader.frag.spv
echo "Done."