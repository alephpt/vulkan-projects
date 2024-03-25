#pragma once
#include "../components/generics.h"

class Existence {
    public:
        bool _initialized = false;
        int _frame_ct = 0;
        VkExtent2D _window_extent { 1660, 1440 };
        struct SDL_Window* _window = nullptr;

        void manifest();    // Init
        void materialize();      // Draw
        void actualize();   // Run
        void cease();       // Cleanup
};