/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description: Trying to Vulkan the Matrix
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

// Main Function where we handle FA and FB to run in parallel and wait for each other
int main(int argc, char* argv[]) {
    report(LOGGER::VLINE, "Manifestation Exists.");

    Existence* creation = Existence::manifest();    // Instantiate the World
    creation->actualize();                          // Run the Simulation
    creation->cease();                              // End the Simulation and Cleanup

    return 0;
}