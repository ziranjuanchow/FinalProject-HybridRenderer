/******************************************************************************/
/*!
\file	VulkanDeferredRenderer.h
\author David Grosman
\par    email: ToDavidGrosman\@gmail.com
\par    Project: CIS 565: GPU Programming and Architecture - Final Project.
\date   11/18/2016
\brief

Compiled using Microsoft (R) C/C++ Optimizing Compiler Version 18.00.21005.1 for
x86 which is my default VS2013 compiler.

This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)

*/
/******************************************************************************/

#ifndef _VULKAN_DEFERRED_RENDERER_H_
#define _VULKAN_DEFERRED_RENDERER_H_

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

#include "Utilities.h"

#include "VulkanUtilities.h"
#include "VulkanRenderer.h"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false

// Texture properties
#define TEX_DIM 2048
#define TEX_FILTER VK_FILTER_LINEAR

// Offscreen frame buffer properties
#define FB_DIM TEX_DIM

class VulkanDeferredRenderer : public VulkanRenderer
{
public:

	VulkanDeferredRenderer(const std::string& fileName);
	~VulkanDeferredRenderer();

	virtual void draw(SRendererContext& context);
	virtual void shutdownVulkan();

	////////////////////////////////////////////////////////////////////////////////////////////////
	////////					VulkanRenderer::VulkanInit() Interface						////////

	// Prepare a new framebuffer for offscreen rendering
	// The contents of this framebuffer are then
	// blitted to our render target
	virtual void setupFrameBuffer();

	virtual void setupUniformBuffers(SRendererContext& context);

	// set up the resources (eg: desriptorPool, descriptorSetLayout) that are going to be used by the descriptors.
	virtual void setupDescriptorFramework();

	// set up the scene shaders' descriptors.
	virtual void setupDescriptors();

	virtual void setupPipelines();

	virtual void buildCommandBuffers();

	void updateUniformBuffersScreen();

	void updateUniformBufferDeferredMatrices(SRendererContext& context);

	// Update fragment shader light position uniform block
	void updateUniformBufferDeferredLights(SRendererContext& context);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	////////					Event-Handler Functions  								     ////////

	virtual void toggleDebugDisplay();

	// Called when view change occurs
	// Can be overriden in derived class to e.g. update uniform buffers 
	// Containing view dependant matrices
	virtual void viewChanged(SRendererContext& context);

private:

	struct SInputTextures
	{
		vkUtils::VulkanTexture m_colorMap;
		vkUtils::VulkanTexture m_normalMap;
	};

	struct SSceneMeshes
	{
		vkMeshLoader::MeshBuffer m_model;
		vkMeshLoader::MeshBuffer m_floor;
		vkMeshLoader::MeshBuffer m_transparentObj;
		vkMeshLoader::MeshBuffer m_quad;
	};

	struct SVkVertices
	{
		VkPipelineVertexInputStateCreateInfo			m_inputState;
		std::vector<VkVertexInputBindingDescription>	m_bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription>	m_attributeDescriptions;
	};

	struct SVkPipelinesLayout
	{
		VkPipelineLayout m_deferred;
		VkPipelineLayout m_offscreen;
	};

	struct SVkPipelines
	{
		VkPipeline m_deferred;
		VkPipeline m_offscreen;
		VkPipeline m_debug;
	};

	struct SVertexShaderUniforms
	{
		glm::mat4 m_projection;
		glm::mat4 m_model;
		glm::mat4 m_view;
		glm::vec4 m_instancePos[3];
	};

	struct SFragShaderUniforms
	{
		SSceneLight m_lights[6];
		glm::vec4	m_viewPos;
	};

	struct SVkUniformData
	{
		vkUtils::UniformData m_vsFullScreen;
		vkUtils::UniformData m_vsOffscreen;
		vkUtils::UniformData m_fsLights;
	};

	struct SVkDescriptorSets
	{
		VkDescriptorSet m_model;
		VkDescriptorSet m_floor;
		VkDescriptorSet m_quad;
	};

	// Framebuffer for offscreen rendering
	struct SFrameBufferAttachment
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFormat format;
	};

	struct SFrameBuffer 
	{
		int32_t width, height;
		VkFramebuffer frameBuffer;
		SFrameBufferAttachment position, normal, albedo;
		SFrameBufferAttachment depth;
		VkRenderPass renderPass;
	};

private:

	// Create a frame buffer attachment
	void createAttachment(VkFormat format, VkImageUsageFlagBits usage, SFrameBufferAttachment *attachment, VkCommandBuffer layoutCmd);

	// Build command buffer for rendering the scene to the offscreen frame buffer attachments
	void buildDeferredCommandBuffer();
	void reBuildCommandBuffers();

	void loadTextures();
	void loadMeshes();
	void generateQuads();	

private:

	SInputTextures			m_floorTex;
	SInputTextures			m_modelTex;

	SSceneMeshes			m_sceneMeshes;

	SVkVertices				m_vertices;

	SVkPipelinesLayout		m_pipelineLayouts;

	SVkPipelines			m_pipelines;
	
	SVertexShaderUniforms	m_uboVS;
	SVertexShaderUniforms	m_uboOffscreenVS;

	SFragShaderUniforms		m_uboFragmentLights;

	SVkUniformData			m_uniformData;

	VkDescriptorSetLayout	m_descriptorSetLayout;
	SVkDescriptorSets		m_descriptorSets;

	SFrameBuffer			m_offScreenFrameBuf;

	// One sampler for the frame buffer color attachments
	VkSampler				m_colorSampler;

	VkCommandBuffer			m_offScreenCmdBuffer;

	// Semaphore used to synchronize between offscreen and final scene rendering
	VkSemaphore				m_offscreenSemaphore;
};

#endif // _VULKAN_DEFERRED_RENDERER_H_