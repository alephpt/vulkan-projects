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

#include <thread>
#include <future>

void fA1() { printf("Function A1\n"); }
void fA2() { printf("Function A2\n"); }
void fA3() { printf("Function A3\n"); }
void fA4() { printf("Function A4\n"); }

void fB1() { printf("Function B1\n"); }
void fB2() { printf("Function B2\n"); }
void fB3() { printf("Function B3\n"); }


void fA(std::promise<void>& waitForB) {
    printf("Function A\n");
    fA1();
    fA2();
    fA3();
    waitForB.set_value();   // Notify fB that fA has completed its work
    fA4();
    printf("Function A Done\n");
}

void fB(std::future<void>& waitForA) {
    printf("Function B\n");
    fB1();
    waitForA.wait();     // Wait for fA to complete its work
    fB2();
    fB3();
    printf("Function B Done\n");
}

// Main Function where we handle FA and FB to run in parallel and wait for each other
int main(int argc, char* argv[]) {
    report(LOGGER::VLINE, "Manifestation Exists.");

    std::promise<void> waitForB;                        // Create a promise that fA will signal fB
    std::future<void> waitForA = waitForB.get_future(); // Get the future from the promise

    std::thread threadB(fB, std::ref(waitForA));
    std::thread threadA(fA, std::ref(waitForB));

    threadA.join();
    threadB.join();

/*
    Existence* creation = Existence::manifest();
    creation->actualize();
    creation->cease();
  */  
    return 0;
}