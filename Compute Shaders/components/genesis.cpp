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

static const glm::vec3 p1 = glm::vec3(-0.5f, 0.5f, 0.0f);
static const glm::vec3 p2 = glm::vec3(0.5f, 0.5f, 0.0f);
static const glm::vec3 p3 = glm::vec3(0.5f, -0.5f, 0.0f);
static const glm::vec3 p4 = glm::vec3(-0.5f, -0.5f, 0.0f);

static int subdivisions = 332;

static inline glm::vec3 randomColor() 
    {
        // picks a random color from the predefined colors (red, green, blue, yellow)
        int random = rand() % 4;

        switch (random) 
            {
                case 0:
                    return red;
                case 1:
                    return green;
                case 2:
                    return blue;
                case 3:
                    return yellow;
                default:
                    return red;
            }
    }

void genesis::createObjects(std::vector<Vertex>* vertices, std::vector<uint32_t>* indices) 
    {
        std::vector<glm::vec3> positions = {};
        std::vector<glm::vec3> colors = {};

        if (subdivisions <= 1) 
            {
                vertices->push_back({p4, yellow}); 
                vertices->push_back({p3, green});
                vertices->push_back({p2, blue});
                vertices->push_back({p1, red});

                // 4, 3, 2
                // 4, 2, 1
                indices->push_back(0);
                indices->push_back(1);
                indices->push_back(2);
                indices->push_back(0);
                indices->push_back(2);
                indices->push_back(3);
            }
        else 
            {
                for (int i = 0; i < subdivisions; i++) 
                    {
                        float t = (float) i / (subdivisions - 1);
                        glm::vec3 a = lerp(p4, p1, t);
                        glm::vec3 b = lerp(p3, p2, t);

                        for (int j = 0; j < subdivisions; j++) 
                            {
                                // interpolate between a and b
                                float s = (float) j / (subdivisions - 1);
                                glm::vec3 p = lerp(a, b, s);
                                positions.push_back(p);
                                colors.push_back(randomColor());

                                // create indices for the quad
                                int p0 = i * subdivisions + j;
                                int p1 = p0 + 1;
                                int p2 = (i + 1) * subdivisions + j + 1;
                                int p3 = (i + 1) * subdivisions + j;

                                indices->push_back(p0);
                                indices->push_back(p1);
                                indices->push_back(p2);
                                indices->push_back(p2);
                                indices->push_back(p3);
                                indices->push_back(p0);
                            }
                    }

                // create vertices from positions and colors
                for (int i = 0; i < positions.size(); i++) 
                    { vertices->push_back({positions[i], colors[i]}); }
            }
    }