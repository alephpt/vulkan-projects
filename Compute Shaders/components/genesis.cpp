#include "genesis.h"
#include "./lexicon.h"

std::vector<char> genesis::loadFile(const std::string& filename) 
    {
        report(LOGGER::VERBOSE, "Genesis - Creating Buffer from %s ..", filename.c_str());
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) 
            { throw std::runtime_error("failed to open file!"); }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }