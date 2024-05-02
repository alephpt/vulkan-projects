#include "../../core.h"
#include <cstring>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

    /////////////////////////////
    // UNIFORM BUFFER CREATION //
    /////////////////////////////

static const VkBufferUsageFlags _uniform_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
static const VkMemoryPropertyFlags _uniform_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

void NovaCore::constructUniformBuffer() 
    {
        report(LOGGER::VLINE, "\t .. Creating Uniform Buffer ..");

        VkDeviceSize _buffer_size = sizeof(MVP);

        uniform.resize(MAX_FRAMES_IN_FLIGHT);
        uniform_data.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                createBuffer(_buffer_size, _uniform_usage, _uniform_properties, &uniform[i]);
                vkMapMemory(logical_device, uniform[i].memory, 0, _buffer_size, 0, &uniform_data[i]);
                queues.deletion.push_fn([=]() { 
                    vkUnmapMemory(logical_device, uniform[i].memory); 
                    destroyBuffer(&uniform[i]);
                    report(LOGGER::VLINE, "\t .. Uniform Buffer Destroyed ..");
                });
            }
    }

void NovaCore::updateUniformBuffer(uint32_t current_frame)
    {
        // MVP for Vertex/Index Buffers
        // static auto _s_t = std::chrono::high_resolution_clock::now();
        // auto _c_t = std::chrono::high_resolution_clock::now();
        // float _e_t = std::chrono::duration<float, std::chrono::seconds::period>(_c_t - _s_t).count();

        // // Model Coordinate System controls the position of the object in the world
        // // View Coordinate System controls the position of the camera in the world
        // // Projection Coordinate System controls the perspective of the camera

        // MVP _mvp = {
        //     .model = glm::rotate(glm::mat4(1.0f), _e_t * glm::radians(65.0f), glm::vec3(0.125f, 0.825f, 1.0f)),                         // This sets the model matrix to rotate around the z-axis 
        //     .view = glm::lookAt(glm::vec3(1.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),             // This sets the view matrix to look at the origin from the z-axis
        //     .proj = glm::perspective(glm::radians(45.0f), swapchain.details.extent.width / (float)swapchain.details.extent.height, 0.1f, 10.0f),    // This sets the projection matrix to a perspective view
        // };

        // _mvp.proj[1][1] *= -1; // This flips the y-axis

        UBO_T ubo{};
        ubo.deltaTime = last_frame_time / 3.0f;

        memcpy(uniform_data[current_frame], &ubo, sizeof(UBO_T));
    }
