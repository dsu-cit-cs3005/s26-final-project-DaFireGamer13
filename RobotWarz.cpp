#include <iostream>
#include <vector>
#include <dlfcn.h>   // For dlopen, dlsym
#include <filesystem> // To find .cpp files easily
#include "Arena.h"

namespace fs = std::filesystem;

int main() {
    // 1. Setup the Arena (Example: 20x20 board)
    Arena arena(20, 20);
    
    // 2. Find and Load Robots
    std::vector<void*> handles; // To close them later
    
    for (const auto& entry : fs::directory_iterator(".")) {
        std::string filename = entry.path().filename().string();
        
        // Only look for files like "Robot_Incinerator.cpp"
        if (filename.find("Robot_") == 0 && entry.path().extension() == ".cpp") {
            
            std::string robot_name = entry.path().stem().string();
            std::string shared_lib = "./lib" + robot_name + ".so";

            // Compile the shared library
            std::string compile_cmd = "g++ -shared -fPIC -o " + shared_lib + 
                                      " " + filename + " RobotBase.o -I. -std=c++20";
            
            if (std::system(compile_cmd.c_str()) != 0) {
                std::cerr << "Failed to compile: " << filename << std::endl;
                continue;
            }

            // Load the library
            void* handle = dlopen(shared_lib.c_str(), RTLD_LAZY);
            if (!handle) {
                std::cerr << "Cannot load: " << dlerror() << std::endl;
                continue;
            }
            handles.push_back(handle);

            // Locate factory function
            RobotFactory create_robot = (RobotFactory)dlsym(handle, "create_robot");
            if (!create_robot) {
                std::cerr << "No factory found in " << filename << std::endl;
                dlclose(handle);
                continue;
            }

            // Create robot and add to Arena
            RobotBase* robot = create_robot();
            // You'll want a way to pick unique start spots
            int start_r = std::rand() % 20;
            int start_c = std::rand() % 20;
            arena.add_robot(robot, start_r, start_c);
        }
    }
    // 3. The Game Loop
    int turn = 1;
    while (!arena.is_game_over() && turn < 500) {
        std::cout << "--- TURN " << turn << " ---" << std::endl;
        arena.play_turn();
        arena.display_board();
        turn++;

        // Add a small delay or wait for user input so it doesn't fly by
        // std::cin.get();
	usleep(10000);
    }

    std::cout << "Game Over!" << std::endl;

    // 4. Cleanup
    for (void* h : handles) {
        dlclose(h); // Close the shared libs
    }

    return 0;
}
