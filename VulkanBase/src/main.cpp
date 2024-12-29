#include "renderer.h"

#define VMA_IMPLEMENTATION
#include <VMA/vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

int main() {
	vu::Renderer app;

	try {
		app.Run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}