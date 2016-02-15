// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// The base class for all Vulkan object implementations.
class VulkanObject
{
public:
	VulkanObject(GrManager* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	virtual ~VulkanObject()
	{}

private:
	GrManager* m_manager = nullptr;
};
/// @}

} // end namespace anki
