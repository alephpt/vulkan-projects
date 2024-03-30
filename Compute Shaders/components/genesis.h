#pragma once

#include <string>
#include <vector>
#include <fstream>


const std::string vert_shader = "/home/persist/z/Documents/repos/learning/Cpp/Vulkan/Compute Shaders/components/shaders/tri_vert.spv";
const std::string frag_shader = "/home/persist/z/Documents/repos/learning/Cpp/Vulkan/Compute Shaders/components/shaders/tri_frag.spv";

namespace genesis {
    std::vector<char> loadFile(const std::string& filename);
}