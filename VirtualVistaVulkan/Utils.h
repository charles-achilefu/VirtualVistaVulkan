
#include <iostream>
#include <string>
#include <functional>
#include <array>

// todo: this might break on a system with no Vulkan support. find an ifdef guard that will prevent this.
#include <vulkan\vulkan.h>
#include <glm/glm.hpp>

#ifndef VIRTUALVISTA_UTILS_H
#define VIRTUALVISTA_UTILS_H

#define VV_SAFE_DELETE(p) { \
		if (!p) {} \
		else { delete p; p = NULL; } \
	}

#ifdef _DEBUG

#define VV_CHECK_SUCCESS(success) { \
        if (success == VK_SUCCESS) { } \
		else throw std::runtime_error(__FUNCTION__  + std::to_string(__LINE__) + __FILE__); \
    }

#define VV_ASSERT(condition, message) \
		if (condition) { } \
		else \
		{ \
			throw std::runtime_error(#message + std::to_string(__LINE__) + __FILE__ ); \
		}

#else

#define VV_CHECK_SUCCESS(success) {}
#define VV_ASSERT(condition, message) {}

#endif

namespace vv
{
	enum RENDERER_TYPE
	{
		VULKAN,
		OPENGL,
		DIRECT3D
	};

	enum WINDOW_TYPE
	{
		GLFW,
		SDL,
		WINDOWS,
		X11
	};

	struct Vertex
	{
	public:
		glm::vec2 position;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDesciption()
		{
			VkVertexInputBindingDescription binding_description = {};
			binding_description.binding = 0;
			binding_description.stride = sizeof(Vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions;

			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0; // layout placement
			attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // type
			attribute_descriptions[0].offset = offsetof(Vertex, position); // placement in vertex 

			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions[1].offset = offsetof(Vertex, color);

			return attribute_descriptions;
		}
	};
}

#endif // VIRTUALVISTA_UTILS_H