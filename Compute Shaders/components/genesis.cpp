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

void genesis::populateVertices(std::vector<Vertex>* vertices) 
    {
        std::vector<glm::vec3> positions = {};
        std::vector<glm::vec3> colors = {};

        if (subdivisions <= 1) 
            {
                vertices->push_back({p4, yellow});
                vertices->push_back({p3, green});
                vertices->push_back({p2, blue});
                vertices->push_back({p4, yellow});
                vertices->push_back({p2, blue});
                vertices->push_back({p1, red});
            }
        else 
            {


                for (int i = 0; i < subdivisions; i++) 
                    {
                        for (int j = 0; j < subdivisions; j++) 
                            {
                                float x = (float) i / (subdivisions - 1);
                                float y = (float) j / (subdivisions - 1);

                                positions.push_back(lerp(lerp(p1, p2, x), lerp(p4, p3, x), y));
                                glm::vec3 random_color_1 = randomColor();
                                glm::vec3 random_color_2 = randomColor();

                                int random_push_back = rand() % 2;
                                if (random_push_back == 0) 
                                    {
                                        colors.push_back(lerp(lerp(red, random_color_1, x), lerp(green, random_color_2, y), 0.5f));
                                    }
                                else 
                                    {
                                        colors.push_back(lerp(lerp(random_color_1, green, x), lerp(blue, random_color_2, y), 0.5f));
                                    }
                            }
                    }

                for (int i = 0; i < subdivisions - 1; i++) 
                    {
                        for (int j = 0; j < subdivisions - 1; j++) 
                            {
                                vertices->push_back({positions[i * subdivisions + j], colors[i * subdivisions + j]});
                                vertices->push_back({positions[i * subdivisions + j + 1], colors[i * subdivisions + j + 1]});
                                vertices->push_back({positions[(i + 1) * subdivisions + j + 1], colors[(i + 1) * subdivisions + j + 1]});

                                vertices->push_back({positions[i * subdivisions + j], colors[i * subdivisions + j]});
                                vertices->push_back({positions[(i + 1) * subdivisions + j + 1], colors[(i + 1) * subdivisions + j + 1]});
                                vertices->push_back({positions[(i + 1) * subdivisions + j], colors[(i + 1) * subdivisions + j]});
                            }
                    }
            }
    }