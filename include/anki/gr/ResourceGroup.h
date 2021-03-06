// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/Buffer.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Texture/Sampler Binding.
class TextureBinding
{
public:
	TexturePtr m_texture;
	SamplerPtr m_sampler; ///< Use it to override texture's sampler.
};

/// Buffer binding info.
class BufferBinding
{
public:
	BufferPtr m_buffer;
	PtrSize m_offset = 0;
	PtrSize m_range = 0;
	Bool m_uploadedMemory = false;
};

/// Resource group initializer.
class ResourceGroupInitInfo
{
public:
	Array<TextureBinding, MAX_TEXTURE_BINDINGS> m_textures;
	Array<BufferBinding, MAX_UNIFORM_BUFFER_BINDINGS> m_uniformBuffers;
	Array<BufferBinding, MAX_STORAGE_BUFFER_BINDINGS> m_storageBuffers;
	Array<BufferBinding, MAX_ATOMIC_BUFFER_BINDINGS> m_atomicBuffers;
	Array<BufferBinding, MAX_VERTEX_ATTRIBUTES> m_vertexBuffers;
	BufferBinding m_indexBuffer;
	I8 m_indexSize = -1; ///< Index size in bytes. 2 or 4
};

/// Resource group.
class ResourceGroup : public GrObject
{
public:
	static const GrObjectType CLASS_TYPE = GrObjectType::RESOURCE_GROUP;

	/// Construct.
	ResourceGroup(GrManager* manager, U64 hash = 0);

	/// Destroy.
	~ResourceGroup();

	/// Access the implementation.
	ResourceGroupImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create.
	void init(const ResourceGroupInitInfo& init);

private:
	UniquePtr<ResourceGroupImpl> m_impl;
};
/// @}

} // end namespace anki
