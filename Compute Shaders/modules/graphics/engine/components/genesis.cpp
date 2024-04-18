#include "genesis.h"
#include "./lexicon.h"
#include <fstream>

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

static inline glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t)
    {
        return a + t * (b - a);
    }

static const glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
static const glm::vec3 green = glm::vec3(0.0f, 1.0f, 0.0f);
static const glm::vec3 blue = glm::vec3(0.0f, 0.0f, 1.0f);
static const glm::vec3 yellow = glm::vec3(1.0f, 1.0f, 0.0f);

static const glm::vec3 p1 = glm::vec3(-0.5f, 0.5f, 0.5f);   // top left front
static const glm::vec3 p2 = glm::vec3(0.5f, 0.5f, 0.5f);    // top right front
static const glm::vec3 p3 = glm::vec3(0.5f, -0.5f, 0.5f);   // bottom right front
static const glm::vec3 p4 = glm::vec3(-0.5f, -0.5f, 0.5f);  // bottom left front
static const glm::vec3 p5 = glm::vec3(-0.5f, 0.5f, -0.5f);  // top left back
static const glm::vec3 p6 = glm::vec3(0.5f, 0.5f, -0.5f);   // top right back
static const glm::vec3 p7 = glm::vec3(0.5f, -0.5f, -0.5f);  // bottom right back
static const glm::vec3 p8 = glm::vec3(-0.5f, -0.5f, -0.5f); // bottom left back


              /////////////////////////////////////////////////////////////////////////
            //                                                                      ///
          //                                                                       ////
        //////////////////////////////////////////////////////////////////////////// //
        //                                                                       //  //
        //        /P5------P6            /7--------6            /R--------B      //  //
        //      P1------P2/ |           3--------2/|           R--------B/|      //  //
        //      | |      |  |           | |      | |           | |      | |      //  //
        //      | P8_____|_P7           | 4______|_5           | Y______|_G      // //
        //      P4/_____P3/             0/_______1/            Y/_______G/       ////
        //                                                                       ///
        //                                                                       //
        //////////////////////////////////////////////////////////////////////////

void genesis::createObjects(std::vector<Vertex>* vertices, std::vector<uint32_t>* indices) 
    {
        std::vector<glm::vec3> positions = {};
        std::vector<glm::vec3> colors = {};

        vertices->push_back({p4, yellow});  // 0
        vertices->push_back({p3, green});   // 1
        vertices->push_back({p2, blue});    // 2
        vertices->push_back({p1, red});     // 3
        vertices->push_back({p8, yellow});  // 4
        vertices->push_back({p7, green});   // 5
        vertices->push_back({p6, blue});    // 6
        vertices->push_back({p5, red});     // 7

        // front
        indices->push_back(0);  // front bottom left
        indices->push_back(1);  // front bottom right
        indices->push_back(2);  // front top right
        indices->push_back(0);  // front bottom left
        indices->push_back(2);  // front top right
        indices->push_back(3);  // front top left

        // back
        indices->push_back(5);  // back bottom left
        indices->push_back(4);  // back bottom right
        indices->push_back(7);  // back top right
        indices->push_back(5);  // back bottom left
        indices->push_back(7);  // back top right
        indices->push_back(6);  // back top left

        // bottom
        indices->push_back(0);  // front bottom left
        indices->push_back(5);  // back bottom left
        indices->push_back(1);  // back bottom right
        indices->push_back(4);  // front bottom left
        indices->push_back(5);  // back bottom right
        indices->push_back(0);  // front bottom right

        // right
        indices->push_back(1);  // front bottom right
        indices->push_back(5);  // back bottom right
        indices->push_back(6);  // back top right
        indices->push_back(1);  // front bottom right
        indices->push_back(6);  // back top right
        indices->push_back(2);  // front top right

        // top
        indices->push_back(2);  // front top right
        indices->push_back(6);  // back top right
        indices->push_back(7);  // back top left
        indices->push_back(2);  // front top right
        indices->push_back(7);  // back top left
        indices->push_back(3);  // front top left

        // left
        indices->push_back(3);  // front top left
        indices->push_back(7);  // back top left
        indices->push_back(4);  // back bottom left
        indices->push_back(3);  // front top left
        indices->push_back(4);  // back bottom left
        indices->push_back(0);  // front bottom left
    }
