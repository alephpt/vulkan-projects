#include "creation.h"

#include <cassert>
#include <functional>
#include <SDL2/SDL_vulkan.h>

Existence* _essence = nullptr;

Existence::Existence() 
    {
        report(LOGGER::INFO, "Existence - Constructing Reality ..");
        assert(_essence == nullptr);
        
        _application_name = "Compute Shaders";
        _window_extent = { 1600, 1200 };
        _actuality = nullptr;
    }

// Singleton to ensure only one instance of Existence is created.
Existence* Existence::manifest()
    {
        report(LOGGER::INFO, "Existence - Manifesting ..");

        if (_essence == nullptr) {
            Existence* essence = new Existence();
            _essence = essence->realize();
        }

        return _essence;
    }

Existence* Existence::realize() 
    {
        // Initialize the Reality with Genesis 
        report(LOGGER::INFO, "Existence - Reality Complete ..");

        _actuality = new Reality(_application_name, _window_extent);
        _actuality->initialized = true;

        return this;
    }

void Existence::materialize() 
    {
       report(LOGGER::VERBOSE, "Existence - Materializing ..");
       // This is where we need to handle the real time calculations and state changes
       // TODO: Implement Menu and User Input Handling and Processing

        return;
    }

void Existence::actualize() 
    {
        report(LOGGER::INFO, "Existence - Actualizing ..");

        _actuality->illuminate();
        //_actuality->illuminate(Existence::materialize);   
    }

// Why aren't we using the destructor to clean up?
void Existence::cease() 
    {
        report(LOGGER::INFO, "Existence - Ceasing ..");

        delete _actuality;

        _essence = nullptr;
    }