#include "../../core.h"
#include "../00atomic/particle.h"

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

static inline VkDescriptorSetLayoutCreateInfo _getLayoutInfo(std::vector<VkDescriptorSetLayoutBinding>& bindings)
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

        std::vector<VkDescriptorSetLayoutBinding> _layout_binding = {
            _ubo_layout_binding,
            _sampler_layout_binding
        };
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

static inline VkDescriptorSetLayoutBinding constructComputeDescriptorSetLayoutBinding(uint32_t i)
    {
        report(LOGGER::VLINE, "\t\t .. Constructing Compute Descriptor Set Layout Binding ..");

        return {
            .binding = i,
            .descriptorType = (i == 0) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = nullptr
        };
    }

// TODO: Create a more dynamic way to create variable descriptor set layouts
void NovaCore::createComputeDescriptorSetLayout()
    {
        report(LOGGER::VLINE, "\t .. Creating Compute Descriptor Set Layout ..");

        std::vector<VkDescriptorSetLayoutBinding> _layout_binding;

        for (size_t i = 0; i < 3; i++)
            { _layout_binding[i] = constructComputeDescriptorSetLayoutBinding(i); }

        VkDescriptorSetLayoutCreateInfo _layout_info = _getLayoutInfo(_layout_binding);

        VK_TRY(vkCreateDescriptorSetLayout(logical_device, &_layout_info, nullptr, &compute_descriptor.layout));
    }

// TODO: Combine all the Descriptor Write Functions with a default parameter set and a 'constructor' function
static inline VkWriteDescriptorSet _getStorageDescriptorWrite(VkDescriptorSet* set, VkDescriptorBufferInfo* buffer_info, uint32_t dst_binding)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Descriptor Write ..");

        return {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = *set,
            .dstBinding = dst_binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo = buffer_info,
        };
    }

void NovaCore::createComputeDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, compute_descriptor.layout);
        VkDescriptorSetAllocateInfo _alloc_info = _getDescriptorSetAllocateInfo(static_cast<uint32_t>(MAX_COMPUTE_QUEUES), &descriptor.pool, layouts);

        compute_descriptor.sets.resize(MAX_FRAMES_IN_FLIGHT);
        VK_TRY(vkAllocateDescriptorSets(logical_device, &_alloc_info, compute_descriptor.sets.data()));

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkDescriptorBufferInfo _buffer_info = _getDescriptorBufferInfo(&uniform[i].buffer, sizeof(MVP));

                std::array<VkWriteDescriptorSet, 3> _write_descriptor{};
                _write_descriptor[0] = _getUniformDescriptorWrite(&compute_descriptor.sets[i], &_buffer_info);

                VkDescriptorBufferInfo _last_storage_buffer_info = _getDescriptorBufferInfo(&storage[(i - 1) % MAX_FRAMES_IN_FLIGHT].buffer, sizeof(Particle) * MAX_PARTICLES);
                _write_descriptor[1] = _getStorageDescriptorWrite(&compute_descriptor.sets[i], &_last_storage_buffer_info, 1);

                VkDescriptorBufferInfo _current_storage_buffer_info = _getDescriptorBufferInfo(&storage[i].buffer, sizeof(Particle) * MAX_PARTICLES);
                _write_descriptor[2] = _getStorageDescriptorWrite(&compute_descriptor.sets[i], &_current_storage_buffer_info, 2);

                vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(_write_descriptor.size()), _write_descriptor.data(), 0, nullptr);
            }
    }   