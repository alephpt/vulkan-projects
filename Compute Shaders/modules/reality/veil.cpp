#include "./veil.h"
#include "../../components/genesis.h"

void createShaderData(Gateway* gateway) 
    {
        report(LOGGER::INFO, "Veil - Creating Shader Data ..");

        std::vector<char> vert_shader_code = genesis::loadFile(vert_shader);
        VkShaderModule vert_shader_module = gateway->createShaderModule(vert_shader_code);
        gateway->addShaderStage(vert_shader_module, VK_SHADER_STAGE_VERTEX_BIT);

        std::vector<char> frag_shader_code = genesis::loadFile(frag_shader);
        VkShaderModule frag_shader_module = gateway->createShaderModule(frag_shader_code);
        gateway->addShaderStage(frag_shader_module, VK_SHADER_STAGE_FRAGMENT_BIT);

        return;
    }
