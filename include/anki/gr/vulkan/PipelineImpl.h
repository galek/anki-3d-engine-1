// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>

namespace anki
{

// Forward
class VertexStateInfo;
class InputAssemblerStateInfo;
class TessellationStateInfo;
class ViewportStateInfo;
class RasterizerStateInfo;
class DepthStencilStateInfo;
class ColorStateInfo;

/// @addtogroup vulkan
/// @{

/// Program pipeline.
class PipelineImpl : public VulkanObject
{
public:
	VkPipeline m_handle = VK_NULL_HANDLE;

	PipelineImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~PipelineImpl();

	void init(const PipelineInitInfo& init);

private:
	void initGraphics(const PipelineInitInfo& init);

	void initCompute(const PipelineInitInfo& init);

	void initShaders(
		const PipelineInitInfo& init, VkGraphicsPipelineCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineVertexInputStateCreateInfo* initVertexStage(
		const VertexStateInfo& vertex,
		VkPipelineVertexInputStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineInputAssemblyStateCreateInfo*
	initInputAssemblyState(const InputAssemblerStateInfo& ia,
		VkPipelineInputAssemblyStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineTessellationStateCreateInfo*
	initTessellationState(const TessellationStateInfo& t,
		VkPipelineTessellationStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineViewportStateCreateInfo* initViewportState(
		VkPipelineViewportStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineRasterizationStateCreateInfo* initRasterizerState(
		const RasterizerStateInfo& r,
		VkPipelineRasterizationStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineMultisampleStateCreateInfo* initMsState(
		VkPipelineMultisampleStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineDepthStencilStateCreateInfo* initDsState(
		const DepthStencilStateInfo& ds,
		VkPipelineDepthStencilStateCreateInfo& ci);

	ANKI_USE_RESULT VkPipelineColorBlendStateCreateInfo* initColorState(
		const ColorStateInfo& c, VkPipelineColorBlendStateCreateInfo& ci);
};
/// @}

} // end namespace anki
