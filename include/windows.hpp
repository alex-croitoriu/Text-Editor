#pragma once

#include <string>

namespace Windows
{
    std::string getPathFromUser(std::string name);
    std::string getStringFromUser(std::string name);
    void throwMessage(std::string message);
    std::string saveAS();
    std::string open();
    int saveModal();
}