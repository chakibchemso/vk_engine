@echo off

echo "[Compiling] vertex shaders..."

for %%f in (assets/shaders/*.vert) do (
    echo "  %%f"
    glslc assets/shaders/%%f -o assets/shaders/%%f.spv
)

echo "[Compiling] fragment shaders..."

for %%f in (assets/shaders/*.frag) do (
    echo "  %%f"
    glslc assets/shaders/%%f -o assets/shaders/%%f.spv
)

echo "Done."