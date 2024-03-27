#include "../components/lexicon.h"

#include <string>

constexpr unsigned int MAX_FRAMES_IN_FLIGHT = 2;

struct FrameData {
    VkSemaphore _image_available;
    VkSemaphore _render_finished;
    VkFence _in_flight;
    VkCommandBuffer _command_buffer;
    VkCommandPool _command_pool;
};

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

        FrameData _frames[MAX_FRAMES_IN_FLIGHT];
        FrameData& _current_frame();

        Reality(std::string name);
        ~Reality();

    private:
        int _frame_ct = 0;

        void init_framework(std::string name);
        void init_swapchain();
        void init_commands();
        void init_sync_structures();
};