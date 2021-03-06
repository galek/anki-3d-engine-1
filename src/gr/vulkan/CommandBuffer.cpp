// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>

namespace anki
{

//==============================================================================
CommandBuffer::CommandBuffer(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

//==============================================================================
CommandBuffer::~CommandBuffer()
{
}

//==============================================================================
void CommandBuffer::init(CommandBufferInitInfo& inf)
{
}

//==============================================================================
CommandBufferInitHints CommandBuffer::computeInitHints() const
{
}

//==============================================================================
void CommandBuffer::flush()
{
}

//==============================================================================
void CommandBuffer::finish()
{
}

//==============================================================================
void CommandBuffer::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
}

//==============================================================================
void CommandBuffer::setPolygonOffset(F32 offset, F32 units)
{
}

//==============================================================================
void CommandBuffer::bindPipeline(PipelinePtr ppline)
{
}

//==============================================================================
void CommandBuffer::beginRenderPass(FramebufferPtr fb)
{
}

//==============================================================================
void CommandBuffer::endRenderPass()
{
}

//==============================================================================
void CommandBuffer::bindResourceGroup(
	ResourceGroupPtr rc, U slot, const TransientMemoryInfo* dynInfo)
{
}

//==============================================================================
void CommandBuffer::drawElements(U32 count,
	U32 instanceCount,
	U32 firstIndex,
	U32 baseVertex,
	U32 baseInstance)
{
}

//==============================================================================
void CommandBuffer::drawArrays(
	U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
}

//==============================================================================
void CommandBuffer::drawElementsConditional(OcclusionQueryPtr query,
	U32 count,
	U32 instanceCount,
	U32 firstIndex,
	U32 baseVertex,
	U32 baseInstance)
{
}

//==============================================================================
void CommandBuffer::drawArraysConditional(OcclusionQueryPtr query,
	U32 count,
	U32 instanceCount,
	U32 first,
	U32 baseInstance)
{
}

//==============================================================================
void CommandBuffer::dispatchCompute(
	U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
}

//==============================================================================
void CommandBuffer::generateMipmaps(TexturePtr tex, U depth, U face)
{
}

//==============================================================================
void CommandBuffer::copyTextureToTexture(TexturePtr src,
	const TextureSurfaceInfo& srcSurf,
	TexturePtr dest,
	const TextureSurfaceInfo& destSurf)
{
}

//==============================================================================
void CommandBuffer::clearTexture(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const ClearValue& clearValue)
{
}

//==============================================================================
void CommandBuffer::uploadTextureSurface(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const TransientMemoryToken& token)
{
}

//==============================================================================
void CommandBuffer::uploadBuffer(
	BufferPtr buff, PtrSize offset, const TransientMemoryToken& token)
{
}

//==============================================================================
void CommandBuffer::setPipelineBarrier(
	PipelineStageBit src, PipelineStageBit dst)
{
}

//==============================================================================
void CommandBuffer::setBufferMemoryBarrier(
	BufferPtr buff, ResourceAccessBit src, ResourceAccessBit dst)
{
}

//==============================================================================
void CommandBuffer::beginOcclusionQuery(OcclusionQueryPtr query)
{
}

//==============================================================================
void CommandBuffer::endOcclusionQuery(OcclusionQueryPtr query)
{
}

//==============================================================================
void CommandBuffer::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
}

//==============================================================================
Bool CommandBuffer::isEmpty() const
{
}

} // end namespace anki