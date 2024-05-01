#pragma once
#include "../atomic.h"

class ComputePipeline {
    public:
        VkPipeline instance;
        VkPipelineLayout layout;

        ComputePipeline();
        ~ComputePipeline();

        ComputePipeline& shaders(VkDevice*);
        ComputePipeline& createLayout(VkDevice*, VkDescriptorSetLayout*);
        ComputePipeline& create(VkDevice*);

    private:
        std::vector<VkShaderModule> _shader_modules;
        std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;
        VkPipelineLayoutCreateInfo _pipeline_layout_info;

        void clear();
        void addShaderStage(VkShaderModule, VkShaderStageFlagBits);

};