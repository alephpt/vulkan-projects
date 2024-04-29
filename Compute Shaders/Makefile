VULKAN_SDK_PATH = /usr/include/vulkan
STB_PATH = /usr/include/stb
TINYOBJ_PATH = /usr/include/
VK_EXP_PATH = /etc/vulkan/explicit_layer.d

CFLAGS = -std=c++17 -I $(VULKAN_SDK_PATH) -I $(STB_PATH) -I $(TINYOBJ_PATH)
LDFLAGS = -L $(VULKAN_SDK_PATH) -lSDL2 -lvulkan -ldl -pthread 
DBFLAGS = -fsanitize=address,undefined --debug
INCLUDES = ./nova/*.cpp ./nova/engine/*.cpp ./nova/engine/core/*.cpp ./nova/engine/core/components/*/*.cpp ./nova/engine/core/sectors/*/*.cpp ./nova/engine/core/sectors/*/*/*.cpp 
OUT = ~/compute

compute:
	g++ $(CFLAGS) -o $(OUT) main.cpp $(LDFLAGS) $(INCLUDES)

run: main.cpp
	g++ $(CFLAGS) -o $(OUT) main.cpp $(LDFLAGS) $(INCLUDES)
	$(OUT)

.PHONY: test clean

debug:
	g++ $(CFLAGS) -o $(OUT) main.cpp $(LDFLAGS) $(INCLUDES) $(DBFLAGS)

test: compute
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH) VK_LAYER_PATH=$(VK_EXP_PATH) $(OUT)

clean:
	rm -f ~/compute
