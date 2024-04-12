#pragma once

#include <string>
#include <vector>
#include "./vertex.h"

const std::string vert_shader = "/home/persist/z/Documents/repos/learning/Cpp/Vulkan/Compute Shaders/modules/graphics/engine/components/shaders/sq1_v.spv";
const std::string frag_shader = "/home/persist/z/Documents/repos/learning/Cpp/Vulkan/Compute Shaders/modules/graphics/engine/components/shaders/sq1_f.spv";

namespace genesis {
    std::vector<char> loadFile(const std::string&);
    void createObjects(std::vector<Vertex>*, std::vector<uint32_t>*);
}