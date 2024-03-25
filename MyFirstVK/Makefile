VULKAN_SDK_PATH = /usr/include/vulkan
STB_PATH = /usr/include/stb
TINYOBJ_PATH = /usr/include/
VK_EXP_PATH = /etc/vulkan/explicit_layer.d

CFLAGS = -std=c++17 -I $(VULKAN_SDK_PATH) -I $(STB_PATH) -I $(TINYOBJ_PATH)
LDFLAGS = -L $(VULKAN_SDK_PATH) -lglfw3 -lvulkan -ldl -pthread 
DBFLAGS = -fsanitize=address,undefined --debug

Triangles: main.cpp
	g++ $(CFLAGS) -o Triangles main.cpp $(LDFLAGS)

.PHONY: test clean

debug:
	g++ $(CFLAGS) -o Triangles main.cpp $(LDFLAGS) $(DBFLAGS)

test: Triangles
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH) VK_LAYER_PATH=$(VK_EXP_PATH) ./Triangles

clean:
	rm -f Triangles

