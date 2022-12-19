#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <vector>
#include <map>

class HelloTriangle {
    public:
        void run () {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
        GLFWwindow* window;
        const uint32_t WIDTH = 800;
        const uint32_t HEIGHT = 600;
        VkInstance instance;

        struct QueueFamilyIndices {
            std::optional<uint32_t> gfxFamily;

            bool isComplete () {
                return gfxFamily.has_value();
            }
        };
        
        // Window instantiation

        void initWindow () {
            glfwInit();
            
            if (glfwVulkanSupported() != GLFW_TRUE) {
                printf("Vulkan GLFW not supported\n");
                exit(-1);
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(WIDTH, HEIGHT, "Testing", nullptr, nullptr);
        }

        std::vector<const char*> getRequiredExtensions () {
            uint32_t glfwExtensionCt = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCt);
            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCt);

            return extensions;
        }

        // Application instantiation

        void createInstance () {
            VkApplicationInfo appInfo{};
            VkInstanceCreateInfo createInfo{};
            
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;
 
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledLayerCount = 0;

            auto extensions = getRequiredExtensions();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

        }

        // physical device instantiation

        bool deviceSuitability (VkPhysicalDevice device) {
            int score = 0;
            VkPhysicalDeviceProperties devP;
            VkPhysicalDeviceFeatures devF;
            vkGetPhysicalDeviceProperties(device, &devP);
            vkGetPhysicalDeviceFeatures(device, &devF);

            if (devP.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                score += 1000;
            }

            score += devP.limits.maxImageDimension2D;

            if (!devF.geometryShader) {
                return 0;
            }

            return score;
        }

        void initPhysicalDevice () {
            std::multimap<int, VkPhysicalDevice> candidates;
            VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
            uint32_t deviceCount = 0;

            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("Failed to find GPU with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            for (const auto& device : devices) {
                int score = deviceSuitability(device);
                candidates.insert(std::make_pair(score, device));
            }

            if (candidates.rbegin()->first > 0) {
                physicalDevice = candidates.rbegin()->second;
            } else {
                throw std::runtime_error("Failed to find suitable GPU");
            }
        }

        // find queue families

        QueueFamilyIndices findQueueFamilies (VkPhysicalDevice device) {
            QueueFamilyIndices indices;

            return indices;
        }


        // vulkan initialization steps

        void initVulkan () {
            createInstance();
            initPhysicalDevice();
        }

        // main vulkan api loop

        void mainLoop () {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
            }
        }

        // exit

        void cleanup () {
            vkDestroyInstance(instance, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();
        }
};

int main () {
    HelloTriangle hello;

    try {
        hello.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
