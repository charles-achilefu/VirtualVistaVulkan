
#include "Scene.h"

#include <chrono>

#include "Settings.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace vv
{
	///////////////////////////////////////////////////////////////////////////////////////////// Public
	Scene::Scene()
	{
	}


	Scene::~Scene()
	{
	}


	void Scene::create(VulkanDevice *device, VulkanRenderPass *render_pass)
	{
        _device = device;
        _render_pass = render_pass;

        createDescriptorPool(); // Global descriptor pool from which all descriptor sets are allocated from.
        createSceneUniforms();
        createSampler();

        MaterialTemplate *dummy_template = new MaterialTemplate();
        createMaterialTemplates(); // Load material templates to prepare for model loading queries

        _model_manager = new ModelManager();
        _model_manager->create(_device, dummy_template, _descriptor_pool, _sampler);
        _initialized = true;


        // layouts
        // shader
        // pipeline
        // mesh
        // texture image view
        // sampler
        // vertex/index buffers
        // ubo 
        // descriptor pool
        // sets
	}


	void Scene::shutDown()
	{

	}


    void Scene::signalAllLightsAdded()
    {
        // todo: initialize lights descriptor info

        createPipelines();
    }


    void Scene::addLight()
    {

    }


    Model* Scene::addModel(std::string path, std::string name, std::string material_template)
    {
        VV_ASSERT(_initialized, "ERROR: you need to properly initialize scene before adding models");
        Model *model = new Model();
        _model_manager->loadModel(path, name, material_templates[material_template], model);

        Mesh * mesh_data = _model_manager->_loaded_meshes[model->_data_handle][0];

        _temp_model_vertex_buffer = new VulkanBuffer();
        _temp_model_vertex_buffer->create(_device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(mesh_data->_vertices[0]) * mesh_data->_vertices.size());
        _temp_model_vertex_buffer->updateAndTransfer(mesh_data->_vertices.data());

        _temp_model_index_buffer = new VulkanBuffer();
        _temp_model_index_buffer->create(_device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(mesh_data->_indices[0]) * mesh_data->_indices.size());
        _temp_model_index_buffer->updateAndTransfer(mesh_data->_indices.data());

        _models.push_back(model);
        return model;
    }


    void Scene::addCamera()
    {

    }


    void Scene::updateSceneUniforms(VkExtent2D extent)
    {
        // update scene uniforms
        static auto start_time = std::chrono::high_resolution_clock::now();
		auto curr_time = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - start_time).count() / 1000.0f;

		_ubo.model = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		_ubo.model = glm::rotate(_ubo.model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		_ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		_ubo.proj = glm::perspective(glm::radians(45.0f), extent.width / static_cast<float>(extent.height), 0.1f, 10.0f);
		_ubo.normal = glm::vec3(glm::rotate(glm::mat4(), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        _scene_uniform_buffer->updateAndTransfer(&_ubo);
    }


    void Scene::render(VkCommandBuffer command_buffer)
    {

        bool first_run = true;
        MaterialTemplate *curr_template = _models[0]->material_template;
        curr_template->pipeline->bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
        Mesh *mesh = _model_manager->_loaded_meshes["../assets/models/chalet/chalet.obj"][0];
        std::array<VkDeviceSize, 1> offsets = { 0 };
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &_temp_model_vertex_buffer->buffer, offsets.data());
        vkCmdBindIndexBuffer(command_buffer, _temp_model_index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(command_buffer, (uint32_t)mesh->_indices.size(), 1, 0, 0, 0);

        /*for (auto &model : _models)
        {
            // reduce pipeline state switches as much as possible
            if (first_run || (curr_template->name != model->material_template->name))
            {
                first_run = false;
                curr_template = model->material_template;
                curr_template->pipeline->bind(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS);

                // bind updated descriptor set
                // todo: this really only needs to be set a single time. I never change the set, I simply update the vulkan uniform buffer.
                //vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, curr_template->pipeline_layout, 0, 1, &_scene_descriptor_set, 0, nullptr);
            }

            std::vector<Mesh *> meshes = _model_manager->_loaded_meshes[model->_data_handle];
            std::vector<Material *> materials = _model_manager->_loaded_materials[model->_data_handle][model->_material_id_set];

            for (auto &mesh : meshes)
            {
                //Material *material = materials[mesh->material_id];
                std::vector<VkDescriptorSet> descriptor_sets = { _scene_descriptor_set , _lights_descriptor_set };
                //material->bindDescriptorSets(command_buffer, descriptor_sets);
                //mesh->bindBuffers(command_buffer);
                std::array<VkDeviceSize, 1> offsets = { 0 };
                vkCmdBindVertexBuffers(command_buffer, 0, 1, &_temp_model_vertex_buffer->buffer, offsets.data());
                vkCmdBindIndexBuffer(command_buffer, _temp_model_index_buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
                mesh->render(command_buffer);
            }
        }*/
    }


	///////////////////////////////////////////////////////////////////////////////////////////// Private
    void Scene::createMaterialTemplates()
    {
        // note: apply name to template based on name assigned to spriv shader

        // for:
        //     load shader file and parse using spirv-cross
        //     generate unique pipeline/shader classes based on file
        //     store template in vector? whatever would be best for getting it by name at initialization time.

        MaterialTemplate *material_template = new MaterialTemplate();
        material_template->name = "triangle";

        // note: for now manually loading single template
        std::vector<DescriptorType> descriptor_orderings;
        descriptor_orderings.push_back(DescriptorType::CONSTANTS);
        material_template->descriptor_orderings = descriptor_orderings;

        // Descriptor Set Layouts
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
        std::vector<VkDescriptorSetLayoutBinding> temp_bindings_buffer;

        /// general scene layout
        //descriptor_set_layouts.push_back(_scene_descriptor_set_layout);

        /// Material template descriptor layouts
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT));
        //material_template->descriptor_set_layout = util::createVulkanDescriptorSetLayout(_device->logical_device, temp_bindings_buffer);
        
        createVulkanDescriptorSetLayout(_device->logical_device, temp_bindings_buffer, material_template->descriptor_set_layout);
        descriptor_set_layouts.push_back(material_template->descriptor_set_layout);

        /// non-standard descriptor set types (i.e. not associated with lights, scene, or standard uniform types

        // Shader
        Shader *shader = new Shader();
        shader->create(_device, material_template->name);
        material_template->shader = shader;

        // Pipeline
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.flags = 0;
		pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
		pipeline_layout_create_info.pPushConstantRanges = nullptr; // todo: add push constants
		pipeline_layout_create_info.pushConstantRangeCount = 0;

		VV_CHECK_SUCCESS(vkCreatePipelineLayout(_device->logical_device, &pipeline_layout_create_info, nullptr, &material_template->pipeline_layout));

        VulkanPipeline *pipeline = new VulkanPipeline();

        // todo: all pipelines should be created via single call to vkCreateGraphicsPipelines
		pipeline->create(_device, material_template->shader, material_template->pipeline_layout, _render_pass, true, true); // todo: add option for settings passed.
        material_template->pipeline = pipeline;

        material_templates[material_template->name] = material_template;
    }


    void Scene::createPipelines()
    {
        // todo: figure out how to report light descriptor info to pipeline creation properly
    }


    void Scene::createDescriptorPool()
	{
        // global pool
        //std::vector<VkDescriptorPoolSize> pool_sizes;
        /*VkDescriptorPoolSize uniform_buffer_size = {};
        uniform_buffer_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniform_buffer_size.descriptorCount = 100;
        
        VkDescriptorPoolSize sampler_size = {};
        sampler_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_size.descriptorCount = 100;
        //pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_UNIFORM_BUFFERS });
        //pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_COMBINED_IMAGE_SAMPLERS });
        pool_sizes.push_back(uniform_buffer_size);
        pool_sizes.push_back(sampler_size);*/

        std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = 100;
		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[1].descriptorCount = 100;

		VkDescriptorPoolCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		create_info.pPoolSizes = pool_sizes.data();
		create_info.maxSets = MAX_DESCRIPTOR_SETS;
		create_info.flags = 0; // can be: VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT

        // set up global descriptor pool
		VV_CHECK_SUCCESS(vkCreateDescriptorPool(_device->logical_device, &create_info, nullptr, &_descriptor_pool));
	}

    
    void Scene::createSceneUniforms()
	{
        // Vulkan Buffer
        _ubo = { glm::mat4(), glm::mat4(), glm::mat4(), glm::vec3(0.0, 0.0, 1.0) };
        _scene_uniform_buffer = new VulkanBuffer();
        _scene_uniform_buffer->create(_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(_ubo));

        // Layout
        std::vector<VkDescriptorSetLayoutBinding> temp_bindings_buffer;
        temp_bindings_buffer.push_back(createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT));
        //_scene_descriptor_set_layout = util::createVulkanDescriptorSetLayout(_device->logical_device, temp_bindings_buffer);
        
        createVulkanDescriptorSetLayout(_device->logical_device, temp_bindings_buffer, _scene_descriptor_set_layout);

        // Descriptor Set
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = _descriptor_pool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &_scene_descriptor_set_layout;

		VV_CHECK_SUCCESS(vkAllocateDescriptorSets(_device->logical_device, &alloc_info, &_scene_descriptor_set));

		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = _scene_uniform_buffer->buffer;
		buffer_info.offset = 0;
		buffer_info.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet write_set = {};
		write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_set.dstSet = _scene_descriptor_set;
		write_set.dstBinding = 0;
		write_set.dstArrayElement = 0;
		write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_set.descriptorCount = 1; // how many elements to update
		write_set.pBufferInfo = &buffer_info;

		vkUpdateDescriptorSets(_device->logical_device, 1, &write_set, 0, nullptr);
	}


	void Scene::createSampler()
	{
		VkSamplerCreateInfo sampler_create_info = {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.magFilter = VK_FILTER_LINEAR; // VK_FILTER_NEAREST
		sampler_create_info.minFilter = VK_FILTER_LINEAR; // VK_FILTER_NEAREST

		// can be a number of things: https://www.khronos.org/registry/vulkan/specs/1.0/xhtml/vkspec.html#VkSamplerAddressMode
		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		sampler_create_info.anisotropyEnable = VK_TRUE;
		sampler_create_info.maxAnisotropy = 16; // max value

		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; // black, white, transparent
		sampler_create_info.unnormalizedCoordinates = VK_FALSE; // [0,1]
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 0.0f; // todo: figure out how lod works with these things

		VV_CHECK_SUCCESS(vkCreateSampler(_device->logical_device, &sampler_create_info, nullptr, &_sampler));
	}


    void Scene::createVulkanDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout &layout)
    {
        VkDescriptorSetLayoutCreateInfo layout_create_info = {};
	    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	    layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
	    layout_create_info.pBindings = bindings.data();

	    VV_CHECK_SUCCESS(vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr, &layout));
    }


    VkDescriptorSetLayoutBinding Scene::createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type, uint32_t count, VkShaderStageFlags shader_stage) const
    {
        VkDescriptorSetLayoutBinding layout_binding = {};
        layout_binding.binding = binding;
		layout_binding.descriptorType = descriptor_type;
		layout_binding.descriptorCount = count; // number of these elements in array sent to device

		layout_binding.stageFlags = shader_stage;
		layout_binding.pImmutableSamplers = nullptr; // todo: figure out if I ever need this
        return layout_binding;
    }
}
