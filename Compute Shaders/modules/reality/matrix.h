#pragma once
#include "../../components/lexicon.h"
#include "./atomic.h"
#include <string>

class Reality {
    public:
        bool initialized = false;

        Reality(std::string name, VkExtent2D window_extent);
        ~Reality();

        void illuminate();//fnManifest manifest);

    private:
        std::string _application_name;
        bool _suspended = false;
        int _frame_ct = 0;

        struct SDL_Window* _window = nullptr;
        VkExtent2D _window_extent { 800, 600 };

        VkInstance _instance;
        VkDebugUtilsMessengerEXT _debug_messenger;
        VkPhysicalDevice _physical_gpu;
        VkDevice _logical_gpu;
        VkQueue _graphics_queue;
        VkQueue _present_queue;
        VkSurfaceKHR _surface;
        VkSwapchainKHR _swapchain;

        Queues _queues;
        FrameData _frames[MAX_FRAMES_IN_FLIGHT];
        FrameData& _current_frame();

        void init_framework();
        void init_swapchain();
        void init_commands();
        void init_sync_structures();
};