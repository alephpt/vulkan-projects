#pragma once
#include "../../components/lexicon.h"
#include "./atomic.h"
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
        Queues _queues;

        FrameData _frames[MAX_FRAMES_IN_FLIGHT];
        FrameData& _current_frame();

        Reality(std::string name, struct SDL_Window* window);
        ~Reality();

    private:
        int _frame_ct = 0;

        void init_framework(std::string name, struct SDL_Window* window);
        void init_swapchain();
        void init_commands();
        void init_sync_structures();
};