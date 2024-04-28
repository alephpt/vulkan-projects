#include "../../core.h"

    ///////////////////////////
    // DESCRIPTOR SET LAYOUT //
    ///////////////////////////

static inline VkDescriptorSetLayoutBinding _getVertexLayoutBinding()
    {
        return {
                binding: 0,
                descriptorType: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                descriptorCount: 1,
                stageFlags: VK_SHADER_STAGE_VERTEX_BIT,
                pImmutableSamplers: nullptr
            };
    }

static inline VkDescriptorSetLayoutBinding _getFragmentLayoutBinding()
    {
        return {
                binding: 1,
                descriptorType: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                descriptorCount: 1,
                stageFlags: VK_SHADER_STAGE_FRAGMENT_BIT,
                pImmutableSamplers: nullptr
            };
    }

static inline VkDescriptorSetLayoutCreateInfo _getLayoutInfo(std::array<VkDescriptorSetLayoutBinding, 2>& bindings)
    {
        return {
                sType: VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                bindingCount: static_cast<uint32_t>(bindings.size()),
                pBindings: bindings.data()
            };
    }

void NovaCore::createDescriptorSetLayout() 
    {
        report(LOGGER::VLINE, "\t .. Creating Descriptor Set Layout ..");

        VkDescriptorSetLayoutBinding _ubo_layout_binding = _getVertexLayoutBinding();
        VkDescriptorSetLayoutBinding _sampler_layout_binding = _getFragmentLayoutBinding();

        std::array<VkDescriptorSetLayoutBinding, 2> _layout_binding = {_ubo_layout_binding, _sampler_layout_binding};
        VkDescriptorSetLayoutCreateInfo _layout_info = _getLayoutInfo(_layout_binding);

        VK_TRY(vkCreateDescriptorSetLayout(logical_device, &_layout_info, nullptr, &descriptor.layout));

        return;
    }

static inline VkDescriptorPoolSize _getUniformPoolSize(uint32_t ct)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Pool Size of %d ..", ct);

        return {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = ct
        };
    }

static inline VkDescriptorPoolSize _getSamplerPoolSize(uint32_t ct)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Pool Size of %d ..", ct);

        return {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = ct
        };
    }

static inline VkDescriptorPoolCreateInfo _getPoolInfo(uint32_t ct, std::array<VkDescriptorPoolSize, 2>& sizes)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Pools Info with size %d ..", sizes.size());

        return {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = ct,
            .poolSizeCount = static_cast<uint32_t>(sizes.size()),
            .pPoolSizes = sizes.data()
        };
    }

void NovaCore::constructDescriptorPool() 
    {
        report(LOGGER::VLINE, "\t .. Constructing Descriptor Pool ..");

        std::array<VkDescriptorPoolSize, 2> _pool_size = {
            _getUniformPoolSize(MAX_FRAMES_IN_FLIGHT),
            _getSamplerPoolSize(MAX_FRAMES_IN_FLIGHT)
        };

        VkDescriptorPoolCreateInfo _pool_info = _getPoolInfo(MAX_FRAMES_IN_FLIGHT, _pool_size);

        VK_TRY(vkCreateDescriptorPool(logical_device, &_pool_info, nullptr, &descriptor.pool));

        return;
    }

static inline VkDescriptorSetAllocateInfo _getDescriptorSetAllocateInfo(uint32_t ct, VkDescriptorPool* pool, std::vector<VkDescriptorSetLayout>& layouts)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Set Allocate Info ..");

        return {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = *pool,
            .descriptorSetCount = ct,
            .pSetLayouts = layouts.data()
        };
    }

static inline VkDescriptorBufferInfo _getDescriptorBufferInfo(VkBuffer* buffer, VkDeviceSize size)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Buffer Info ..");

        return {
            .buffer = *buffer,
            .offset = 0,
            .range = size
        };
    }

static inline VkDescriptorImageInfo _getDescriptorImageInfo(ImageContext* texture)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Image Info ..");

        return {
            .sampler = texture->sampler,
            .imageView = texture->view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
    }

static inline VkWriteDescriptorSet _getUniformDescriptorWrite(VkDescriptorSet* set, VkDescriptorBufferInfo* buffer_info)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Write ..");

        return {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = *set,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = buffer_info,
            .pTexelBufferView = nullptr
        };
    }

static inline VkWriteDescriptorSet _getSamplerDescriptorWrite(VkDescriptorSet* set, VkDescriptorImageInfo* image_info)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Write ..");

        return {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = *set,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = image_info,
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr
        };
    }

void NovaCore::createDescriptorSets() 
    {
        report(LOGGER::VLINE, "\t .. Creating Descriptor Sets ..");

        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptor.layout);
        VkDescriptorSetAllocateInfo _alloc_info = _getDescriptorSetAllocateInfo(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT), &descriptor.pool, layouts);

        descriptor.sets.resize(MAX_FRAMES_IN_FLIGHT);

        VK_TRY(vkAllocateDescriptorSets(logical_device, &_alloc_info, descriptor.sets.data()));

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
            {
                VkDescriptorBufferInfo _buffer_info = _getDescriptorBufferInfo(&uniform[i].buffer, sizeof(MVP));
                VkDescriptorImageInfo _image_info = _getDescriptorImageInfo(&texture);
                
                std::array<VkWriteDescriptorSet, 2> _write_descriptor = {
                    _getUniformDescriptorWrite(&descriptor.sets[i], &_buffer_info),
                    _getSamplerDescriptorWrite(&descriptor.sets[i], &_image_info)
                };

                vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(_write_descriptor.size()), _write_descriptor.data(), 0, nullptr);
            }
    }

