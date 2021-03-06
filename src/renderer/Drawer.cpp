// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Drawer.h>
#include <anki/renderer/Ms.h>
#include <anki/resource/ShaderResource.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/resource/Material.h>
#include <anki/scene/RenderComponent.h>
#include <anki/scene/Visibility.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/TextureResource.h>
#include <anki/renderer/Renderer.h>
#include <anki/core/Trace.h>
#include <anki/util/Logger.h>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Common stuff to pass to renderSingle
class DrawContext
{
public:
	const FrustumComponent* m_frc = nullptr;
	Pass m_pass;
	CommandBufferPtr m_cmdb;

	Array<Mat4, MAX_INSTANCES> m_cachedTrfs;
	U m_cachedTrfCount = 0;
	const MaterialVariant* m_variant = nullptr;
	TransientMemoryInfo m_dynBufferInfo;
	F32 m_flod = 0.0;
	VisibleNode* m_visibleNode = nullptr;
	VisibleNode* m_nextVisibleNode = nullptr;
};

/// Visitor that sets a uniform
class SetupRenderableVariableVisitor
{
public:
	DrawContext* m_ctx ANKI_DBG_NULLIFY_PTR;
	const RenderableDrawer* m_drawer ANKI_DBG_NULLIFY_PTR;
	WeakArray<U8> m_uniformBuffer;

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const MaterialVariable& mtlVar, const T* value, U32 size)
	{
		mtlVar.writeShaderBlockMemory<T>(*m_ctx->m_variant,
			value,
			size,
			&m_uniformBuffer[0],
			&m_uniformBuffer[0] + m_uniformBuffer.getSize());
	}

	template<typename TRenderableVariableTemplate>
	Error visit(const TRenderableVariableTemplate& rvar);
};

//==============================================================================
template<typename TRenderableVariableTemplate>
Error SetupRenderableVariableVisitor::visit(
	const TRenderableVariableTemplate& rvar)
{
	using DataType = typename TRenderableVariableTemplate::Type;
	const MaterialVariable& mvar = rvar.getMaterialVariable();

	// Array size
	U cachedTrfs = m_ctx->m_cachedTrfCount;
	ANKI_ASSERT(cachedTrfs <= MAX_INSTANCES);

	// Set uniform
	//
	const Mat4& vp = m_ctx->m_frc->getViewProjectionMatrix();
	const Mat4& v = m_ctx->m_frc->getViewMatrix();

	switch(mvar.getBuiltin())
	{
	case BuiltinMaterialVariableId::NONE:
		uniSet<DataType>(mvar, &rvar.getValue(), 1);
		break;
	case BuiltinMaterialVariableId::MVP_MATRIX:
	{
		ANKI_ASSERT(cachedTrfs > 0);

		DynamicArrayAuto<Mat4> mvp(m_drawer->m_r->getFrameAllocator());
		mvp.create(cachedTrfs);

		for(U i = 0; i < cachedTrfs; i++)
		{
			mvp[i] = vp * m_ctx->m_cachedTrfs[i];
		}

		uniSet(mvar, &mvp[0], cachedTrfs);
		break;
	}
	case BuiltinMaterialVariableId::MV_MATRIX:
	{
		ANKI_ASSERT(cachedTrfs > 0);

		DynamicArrayAuto<Mat4> mv(m_drawer->m_r->getFrameAllocator());
		mv.create(cachedTrfs);

		for(U i = 0; i < cachedTrfs; i++)
		{
			mv[i] = v * m_ctx->m_cachedTrfs[i];
		}

		uniSet(mvar, &mv[0], cachedTrfs);
		break;
	}
	case BuiltinMaterialVariableId::VP_MATRIX:
		ANKI_ASSERT(cachedTrfs == 0 && "Cannot have transform");
		uniSet(mvar, &vp, 1);
		break;
	case BuiltinMaterialVariableId::NORMAL_MATRIX:
	{
		ANKI_ASSERT(cachedTrfs > 0);

		DynamicArrayAuto<Mat3> normMats(m_drawer->m_r->getFrameAllocator());
		normMats.create(cachedTrfs);

		for(U i = 0; i < cachedTrfs; i++)
		{
			Mat4 mv = v * m_ctx->m_cachedTrfs[i];
			normMats[i] = mv.getRotationPart();
			normMats[i].reorthogonalize();
		}

		uniSet(mvar, &normMats[0], cachedTrfs);
		break;
	}
	case BuiltinMaterialVariableId::BILLBOARD_MVP_MATRIX:
	{
		// Calc the billboard rotation matrix
		Mat3 rot = v.getRotationPart().getTransposed();

		DynamicArrayAuto<Mat4> bmvp(m_drawer->m_r->getFrameAllocator());
		bmvp.create(cachedTrfs);

		for(U i = 0; i < cachedTrfs; i++)
		{
			Mat4 trf = m_ctx->m_cachedTrfs[i];
			trf.setRotationPart(rot);
			bmvp[i] = vp * trf;
		}

		uniSet(mvar, &bmvp[0], cachedTrfs);
		break;
	}
	case BuiltinMaterialVariableId::MAX_TESS_LEVEL:
	{
		const RenderComponentVariable& base = rvar;
		F32 maxtess = base.getValue<F32>();
		F32 tess = 0.0;

		if(m_ctx->m_flod >= 1.0)
		{
			tess = 1.0;
		}
		else
		{
			tess = maxtess - m_ctx->m_flod * maxtess;
			tess = std::max(tess, 1.0f);
		}

		uniSet(mvar, &tess, 1);
		break;
	}
	case BuiltinMaterialVariableId::MS_DEPTH_MAP:
		// Do nothing
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	return ErrorCode::NONE;
}

//==============================================================================
// Texture specialization
template<>
void SetupRenderableVariableVisitor::uniSet<TextureResourcePtr>(
	const MaterialVariable& mtlvar, const TextureResourcePtr* values, U32 size)
{
	ANKI_ASSERT(size == 1);
	// Do nothing
}

//==============================================================================
// RenderableDrawer                                                            =
//==============================================================================

//==============================================================================
RenderableDrawer::~RenderableDrawer()
{
}

//==============================================================================
void RenderableDrawer::setupUniforms(DrawContext& ctx,
	const RenderComponent& renderable,
	const RenderingKey& key)
{
	const Material& mtl = renderable.getMaterial();
	const MaterialVariant& variant = mtl.getVariant(key);
	ctx.m_variant = &variant;

	// Get some memory for uniforms
	U8* uniforms =
		static_cast<U8*>(m_r->getGrManager().allocateFrameTransientMemory(
			variant.getDefaultBlockSize(),
			BufferUsage::UNIFORM,
			ctx.m_dynBufferInfo.m_uniformBuffers[0]));

	// Call the visitor
	SetupRenderableVariableVisitor visitor;
	visitor.m_ctx = &ctx;
	visitor.m_drawer = this;
	visitor.m_uniformBuffer =
		WeakArray<U8>(uniforms, variant.getDefaultBlockSize());

	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd();
		++it)
	{
		RenderComponentVariable* rvar = *it;

		if(variant.variableActive(rvar->getMaterialVariable()))
		{
			Error err = rvar->acceptVisitor(visitor);
			(void)err;
		}
	}
}

