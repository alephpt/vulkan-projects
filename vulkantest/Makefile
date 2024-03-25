VULKAN_SDK_PATH = /user/include/vulkan

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)
LDFLAGS = -L$(VULKAN_SDK_PATH) `pkg-config --static --libs glfw3` -lvulkan -ldl -pthread

VulkanTest: main.cpp
	g++ $(CFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH) VK_LAYER_PATH=$/etc/vulkan/explicit_layer.d ./VulkanTest

clean:
	rm -f VulkanTest

