#include "../../core.h"

    ///////////////////////////
    // PIPELINE CONSTRUCTION //
    ///////////////////////////

void NovaCore::destroyPipeline(Pipeline* pipeline)
    {
        report(LOGGER::DEBUG, "Management - Destroying Pipeline.");
        vkDestroyPipeline(logical_device, pipeline->instance, nullptr);
        vkDestroyPipelineLayout(logical_device, pipeline->layout, nullptr);
        delete pipeline;
        return;
    }

void NovaCore::constructGraphicsPipeline()
    { 
        report(LOGGER::DEBUG, "Management - Constructing Graphics Pipeline .."); 
        
        graphics_pipeline = new Pipeline();

        graphics_pipeline->shaders(&logical_device)
                .vertexInput()
                .inputAssembly()
                .viewportState()
                .rasterizer()
                .multisampling(msaa_samples)
                .colorBlending()
                .dynamicState()
                .createLayout(&logical_device, &descriptor.layout)
                .pipe(&render_pass)
                .create(&logical_device);

        return; 
    }
    
void NovaCore::constructComputePipeline()
    { 
        report(LOGGER::DEBUG, "Management - Constructing Compute Pipeline .."); 
        
        return; 
    }
