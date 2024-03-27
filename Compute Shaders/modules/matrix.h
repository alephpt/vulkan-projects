#include "../components/lexicon.h"

#include <string>

class Reality {
    public:
        VkInstance _instance;
        VkDebugUtilsMessengerEXT _debug_messenger;
        VkPhysicalDevice _physical_gpu;
        VkDevice _logical_gpu;
        VkQueue _graphics_queue;
        VkQueue _present_queue;
        VkSurfaceKHR _surface;
        VkSwapchainKHR _swapchain;

        Reality(std::string name);
        ~Reality();
    private:
        void init_framework(std::string name);
        void init_swapchain();
        void init_commands();
        void init_sync_structures();
};