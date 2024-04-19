#pragma once
#include "./engine/engine.h"
#include <string>

class  Nova {
    public:
         Nova();
        ~ Nova();

        static  Nova* manifest();   // Singleton

        static void materialize();      // Draw
        void actualize();               // Run
    
    private:
        std::string _application_name;
        VkExtent2D _window_extent;
        NovaEngine* _engine;

         Nova* realize();            // Init
};