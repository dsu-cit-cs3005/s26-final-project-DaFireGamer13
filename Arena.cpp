#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "Arena.h"

// Compile the file into a shared library - put this in a loop that traverses an array of all the robot.cpp files...
std::string compile_cmd = "g++ -shared -fPIC -o " + shared_lib + " " + filename + " RobotBase.o -I. -std=c++20";
std::cout << "Compiling " << filename << " to " << shared_lib << "...\n";

int compile_result = std::system(compile_cmd.c_str());
if (compile_result != 0)
{
    std::cerr << "Failed to compile " << filename << " with command: " << compile_cmd << std::endl;
    continue;
}

//  ...

//to load the shared objects:
void* handle = dlopen(shared_lib.c_str(), RTLD_LAZY);
if (!handle)
{
    std::cerr << "Failed to load " << shared_lib << ": " << dlerror() << std::endl;
    continue;
}

// to Locate the factory function to create the robot (note: RobotBase has a typedef for RobotFactory... go look at it!)
RobotFactory create_robot = (RobotFactory)dlsym(handle, "create_robot");
if (!create_robot)
{
    std::cerr << "Failed to find create_robot in " << shared_lib << ": " << dlerror() << std::endl;
    dlclose(handle);
    continue;
}

// To Instantiate the robot, you call the function that you LOADED from the shared library:
// you'll want to ensure that this function worked and then add this robot to a vector of robots.
// once you've gotten the robot, do with it what you like, but this is how you actually call it:

RobotBase* robot = create_robot();
//

//
