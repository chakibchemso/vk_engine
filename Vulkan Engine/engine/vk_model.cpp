#define TINYOBJLOADER_IMPLEMENTATION

#include "vk_model.hpp"
#include "../engine/vk_utils.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <glm/gtx/hash.hpp>
#include <TinyObjLoader/tiny_obj_loader.hpp>

using vk_engine::vk_model;

namespace std
{
	template <>
	struct hash<vk_model::vertex>
	{
		size_t operator()(const vk_model::vertex& vertex) const noexcept
		{
			size_t seed{0};
			vk_engine::hash_combine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

std::vector<VkVertexInputBindingDescription> vk_model::vertex::get_binding_descriptions()
{
	std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
	binding_descriptions[0].binding = 0;
	binding_descriptions[0].stride = sizeof vertex;
	binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> vk_model::vertex::get_attribute_descriptions()
{
	std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

	attribute_descriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position)});
	attribute_descriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, color)});
	attribute_descriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, normal)});
	attribute_descriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, uv)});
	
	return attribute_descriptions;
}

bool vk_model::vertex::operator==(const vertex& other) const
{
	return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
}

void vk_model::builder::load_model(const std::string& file_path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	if (!LoadObj(&attrib, &shapes, &materials, &warning, &error, file_path.c_str()))
		throw std::runtime_error(error + warning);

	vertices.clear();
	indices.clear();

	std::unordered_map<vertex, uint32_t> unique_vertices{};

	for (const auto& [name, mesh, lines, points] : shapes)
		for (const auto& [vertex_index, normal_index, texcoord_index] : mesh.indices)
		{
			vertex vertex{};

			if (vertex_index >= 0)
			{
				vertex.position = {
					attrib.vertices[3 * vertex_index + 0],
					attrib.vertices[3 * vertex_index + 1],
					attrib.vertices[3 * vertex_index + 2],
				};

				vertex.color = {
					attrib.colors[3 * vertex_index + 0],
					attrib.colors[3 * vertex_index + 1],
					attrib.colors[3 * vertex_index + 2],
				};
			}

			if (normal_index >= 0)
			{
				vertex.normal = {
					attrib.normals[3 * normal_index + 0],
					attrib.normals[3 * normal_index + 1],
					attrib.normals[3 * normal_index + 2],
				};
			}

			if (texcoord_index >= 0)
			{
				vertex.uv = {
					attrib.texcoords[2 * texcoord_index + 0],
					attrib.texcoords[2 * texcoord_index + 1],
				};
			}

			if (unique_vertices.count(vertex) == 0)
			{
				unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(unique_vertices[vertex]);
		}
}

vk_model::vk_model(vk_device& device, const builder& builder): device(device)
{
	create_vertex_buffers(builder.vertices);
	create_index_buffers(builder.indices);
}

vk_model::~vk_model()
{
	vkDestroyBuffer(device.device(), vertex_buffer, nullptr);
	vkFreeMemory(device.device(), vertex_buffer_memory, nullptr);
	if (has_index_buffer)
	{
		vkDestroyBuffer(device.device(), index_buffer, nullptr);
		vkFreeMemory(device.device(), index_buffer_memory, nullptr);
	}
}

std::unique_ptr<vk_model> vk_model::create_model_from_file(vk_device& device, const std::string& file_path)
{
	builder builder{};
	builder.load_model(file_path);

	std::cout
		<< "[Model Loader]" << std::endl
		<< "	loaded model: " << std::endl
		<< "	file path: " << file_path << std::endl
		<< "	vertex count: " << builder.vertices.size() << std::endl
		<< "	index count: " << builder.indices.size() << std::endl;

	return std::make_unique<vk_model>(device, builder);
}

void vk_model::bind(const VkCommandBuffer command_buffer) const
{
	const VkBuffer buffers[] = {vertex_buffer};
	constexpr VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
	if (has_index_buffer)
		vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void vk_model::draw(const VkCommandBuffer command_buffer) const
{
	if (has_index_buffer)
		vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
	else
		vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
}

void vk_model::create_vertex_buffers(const std::vector<vertex>& vertices)
{
	vertex_count = static_cast<uint32_t>(vertices.size());
	assert(vertex_count >= 3 && "Vertex count must be at least 3");

	const VkDeviceSize buffer_size = sizeof vertices[0] * vertex_count;
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	device.create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staging_buffer,
		staging_buffer_memory);

	void* data;
	vkMapMemory(device.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, vertices.data(), buffer_size);
	vkUnmapMemory(device.device(), staging_buffer_memory);

	device.create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertex_buffer,
		vertex_buffer_memory);

	device.copy_buffer(staging_buffer, vertex_buffer, buffer_size);

	vkDestroyBuffer(device.device(), staging_buffer, nullptr);
	vkFreeMemory(device.device(), staging_buffer_memory, nullptr);
}

void vk_model::create_index_buffers(const std::vector<uint32_t>& indices)
{
	index_count = static_cast<uint32_t>(indices.size());
	has_index_buffer = index_count > 0;

	if (!has_index_buffer)
		return;

	const VkDeviceSize buffer_size = sizeof indices[0] * index_count;
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;

	device.create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staging_buffer,
		staging_buffer_memory);

	void* data;
	vkMapMemory(device.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, indices.data(), buffer_size);
	vkUnmapMemory(device.device(), staging_buffer_memory);

	device.create_buffer(
		buffer_size,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		index_buffer,
		index_buffer_memory);

	device.copy_buffer(staging_buffer, index_buffer, buffer_size);

	vkDestroyBuffer(device.device(), staging_buffer, nullptr);
	vkFreeMemory(device.device(), staging_buffer_memory, nullptr);
}
