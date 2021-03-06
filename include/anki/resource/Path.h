// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Math.h>

namespace anki
{

/// Path @ref Resource resource
class Path
{
public:
	Vector<Vec3> positions; ///< AKA translations
	Vector<Quat> rotations;
	Vector<F32> scales;
	F32 step;

	Path()
	{
	}
	~Path()
	{
	}
	bool load(const char* filename);
};

} // end namespace anki
