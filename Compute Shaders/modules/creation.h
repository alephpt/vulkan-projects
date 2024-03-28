#pragma once
#include "./reality/matrix.h"
#include "../components/lexicon.h"
#include <string>

class Existence {
    public:
        static Existence* manifest();   // Singleton

        void actualize();               // Run
        void cease();                   // Cleanup
    
    private:
        std::string _application_name = "Compute Shaders";
        LOGGER _debug_level = LOGGER::DEBUG;
        VkExtent2D _window_extent { 1660, 1440 };

        Reality* _actuality;

        void materialize();             // Draw
        Existence* create();            // Init
};