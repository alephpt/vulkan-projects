#pragma once
#include "../atomic.h"
#include "../vertex.h"

#include <vector>

/*
    The Pipeline class is a builder that allows for the creation of a graphics pipeline
    and is responsible for the creation of the pipeline layout, shaders, and other pipeline related
    configurations.
*/

// TODO: Assert Singleton
class GraphicsPipeline {
    public:
	    GraphicsPipeline();
        ~GraphicsPipeline();

        VkPipeline instance;
        VkPipelineLayout layout;
        std::vector<Vertex> vertices = {};
        std::vector<uint32_t> indices = {};

        GraphicsPipeline& shaders(VkDevice*);
        GraphicsPipeline& vertexInput();
        GraphicsPipeline& inputAssembly();
        GraphicsPipeline& viewportState();
        GraphicsPipeline& rasterizer();
        GraphicsPipeline& multisampling(VkSampleCountFlagBits);
        GraphicsPipeline& depthStencil();
        GraphicsPipeline& colorBlending();
        GraphicsPipeline& dynamicState();
        GraphicsPipeline& createLayout(VkDevice*, VkDescriptorSetLayout*);
        GraphicsPipeline& pipe(VkRenderPass*);
        GraphicsPipeline& create(VkDevice*);
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
       
        VkVertexInputBindingDescription _binding_description;
        std::array<VkVertexInputAttributeDescription, 2U> _attribute_descriptions;

        void addShaderStage(VkShaderModule, VkShaderStageFlagBits);
};
