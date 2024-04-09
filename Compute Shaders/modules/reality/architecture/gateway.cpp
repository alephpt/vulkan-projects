#include "./gateway.h"
#include "../../../components/genesis.h"


    ////////////////////////
    // GATEWAY DEFINITION //
    ////////////////////////

Gateway::Gateway()
    { 
        report(LOGGER::VLINE, "\t .. Initializing Pipeline ..");
        clear(); 

        report(LOGGER::VLINE, "\t .. Populating Vertices ..");
        genesis::populateVertices(&vertices);
    }

Gateway::~Gateway()
    { 
        report(LOGGER::INFO, "Gateway - Deconstructing Pipeline ..");
        clear();
    }

void Gateway::clear()
    {
        report(LOGGER::VLINE, "\t .. Clearing Gateway ..");
        _vertex_input_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        _input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        _viewport_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        _rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        _multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        _color_blend_attachment = {  };
        _color_blending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        _pipeline_info = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        _dynamic_state = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        _pipeline_layout_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        pipeline_layout = {};
        _depth_stencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO }; // not used
        _render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        _shader_stages.clear();
    }


    /////////////
    // SHADERS //
    /////////////

static inline void createShaderModule(VkDevice* logical_device, std::vector<char>& code, VkShaderModule* shader_module)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Shader Module ..");

        VkShaderModuleCreateInfo _create_info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data())
            };

        VK_TRY(vkCreateShaderModule(*logical_device, &_create_info, nullptr, shader_module));

        return;
    }

void Gateway::addShaderStage(VkShaderModule shader_module, VkShaderStageFlagBits stage)
    {
        std::string stage_name = stage == VK_SHADER_STAGE_VERTEX_BIT ? "Vertex" : "Fragment";
        report(LOGGER::VLINE, "\t\t .. Adding %s Shader Stage ..", stage_name.c_str());
        _shader_modules.push_back(shader_module);

        VkPipelineShaderStageCreateInfo _shader_stage = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = stage,
            .module = shader_module,
            .pName = "main"
        };

        _shader_stages.push_back(_shader_stage); // We can pass this by reference and make this a static inline function

        return;
    }

Gateway& Gateway::shaders(VkDevice* logical_device)
    {
        report(LOGGER::VLINE, "\t .. Creating Shaders ..");

        std::vector<char> _vert_shader_code = genesis::loadFile(vert_shader);
        VkShaderModule _vert_shader_module;
        createShaderModule(logical_device, _vert_shader_code, &_vert_shader_module);
        addShaderStage(_vert_shader_module, VK_SHADER_STAGE_VERTEX_BIT);

        std::vector<char> _frag_shader_code = genesis::loadFile(frag_shader);
        VkShaderModule _frag_shader_module;
        createShaderModule(logical_device, _frag_shader_code, &_frag_shader_module);
        addShaderStage(_frag_shader_module, VK_SHADER_STAGE_FRAGMENT_BIT);

        return *this;
    }


    //////////////////
    // VERTEX INPUT //
    //////////////////

Gateway& Gateway::vertexInput()
    {
        report(LOGGER::VLINE, "\t .. Creating Vertex Input State ..");

        /*
        report(LOGGER::INFO, "\t\t .. Vertices: %d", vertices.size());
        
        for (auto vertex : vertices) 
            { 
                report(LOGGER::INFO, "\t\t .. Vertex: %f, %f, %f", vertex.position.x, vertex.position.y, vertex.position.z); 
                report(LOGGER::INFO, "\t\t .. Color: %f, %f, %f", vertex.color.x, vertex.color.y, vertex.color.z);
            }
*/
        _binding_description = Vertex::getBindingDescription();
        _attribute_descriptions = Vertex::getAttributeDescriptions();

        _vertex_input_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &_binding_description,
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(_attribute_descriptions.size()),
                .pVertexAttributeDescriptions = _attribute_descriptions.data()
            };

        return *this;
    }


    ////////////////////
    // INPUT ASSEMBLY //
    ////////////////////

