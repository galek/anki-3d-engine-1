// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/RenderingPass.h>
#include <anki/Gr.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ShaderResource.h>
#include <anki/core/Timestamp.h>

namespace anki
{

class ShaderProgram;

/// @addtogroup renderer
/// @{

/// Bloom pass.
class Bloom : public RenderingPass
{
anki_internal:
	static const PixelFormat RT_PIXEL_FORMAT;

	Bloom(Renderer* r)
		: RenderingPass(r)
	{
	}

	~Bloom();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	void run(RenderingContext& ctx);

	TexturePtr& getRt()
	{
		return m_vblurRt;
	}

	TexturePtr& getRt1()
	{
		return m_hblurRt;
	}

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

private:
	U32 m_width, m_height;
	F32 m_threshold = 10.0; ///< How bright it is
	F32 m_scale = 1.0;
	F32 m_blurringDist = 1.0; ///< Distance in blurring

	FramebufferPtr m_hblurFb;
	FramebufferPtr m_vblurFb;

	ShaderResourcePtr m_quadVert;
	ShaderResourcePtr m_toneFrag;
	ShaderResourcePtr m_hblurFrag;
	ShaderResourcePtr m_vblurFrag;

	PipelinePtr m_tonePpline;
	PipelinePtr m_hblurPpline;
	PipelinePtr m_vblurPpline;

	TexturePtr m_hblurRt; ///< pass0Fai with the horizontal blur FAI
	TexturePtr m_vblurRt; ///< The final FAI

	ResourceGroupPtr m_firstDescrGroup;
	ResourceGroupPtr m_hDescrGroup;
	ResourceGroupPtr m_vDescrGroup;

	ANKI_USE_RESULT Error initFb(FramebufferPtr& fb, TexturePtr& rt);
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};

/// @}

} // end namespace anki
