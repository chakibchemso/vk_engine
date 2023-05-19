@echo off
echo "Compiling shaders..."
glslc assets/shaders/simple_shader.vert -o assets/shaders/simple_shader.vert.spv
glslc assets/shaders/simple_shader.frag -o assets/shaders/simple_shader.frag.spv
echo "Done."