#pragma once
#include "../../components/lexicon.h"
#include "./atomic.h"
#include "./gateway.h"

#include <string>
#include <future>


class Reality {
    public:
        bool initialized = false;
        FrameData& current_frame();

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


        Gateway* _gateway;

        void init_framework();
        void init_swapchain(std::future<void>&&, std::promise<void>&&);
        void init_pipeline(std::promise<void>&&);
        void init_commands();
        void init_sync_structures();
};