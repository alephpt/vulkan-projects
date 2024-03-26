VULKAN_SDK_PATH = /usr/include/vulkan
STB_PATH = /usr/include/stb
TINYOBJ_PATH = /usr/include/
VK_EXP_PATH = /etc/vulkan/explicit_layer.d

CFLAGS = -std=c++17 -I $(VULKAN_SDK_PATH) -I $(STB_PATH) -I $(TINYOBJ_PATH)
LDFLAGS = -L $(VULKAN_SDK_PATH) -lSDL2 -lvulkan -ldl -pthread 
DBFLAGS = -fsanitize=address,undefined --debug
INCLUDES = ./modules/*.cpp ./components/*.cpp

compute: main.cpp
	g++ $(CFLAGS) -o ~/compute main.cpp $(LDFLAGS) $(INCLUDES)

.PHONY: test clean

debug:
	g++ $(CFLAGS) -o ~/compute main.cpp $(LDFLAGS) $(INCLUDES) $(DBFLAGS)

test: compute
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH) VK_LAYER_PATH=$(VK_EXP_PATH) ./compute

clean:
	rm -f ~/compute
