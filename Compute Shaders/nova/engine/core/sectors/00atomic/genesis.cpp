#include "genesis.h"
#include "lexicon.h"
#include <fstream>

#include <random>
#include <ctime>
#include <vulkan/vulkan.h>

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

void genesis::createShaderModule(VkDevice* logical_device, std::vector<char>& code, VkShaderModule* shader_module)
    {
        report(LOGGER::VLINE, "\t\t .. Creating Shader Module ..");

        VkShaderModuleCreateInfo _create_info = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data())
            };

        VK_TRY(vkCreateShaderModule(*logical_device, &_create_info, nullptr, shader_module));

        return;
    }


static inline glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t) { return a + t * (b - a); }
static const glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
static const glm::vec3 green = glm::vec3(0.0f, 1.0f, 0.0f);
static const glm::vec3 blue = glm::vec3(0.0f, 0.0f, 1.0f);
static const glm::vec3 yellow = glm::vec3(1.0f, 1.0f, 0.0f);

const int screen_height = 1200;

void genesis::createParticles(std::vector<Particle>* particles)   
    {
        std::default_random_engine random_engine((unsigned)time(nullptr));
        std::uniform_real_distribution<float> random_dist(0.0f, 1.0f);

        // this creates a circle of particles that are randomly colored
        // for (auto& particle : *particles) 
        //     {
        //         float r = 0.25f * sqrt(random_dist(random_engine));
        //         float theta = 2.0f * 3.14159265359f * random_dist(random_engine);
        //         float phi = acos(2.0f * random_dist(random_engine) - 1.0f);
        //         float x = r * sin(phi) * cos(theta);
        //         float y = r * sin(phi) * sin(theta);

        //         particle.position = glm::vec2(x, y);
        //         particle.velocity = glm::normalize(glm::vec2(-y, x) * 0.00025f);
        //         glm::vec3 t_r = lerp(red, yellow, random_dist(random_engine));
        //         glm::vec3 t_g = lerp(green, blue, random_dist(random_engine));
        //         glm::vec3 t_b = lerp(blue, red, random_dist(random_engine));
        //         particle.color = glm::vec4(lerp(t_r, lerp(t_g, t_b, random_dist(random_engine)), random_dist(random_engine)), random_dist(random_engine));
        //     }

        // this creates a grouping of particles that are all near the top like rain
        for (auto& particle : *particles) 
            {
                float x = random_dist(random_engine) * 2.0f - 1.0f;
                float y = screen_height + random_dist(random_engine) * 100.0f;
                particle.position = glm::vec2(x, y);
                particle.velocity = glm::vec2(0.0f, -0.00025f);
                particle.color = glm::vec4(lerp(blue, lerp(green, blue, random_dist(random_engine)), random_dist(random_engine)), random_dist(random_engine));
            }
    }



static const glm::vec3 p1 = glm::vec3(-0.5f, 0.5f, 0.5f);   // top left front
static const glm::vec3 p2 = glm::vec3(0.5f, 0.5f, 0.5f);    // top right front
static const glm::vec3 p3 = glm::vec3(0.5f, -0.5f, 0.5f);   // bottom right front
static const glm::vec3 p4 = glm::vec3(-0.5f, -0.5f, 0.5f);  // bottom left front
static const glm::vec3 p5 = glm::vec3(-0.5f, 0.5f, -0.5f);  // top left back
static const glm::vec3 p6 = glm::vec3(0.5f, 0.5f, -0.5f);   // top right back
static const glm::vec3 p7 = glm::vec3(0.5f, -0.5f, -0.5f);  // bottom right back
static const glm::vec3 p8 = glm::vec3(-0.5f, -0.5f, -0.5f); // bottom left back

static const glm::vec2 uv1 = glm::vec2(0.0f, 0.0f); // top left
static const glm::vec2 uv2 = glm::vec2(1.0f, 0.0f); // top right
static const glm::vec2 uv3 = glm::vec2(1.0f, 1.0f); // bottom right
static const glm::vec2 uv4 = glm::vec2(0.0f, 1.0f); // bottom left


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
        vertices->push_back({p4, yellow, uv4});  // 0
        vertices->push_back({p3, green, uv3});  // 1
        vertices->push_back({p2, red, uv2});    // 2
        vertices->push_back({p1, blue, uv1});     // 3
        vertices->push_back({p8, yellow, uv3});  // 4
        vertices->push_back({p7, green, uv4});   // 5
        vertices->push_back({p6, red, uv1});    // 6
        vertices->push_back({p5, blue, uv2});     // 7

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
