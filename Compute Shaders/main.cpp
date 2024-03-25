/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description: Trying to Triangulate in Vulkan
 *
 *        Version:  1.0
 *        Created:  07/29/20 20:30:24
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Richard Isaac
 *   Organization:  NeoTec, LLC
 *
 * =====================================================================================
 */
#include <iostream>
#include "modules/manifest.h"

int main(int argc, char* argv[]) {
    std::cout << "Creating Vulkan Renderer." << std::endl;

    Existence essence;
    essence.manifest();
    essence.actualize();
    essence.cease();
    
    return 0;
}