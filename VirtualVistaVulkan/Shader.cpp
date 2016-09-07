
#include <ostream>
#include <fstream>
#include <iostream>

#include "Shader.h"
#include "Utils.h"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Shader::Shader()
	{
	}


	Shader::~Shader()
	{
		if (vert_module != VK_NULL_HANDLE)
			vkDestroyShaderModule(device_, vert_module, nullptr);

		if (frag_module != VK_NULL_HANDLE)
			vkDestroyShaderModule(device_, frag_module, nullptr);
	}


	void Shader::create(std::string path, std::string name, VkDevice device)
	{
		device_ = device;
		vert_path_ = path + name + "_vert" + ".spv";
		frag_path_ = path + name + "_frag" + ".spv";
		vert_binary_data_ = loadSpirVBinary(vert_path_);
		frag_binary_data_ = loadSpirVBinary(frag_path_);

		createShaderModule(vert_binary_data_, vert_module);
		createShaderModule(frag_binary_data_, frag_module);
	}

	
	///////////////////////////////////////////////////////////////////////////////////////////// Private
	std::vector<char> Shader::loadSpirVBinary(std::string file_name)
	{
		std::ifstream file(file_name, std::ios::ate | std::ios::binary);
		VV_ASSERT(file.is_open(), "Vulkan Error: failed to open Spir-V file: " + file_name);

		size_t file_size = (size_t)file.tellg();
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);

		VV_ASSERT(!buffer.empty(), "Vulkan Error: Spir-V file empty: " + file_name);
		file.close();
		return buffer;
	}


	void Shader::createShaderModule(const std::vector<char> byte_code, VkShaderModule &module)
	{
		VkShaderModuleCreateInfo shader_module_create_info = {};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.flags = 0;
		shader_module_create_info.codeSize = byte_code.size();
		shader_module_create_info.pCode = (uint32_t *)byte_code.data();

		VV_CHECK_SUCCESS(vkCreateShaderModule(device_, &shader_module_create_info, nullptr, &module));
	}
}