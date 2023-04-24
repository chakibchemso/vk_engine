#include "vk_pipeline.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace vk_engine
{
    vk_pipeline::vk_pipeline(vk_device& device,
                             const std::string& vert_shader_path,
                             const std::string& frag_shader_path,
                             const pipeline_config_info& config_info) : device(device)
    {
        create_graphics_pipeline(vert_shader_path, frag_shader_path, config_info);
    }

    vk_pipeline::~vk_pipeline()
    {
        vkDestroyShaderModule(device.device(), vert_shader_module, nullptr);
        vkDestroyShaderModule(device.device(), frag_shader_module, nullptr);
        vkDestroyPipeline(device.device(), graphics_pipeline, nullptr);
    }

    pipeline_config_info vk_pipeline::default_pipeline_config_info(const uint32_t width, const uint32_t height)
    {
        pipeline_config_info config_info{};

        config_info.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        config_info.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config_info.input_assembly_info.primitiveRestartEnable = VK_FALSE;

        config_info.viewport.x = 0.0f;
        config_info.viewport.y = 0.0f;
        config_info.viewport.width = static_cast<float>(width);
        config_info.viewport.height = static_cast<float>(height);
        config_info.viewport.minDepth = 0.0f;
        config_info.viewport.maxDepth = 1.0f;

        config_info.scissor.offset = {0, 0};
        config_info.scissor.extent = {width, height};

        config_info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        config_info.rasterization_info.depthClampEnable = VK_FALSE;
        config_info.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
        config_info.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
        config_info.rasterization_info.lineWidth = 1.0f;
        config_info.rasterization_info.cullMode = VK_CULL_MODE_NONE;
        config_info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        config_info.rasterization_info.depthBiasEnable = VK_FALSE;
        config_info.rasterization_info.depthBiasConstantFactor = 0.0f; // Optional
        config_info.rasterization_info.depthBiasClamp = 0.0f; // Optional
        config_info.rasterization_info.depthBiasSlopeFactor = 0.0f; // Optional

        config_info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        config_info.multisample_info.sampleShadingEnable = VK_FALSE;
        config_info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        config_info.multisample_info.minSampleShading = 1.0f; // Optional
        config_info.multisample_info.pSampleMask = nullptr; // Optional
        config_info.multisample_info.alphaToCoverageEnable = VK_FALSE; // Optional
        config_info.multisample_info.alphaToOneEnable = VK_FALSE; // Optional

        config_info.color_blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |VK_COLOR_COMPONENT_A_BIT;
        config_info.color_blend_attachment.blendEnable = VK_FALSE;
        config_info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        config_info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        config_info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        config_info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        config_info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        config_info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        config_info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        config_info.color_blend_info.logicOpEnable = VK_FALSE;
        config_info.color_blend_info.logicOp = VK_LOGIC_OP_COPY; // Optional
        config_info.color_blend_info.attachmentCount = 1;
        config_info.color_blend_info.pAttachments = &config_info.color_blend_attachment;
        config_info.color_blend_info.blendConstants[0] = 0.0f; // Optional
        config_info.color_blend_info.blendConstants[1] = 0.0f; // Optional
        config_info.color_blend_info.blendConstants[2] = 0.0f; // Optional
        config_info.color_blend_info.blendConstants[3] = 0.0f; // Optional

        config_info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        config_info.depth_stencil_info.depthTestEnable = VK_TRUE;
        config_info.depth_stencil_info.depthWriteEnable = VK_TRUE;
        config_info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
        config_info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
        config_info.depth_stencil_info.minDepthBounds = 0.0f; // Optional
        config_info.depth_stencil_info.maxDepthBounds = 1.0f; // Optional
        config_info.depth_stencil_info.stencilTestEnable = VK_FALSE;
        config_info.depth_stencil_info.front = {}; // Optional
        config_info.depth_stencil_info.back = {}; // Optional

        return config_info;
    }

    std::vector<char> vk_pipeline::read_file(const std::string& file_path)
    {
        std::ifstream file{file_path, std::ios::ate | std::ios::binary};

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + file_path);
        }

        const size_t file_size = file.tellg();

        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        file.close();
        return buffer;
    }

    void vk_pipeline::create_graphics_pipeline(
        const std::string& vert_shader_path,
        const std::string& frag_shader_path,
        const pipeline_config_info&
        config_info)
    {
        assert(config_info.pipeline_layout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided in config info");
        assert(config_info.render_pass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no render pass provided in config info");
        
        const auto vert_code = read_file(vert_shader_path);
        const auto frag_code = read_file(frag_shader_path);

        std::cout << "Vertex shader code size: " << vert_code.size() << std::endl;
        std::cout << "Fragment shader code size: " << frag_code.size() << std::endl;

        create_shader_module(vert_code, &vert_shader_module);
        create_shader_module(frag_code, &frag_shader_module);

        VkPipelineShaderStageCreateInfo shader_stages[2];
        shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages[0].module = vert_shader_module;
        shader_stages[0].pName = "main";
        shader_stages[0].flags = 0;
        shader_stages[0].pNext = nullptr;
        shader_stages[0].pSpecializationInfo = nullptr;
        
        shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages[1].module = frag_shader_module;
        shader_stages[1].pName = "main";
        shader_stages[1].flags = 0;
        shader_stages[1].pNext = nullptr;
        shader_stages[1].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexAttributeDescriptionCount = 0;
        vertex_input_info.vertexBindingDescriptionCount = 0;
        vertex_input_info.pVertexAttributeDescriptions = nullptr; 
        vertex_input_info.pVertexBindingDescriptions = nullptr;

        VkPipelineViewportStateCreateInfo viewport_info{};
        viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_info.viewportCount = 1;
        viewport_info.pViewports = &config_info.viewport;
        viewport_info.scissorCount = 1;
        viewport_info.pScissors = &config_info.scissor;

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = shader_stages;
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
        pipeline_info.pViewportState = &viewport_info;
        pipeline_info.pRasterizationState = &config_info.rasterization_info;
        pipeline_info.pMultisampleState = &config_info.multisample_info;
        pipeline_info.pColorBlendState = &config_info.color_blend_info;
        pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
        pipeline_info.pDynamicState = nullptr; 

        pipeline_info.layout = config_info.pipeline_layout;
        pipeline_info.renderPass = config_info.render_pass;
        pipeline_info.subpass = config_info.sub_pass;

        pipeline_info.basePipelineIndex = -1; 
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline!");
    }

    void vk_pipeline::create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module) const
    {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device.device(), &create_info, nullptr, shader_module) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module!");
    }
    
}