Gateway& Gateway::inputAssembly()
    {
        report(LOGGER::VLINE, "\t .. Creating Input Assembly ..");

        _input_assembly = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE
            };

        return *this;
    }


    ////////////////////
    // VIEWPORT STATE //
    ////////////////////

Gateway& Gateway::viewportState()
    {
        report(LOGGER::VLINE, "\t .. Creating Viewport State ..");

        _viewport_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount = 1,
            };

        return *this;
    }


    ////////////////
    // RASTERIZER //
    ////////////////

Gateway& Gateway::rasterizer()
    {
        report(LOGGER::VLINE, "\t .. Creating Rasterizer ..");

        _rasterizer = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f,
                .depthBiasClamp = 0.0f,
                .depthBiasSlopeFactor = 0.0f,
                .lineWidth = 1.0f
            };

        return *this;
    }


    ///////////////////
    // MULTISAMPLING //
    ///////////////////

Gateway& Gateway::multisampling()
    {
        report(LOGGER::VLINE, "\t .. Creating Multisampling ..");

        _multisampling = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE
            };

        return *this;
    }


    ///////////////////
    // DEPTH STENCIL //
    ///////////////////

Gateway& Gateway::depthStencil()
    {
        report(LOGGER::VLINE, "\t .. Creating Depth Stencil ..");

        _depth_stencil = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
                .depthCompareOp = VK_COMPARE_OP_LESS,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
                .front = {},
                .back = {},
                .minDepthBounds = 0.0f,
                .maxDepthBounds = 1.0f
            };

        return *this;
    }


    /////////////////
    // COLOR BLEND //
    /////////////////

static VkPipelineColorBlendAttachmentState colorBlendAttachment()
    {
        return {
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            };
    }

Gateway& Gateway::colorBlending()
    {
        report(LOGGER::VLINE, "\t .. Creating Color Blend State ..");

        _color_blend_attachment = 
            {
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            };

        _color_blending = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &_color_blend_attachment,
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
            };

        return *this;
    }


    ///////////////////
    // DYNAMIC STATE //
    ///////////////////

Gateway& Gateway::dynamicState()
    {
        report(LOGGER::VLINE, "\t .. Creating Dynamic State ..");

        _dynamic_state = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = static_cast<uint32_t>(_dynamic_states.size()),
                .pDynamicStates = _dynamic_states.data()
            };

        return *this;
    }


    ////////////
    // LAYOUT //
    ////////////

Gateway& Gateway::layout(VkDevice* logical_device)
    {
        report(LOGGER::VLINE, "\t .. Creating Pipeline Layout ..");

        _pipeline_layout_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 0,
                .pSetLayouts = nullptr,
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr
            };

        VK_TRY(vkCreatePipelineLayout(*logical_device, &_pipeline_layout_info, nullptr, &pipeline_layout));

        return *this;
    }

Gateway& Gateway::pipe(VkRenderPass* render_pass)
    {
        report(LOGGER::VLINE, "\t .. Creating Pipeline Create Info ..");

        _pipeline_info = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = static_cast<uint32_t>(_shader_stages.size()),
                .pStages = _shader_stages.data(),
                .pVertexInputState = &_vertex_input_state,
                .pInputAssemblyState = &_input_assembly,
                .pViewportState = &_viewport_state,
                .pRasterizationState = &_rasterizer,
                .pMultisampleState = &_multisampling,
                .pColorBlendState = &_color_blending,
                .pDynamicState = &_dynamic_state,
                .layout = pipeline_layout,
                .renderPass = *render_pass,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE,
            };

        return *this;
    }

Gateway& Gateway::create(VkDevice* logical_device)
    {
        report(LOGGER::VLINE, "\t .. Constructing Pipeline ..");
        VK_TRY(vkCreateGraphicsPipelines(*logical_device, VK_NULL_HANDLE, 1, &_pipeline_info, nullptr, &pipeline));

        report(LOGGER::VLINE, "\t .. Cleaning Up Shader Modules ..");
        for (auto shader_module : _shader_modules) 
            { vkDestroyShaderModule(*logical_device, shader_module, nullptr); }

        return *this;
    }

