#pragma once
#include "../../components/lexicon.h"
#include "./atomic.h"
#include <vector>


class Gateway {
    public:
	    Gateway();
        ~Gateway();

        Gateway& define(EngineContext*);
        Gateway& shaders();
        Gateway& vertexInput();
        Gateway& inputAssembly();
        Gateway& viewportState();
        Gateway& rasterizer();
        Gateway& multisampling();
        Gateway& depthStencil();
        Gateway& colorBlending();
        Gateway& dynamicState();
        Gateway& layout();
        Gateway& pipeline();
        Gateway& create();
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
        VkPipelineColorBlendStateCreateInfo _color_blending;
        std::vector<VkDynamicState> _dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
        VkPipelineDynamicStateCreateInfo _dynamic_state;
        std::vector<VkShaderModule> _shader_modules;
        std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;
        VkPipelineLayoutCreateInfo _pipeline_layout_info;
        VkPipelineLayout _pipeline_layout;
        VkGraphicsPipelineCreateInfo _pipeline_info;

//        inline VkPipelineColorBlendAttachmentState Gateway::colorBlendAttachment();
        void addShaderStage(VkShaderModule, VkShaderStageFlagBits);
};