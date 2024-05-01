#pragma once

#include <string>
#include <vector>
#include "vertex.h"
#include "particle.h"

const std::string vert_shader = "/home/persist/z/Ancillary/Big Stick Studios/repos/learning/Cpp/Vulkan/Compute Shaders/nova/engine/core/components/shaders/sq1_v.spv";
const std::string frag_shader = "/home/persist/z/Ancillary/Big Stick Studios/repos/learning/Cpp/Vulkan/Compute Shaders/nova/engine/core/components/shaders/sq1_f.spv";
const std::string comp_shader = "/home/persist/z/Ancillary/Big Stick Studios/repos/learning/Cpp/Vulkan/Compute Shaders/nova/engine/core/components/shaders/sq1_c.spv";

namespace genesis {
    std::vector<char> loadFile(const std::string&);
    void createObjects(std::vector<Vertex>*, std::vector<uint32_t>*);
    void createParticles(std::vector<Particle>*);
    void createShaderModule(VkDevice*, std::vector<char>&, VkShaderModule*);
}