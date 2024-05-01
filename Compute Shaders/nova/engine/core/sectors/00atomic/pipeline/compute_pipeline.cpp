#include "compute_pipeline.h"
#include "../genesis.h"

ComputePipeline::ComputePipeline() 
    {
        report(LOGGER::INFO, "ComputePipeline - Instantiating ..");
        clear();
    }

ComputePipeline::~ComputePipeline() 
    {
        report(LOGGER::INFO, "ComputePipeline - Destroying ..");
        clear();
    }

void ComputePipeline::clear() 
    {
        report(LOGGER::INFO, "ComputePipeline - Clearing ..");

        instance = VK_NULL_HANDLE;
        layout = VK_NULL_HANDLE;
        _shader_modules.clear();
        _shader_stages.clear();

        return;
    }

// TODO: Abstract this into the Core Pipeline Class for mutual inheritance and abstract the name
void ComputePipeline::addShaderStage(VkShaderModule shader_module, VkShaderStageFlagBits stage) 
    {
        report(LOGGER::INFO, "ComputePipeline - Adding Shader Stage ..");

        VkPipelineShaderStageCreateInfo _shader_stage = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = stage,
                .module = shader_module,
                .pName = "main"
            };

        _shader_stages.push_back(_shader_stage);

        return;
    }

// TODO: build this into a createShader() function in the Core Pipeline Class for inheritance
ComputePipeline& ComputePipeline::shaders(VkDevice* logical_device) 
    {
        report(LOGGER::INFO, "ComputePipeline - Loading Shaders ..");

        std::vector<char> _comp_shader = genesis::loadFile(comp_shader);
        VkShaderModule _comp_shader_module;
        genesis::createShaderModule(logical_device, _comp_shader, &_comp_shader_module);
        addShaderStage(_comp_shader_module, VK_SHADER_STAGE_COMPUTE_BIT);

        return *this;
    }

ComputePipeline& ComputePipeline::createLayout(VkDevice* logical_device, VkDescriptorSetLayout* descriptor_layout) 
    {
        report(LOGGER::INFO, "ComputePipeline - Creating Layout ..");

        _pipeline_layout_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 1,
                .pSetLayouts = descriptor_layout,
        };

        VK_TRY(vkCreatePipelineLayout(*logical_device, &_pipeline_layout_info, nullptr, &layout));

        return *this;
    }

ComputePipeline& ComputePipeline::create(VkDevice* logical_device) 
    {
        report(LOGGER::INFO, "ComputePipeline - Creating Compute Pipeline ..");

        VkComputePipelineCreateInfo _pipeline_info = {
                .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                .stage = _shader_stages[0],
                .layout = layout
            };

        VK_TRY(vkCreateComputePipelines(*logical_device, VK_NULL_HANDLE, 1, &_pipeline_info, nullptr, &instance));

        vkDestroyShaderModule(*logical_device, _shader_stages[0].module, nullptr);

        return *this;
    }
