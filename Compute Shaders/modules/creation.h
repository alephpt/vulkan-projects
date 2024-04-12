#pragma once
#include "./graphics/graphics.h"
#include "../components/lexicon.h"
#include <string>

class Existence {
    public:
        Existence();
        ~Existence();

        static Existence* manifest();   // Singleton

        static void materialize();      // Draw
        void actualize();               // Run
    
    private:
        std::string _application_name;
        VkExtent2D _window_extent;
        Graphics* _actuality;

        Existence* realize();            // Init
};