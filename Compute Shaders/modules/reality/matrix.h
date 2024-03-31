#pragma once
#include "../../components/lexicon.h"
#include "./atomic.h"
#include "./gateway.h"
#include <string>

class Reality {
    public:
        bool initialized = false;

        Reality(std::string, VkExtent2D);
        ~Reality();

        //void illuminate();
        void illuminate(fnManifest);

    private:
        std::string _application_name;
        bool _suspended = false;
        int _frame_ct = 0;

        struct SDL_Window* _window = nullptr;

        EngineContext _context;
        VkDebugUtilsMessengerEXT _debug_messenger;

        FrameData _frames[MAX_FRAMES_IN_FLIGHT];
        FrameData& _current_frame();

        Gateway* _gateway;

        void init_framework();
        void init_swapchain();
        void init_pipeline();
        void init_commands();
        void init_sync_structures();
};