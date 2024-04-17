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

        //      /P5------P6
        //    P1------P2/ |
        //    | |      |  |
        //    | P8_____|_P7
        //    P4/_____P3/

static int subdivisions = 0;

static void subdivide(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices) 
    {
        for (int i = 0; i < subdivisions; i++) 
            {
                glm::vec3 ab = lerp(a, b, (float) i / subdivisions);
                glm::vec3 bc = lerp(b, c, (float) i / subdivisions);
                glm::vec3 cd = lerp(c, d, (float) i / subdivisions);
                glm::vec3 da = lerp(d, a, (float) i / subdivisions);

                for (int j = 0; j < subdivisions; j++) 
                    {
                        glm::vec3 ab_bc = lerp(ab, bc, (float) j / subdivisions);
                        glm::vec3 bc_cd = lerp(bc, cd, (float) j / subdivisions);
                        glm::vec3 cd_da = lerp(cd, da, (float) j / subdivisions);
                        glm::vec3 da_ab = lerp(da, ab, (float) j / subdivisions);

                        glm::vec3 mid = lerp(ab_bc, bc_cd, 0.5f);

                        vertices->push_back({ab_bc, yellow});
                        vertices->push_back({bc_cd, green});
                        vertices->push_back({cd_da, blue});
                        vertices->push_back({da_ab, red});

                        uint32_t offset = vertices->size() - 4;
                        indices->push_back(offset);
                        indices->push_back(offset + 1);
                        indices->push_back(offset + 2);
                        indices->push_back(offset);
                        indices->push_back(offset + 2);
                        indices->push_back(offset + 3);
                    }
            }
    }

void genesis::createObjects(std::vector<Vertex>* vertices, std::vector<uint32_t>* indices) 
    {
        std::vector<glm::vec3> positions = {};
        std::vector<glm::vec3> colors = {};

        if (subdivisions < 1) 
            {
                vertices->push_back({p4, yellow}); 
                vertices->push_back({p3, green});
                vertices->push_back({p2, blue});
                vertices->push_back({p1, red});
                vertices->push_back({p8, yellow});
                vertices->push_back({p7, green});
                vertices->push_back({p6, blue});
                vertices->push_back({p5, red});

                // front
                indices->push_back(0);  // front bottom left
                indices->push_back(1);  // front bottom right
                indices->push_back(2);  // front top right
                indices->push_back(0);  // front bottom left
                indices->push_back(2);  // front top right
                indices->push_back(3);  // front top left

                // back
                indices->push_back(4);  // back bottom left
                indices->push_back(5);  // back bottom right
                indices->push_back(6);  // back top right
                indices->push_back(4);  // back bottom left
                indices->push_back(6);  // back top right
                indices->push_back(7);  // back top left

                // bottom
                indices->push_back(0);  // front bottom left
                indices->push_back(4);  // back bottom left
                indices->push_back(5);  // back bottom right
                indices->push_back(0);  // front bottom left
                indices->push_back(5);  // back bottom right
                indices->push_back(1);  // front bottom right

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
        else {
            for (int sides = 0; sides < 6; sides++) 
                {
                    glm::vec3 a, b, c, d;
                    switch (sides) 
                        {
                            case 0: a = p4; b = p3; c = p2; d = p1; break; // front
                            case 1: a = p8; b = p7; c = p6; d = p5; break; // back
                            case 2: a = p1; b = p2; c = p6; d = p5; break; // top
                            case 3: a = p4; b = p3; c = p7; d = p8; break; // bottom
                            case 4: a = p1; b = p4; c = p8; d = p5; break; // left
                            case 5: a = p2; b = p3; c = p7; d = p6; break; // right
                        }

                    subdivide(a, b, c, d, vertices, indices);
                }
        }
    }