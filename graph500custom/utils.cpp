#include "utils.h"

#include <iostream>
#include <string>
#include <sstream>



bool getint(int &value)
{
    std::string input = " ";

    while (true)
    {
        getline(std::cin, input);
        std::stringstream inputstream(input);
        if (inputstream >> value)
        {
            break;
        }
        std::cout << "Enter a number fool" << std::endl;
    }
    return true;
}
