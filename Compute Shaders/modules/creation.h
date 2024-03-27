#pragma once
#include "../components/lexicon.h"

class Existence {
    public:
        bool _initialized = false;
        int _frame_ct = 0;
        VkExtent2D _window_extent { 1660, 1440 };
        struct SDL_Window* _window = nullptr;

        static Existence &manifest();   // Singleton

        void materialize();             // Draw
        void actualize();               // Run
        void cease();                   // Cleanup
    
    private:
        Existence* create();            // Init
};