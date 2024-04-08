#pragma once
#include <vector>
#include "./atomic.h"


class Gateway {
    public:
	    Gateway();
        ~Gateway();

        VkPipeline pipeline;
        VkPipelineLayout pipeline_layout;
        std::vector<Vertex> vertices = {};

        Gateway& shaders(VkDevice*);
        Gateway& vertexInput();
        Gateway& inputAssembly();
        Gateway& viewportState();
        Gateway& rasterizer();
        Gateway& multisampling();
        Gateway& depthStencil();
        Gateway& colorBlending();
        Gateway& dynamicState();
        Gateway& layout(VkDevice*);
        Gateway& pipe(VkRenderPass*);
        Gateway& create(VkDevice*);
        void clear();


    private:
        VkGraphicsPipelineCreateInfo _pipeline_info;
        VkPipelineLayoutCreateInfo _pipeline_layout_info;
        VkPipelineViewportStateCreateInfo _viewport_state;
        VkPipelineVertexInputStateCreateInfo _vertex_input_state;
        VkPipelineInputAssemblyStateCreateInfo _input_assembly;
        VkPipelineDynamicStateCreateInfo _dynamic_state;
        std::vector<VkShaderModule> _shader_modules;
        std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;
        VkRenderingInfo _render_info;
        VkPipelineRasterizationStateCreateInfo _rasterizer;
        VkPipelineMultisampleStateCreateInfo _multisampling;
        VkPipelineDepthStencilStateCreateInfo _depth_stencil;
        VkPipelineColorBlendAttachmentState _color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo _color_blending;
        std::vector<VkDynamicState> _dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
       
        void addShaderStage(VkShaderModule, VkShaderStageFlagBits);
};