//==============================================================================
Error RenderableDrawer::drawRange(Pass pass,
	const FrustumComponent& frc,
	CommandBufferPtr cmdb,
	VisibleNode* begin,
	VisibleNode* end)
{
	ANKI_ASSERT(begin && end && begin < end);

	DrawContext ctx;
	ctx.m_frc = &frc;
	ctx.m_pass = pass;
	ctx.m_cmdb = cmdb;

	for(; begin != end; ++begin)
	{
		ctx.m_visibleNode = begin;

		if(begin + 1 < end)
		{
			ctx.m_nextVisibleNode = begin + 1;
		}
		else
		{
			ctx.m_nextVisibleNode = nullptr;
		}

		ANKI_CHECK(drawSingle(ctx));
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error RenderableDrawer::drawSingle(DrawContext& ctx)
{
	// Get components
	const RenderComponent& renderable =
		ctx.m_visibleNode->m_node->getComponent<RenderComponent>();
	const Material& mtl = renderable.getMaterial();

	// Check if it can merge drawcalls
	if(ctx.m_nextVisibleNode)
	{
		const RenderComponent& nextRenderable =
			ctx.m_nextVisibleNode->m_node->getComponent<RenderComponent>();

		if(nextRenderable.canMergeDrawcalls(renderable)
			&& ctx.m_cachedTrfCount < MAX_INSTANCES - 1)
		{
			// Can merge, will cache the drawcall and skip the drawcall
			Bool hasTransform;
			Transform trf;
			renderable.getRenderWorldTransform(hasTransform, trf);
			ANKI_ASSERT(hasTransform);

			ctx.m_cachedTrfs[ctx.m_cachedTrfCount] = Mat4(trf);
			++ctx.m_cachedTrfCount;

			return ErrorCode::NONE;
		}
	}

	// Stash the transform
	{
		Bool hasTransform;
		Transform trf;
		renderable.getRenderWorldTransform(hasTransform, trf);

		if(hasTransform)
		{
			ctx.m_cachedTrfs[ctx.m_cachedTrfCount] = Mat4(trf);
			++ctx.m_cachedTrfCount;
		}
	}

	// Calculate the key
	F32 flod =
		m_r->calculateLod(sqrt(ctx.m_visibleNode->m_frustumDistanceSquared));
	flod = min<F32>(flod, MAX_LODS - 1);
	ctx.m_flod = flod;

	RenderingBuildInfo build;
	build.m_key.m_lod = flod;
	build.m_key.m_pass = ctx.m_pass;
	build.m_key.m_tessellation = m_r->getTessellationEnabled()
		&& mtl.getTessellationEnabled() && build.m_key.m_lod == 0;
	build.m_key.m_instanceCount =
		(ctx.m_cachedTrfCount == 0) ? 1 : ctx.m_cachedTrfCount;

	if(ctx.m_pass == Pass::SM && build.m_key.m_lod > 0)
	{
		build.m_key.m_tessellation = false;
	}

	// Enqueue uniform state updates
	setupUniforms(ctx, renderable, build.m_key);

	// Enqueue vertex, program and drawcall
	build.m_subMeshIndicesArray = &ctx.m_visibleNode->m_spatialIndices[0];
	build.m_subMeshIndicesCount = ctx.m_visibleNode->m_spatialsCount;
	build.m_cmdb = ctx.m_cmdb;
	build.m_dynamicBufferInfo = &ctx.m_dynBufferInfo;

	ANKI_CHECK(renderable.buildRendering(build));

	// Rendered something, reset the cached transforms
	if(ctx.m_cachedTrfCount > 1)
	{
		ANKI_TRACE_INC_COUNTER(
			RENDERER_MERGED_DRAWCALLS, ctx.m_cachedTrfCount - 1);
	}
	ctx.m_cachedTrfCount = 0;

	return ErrorCode::NONE;
}

} // end namespace anki
