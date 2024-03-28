#pragma once
#include "./reality/matrix.h"

class Existence {
    public:
        VkExtent2D _window_extent { 1660, 1440 };
        Reality* _actuality;
        struct SDL_Window* _window = nullptr;

        static Existence* manifest();   // Singleton

        void materialize();             // Draw
        void actualize();               // Run
        void cease();                   // Cleanup
    
    private:
        std::string _application_name = "Compute Shaders";
        bool _initialized = false;
        bool _suspended = false;

        Existence* create();            // Init
};