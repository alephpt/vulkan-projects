#pragma once
#include "../../components/lexicon.h"
#include "./architect.h"

#include <string>
#include <future>


class Reality {
    public:
        bool initialized = false;

        Reality(std::string, VkExtent2D);
        ~Reality();

        void illuminate();
        //void illuminate(fnManifest);

    private:
        std::string _application_name;
        bool _suspended = false;
        VkExtent2D _window_extent;
        struct SDL_Window* _window = nullptr;
        Architect* _architect;

        VkDebugUtilsMessengerEXT _debug_messenger;

        void _initFramework();
        void _initSwapChain(std::promise<void>&, std::future<void>&, std::promise<void>&);
        void _initGateway(std::future<void>&, std::promise<void>&);
        void _initBuffers();
        void _initSyncStructures();
        void _resizeWindow(int, int);
};
