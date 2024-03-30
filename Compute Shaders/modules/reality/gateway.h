#pragma once
#include "../../components/lexicon.h"
#include "./atomic.h"
#include <vector>

class Gateway {
    public:
        std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;

	    Gateway();
        ~Gateway();

        void pipeline();
        Gateway define(EngineContext*);
        VkShaderModule createShaderModule(const std::vector<char>&);
        void addShaderStage(VkShaderModule, VkShaderStageFlagBits);
        void clear();

    private:
        EngineContext *_context;
        VkPipeline _instance;

        VkPipelineInputAssemblyStateCreateInfo _input_assembly;
        VkPipelineViewportStateCreateInfo _viewport_state;
        VkRenderingInfo _render_info;
        VkPipelineVertexInputStateCreateInfo _vertex_input_state;
        VkPipelineRasterizationStateCreateInfo _rasterizer;
        VkPipelineMultisampleStateCreateInfo _multisampling;
        VkPipelineDepthStencilStateCreateInfo _depth_stencil;
        VkPipelineColorBlendAttachmentState _color_blend_attachment;
        
        VkPipelineColorBlendStateCreateInfo _color_blending;
        VkPipelineLayout _pipeline_layout;
        VkGraphicsPipelineCreateInfo _pipeline_info;


        void createViewportState();
        void createColorBlendState();
        void createPipelineInfo();
};