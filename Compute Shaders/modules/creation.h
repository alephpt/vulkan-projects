#pragma once
#include "./reality/matrix.h"
#include "../components/lexicon.h"
#include <string>

class Existence {
    public:
        Existence();
        ~Existence();

        static Existence* manifest();   // Singleton

        static void materialize();      // Draw
        void actualize();               // Run
        void cease();                   // Cleanup
    
    private:
        std::string _application_name;
        VkExtent2D _window_extent;
        Reality* _actuality;

        Existence* realize();            // Init
};