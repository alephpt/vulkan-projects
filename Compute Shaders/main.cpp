/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description: Trying to Triangulate in Vulkan
 *
 *        Version:  0.1.0
 *        Created:  2024-03-25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Richard Isaac
 *   Organization:  NeoTec, LLC
 *
 * =====================================================================================
 */
#include <iostream>
#include "modules/creation.h"
#include "components/logger.h"

int main(int argc, char* argv[]) {
    report(LOGGER::VLINE, "Manifestation Exists.");

    Existence* creation = Existence::manifest();
    creation->actualize();
    creation->cease();
    
    return 0;
}