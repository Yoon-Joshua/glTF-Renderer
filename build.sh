cd shaders/deferred
glslang -V geometry.vert --target-env spirv1.0 -o  bin/geometry.vert.spv
glslang -V geometry.frag --target-env spirv1.0 -o  bin/geometry.frag.spv
glslang -V lighting.vert --target-env spirv1.0 -o  bin/lighting.vert.spv
glslang -V lighting.frag --target-env spirv1.0 -o  bin/lighting.frag.spv
# glslc geometry.vert -o geometry.vert.spv
# glslc geometry.HAS_BASE_COLOR_TEXTURE.frag -o geometry.HAS_BASE_COLOR_TEXTURE.frag.spv
# glslc geometry.frag -o geometry.frag.spv
# glslc lighting.vert -o lighting.vert.spv
# glslc lighting.frag -o lighting.frag.spv
cd ../..