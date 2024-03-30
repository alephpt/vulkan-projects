#include "creation.h"

#include <cassert>
#include <functional>
#include <SDL2/SDL_vulkan.h>


Existence* _essence = nullptr;

// Singleton to ensure only one instance of Existence is created.
Existence* Existence::manifest()
    {
        report(LOGGER::INFO, "Existence - Manifesting ..");

        if (_essence == nullptr) {
            Existence essence;
            return essence.create();
        }

        return _essence;
    }

Existence* Existence::create() 
    {
        assert(_essence == nullptr);    // We don't ever want to re-initialize or our singleton did not work.
        _essence = this;

        report(LOGGER::INFO, "Existence - Constructing Reality ..");
        // Handles Vulkan Rendering Engine and Compute Buffers
        _actuality = new Reality(_application_name, _window_extent);

        // Initialize the Reality with Genesis 

        report(LOGGER::INFO, "Existence - Reality Complete ..");

        _actuality->initialized = true;

        return this;
    }

void Existence::materialize() 
    {
       // report(LOGGER::INFO, "Existence - Materializing ..");

        return;
    }

void Existence::actualize() 
    {
        report(LOGGER::INFO, "Existence - Actualizing ..");

        //_actuality->illuminate();
        _actuality->illuminate(Existence::materialize);
    }

// Why aren't we using the destructor to clean up?
void Existence::cease() 
    {
        report(LOGGER::INFO, "Existence - Ceasing ..");

        delete _actuality;

        _essence = nullptr;
    }