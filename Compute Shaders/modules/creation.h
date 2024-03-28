#pragma once
#include "./reality/matrix.h"
#include "../components/lexicon.h"
#include <string>

class Existence {
    public:
        static Existence* manifest();   // Singleton

        static void materialize();      // Draw
        void actualize();               // Run
        void cease();                   // Cleanup
    
    private:
        std::string _application_name = "Compute Shaders";
        VkExtent2D _window_extent { 1660, 1440 };

        Reality* _actuality;

        Existence* create();            // Init
};