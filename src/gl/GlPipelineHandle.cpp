// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlPipelineHandle.h"
#include "anki/gl/GlPipeline.h"
#include "anki/gl/GlHandleDeferredDeleter.h"

namespace anki {

//==============================================================================
GlPipelineHandle::GlPipelineHandle()
{}

//==============================================================================
GlPipelineHandle::~GlPipelineHandle()
{}

//==============================================================================
Error GlPipelineHandle::create(
	GlCommandBufferHandle& commands,
	std::initializer_list<GlShaderHandle> iprogs)
{
	Array<GlShaderHandle, 6> progs;

	U count = 0;
	for(GlShaderHandle prog : iprogs)
	{
		progs[count++] = prog;
	}

	return commonConstructor(commands, &progs[0], &progs[0] + count);
}

//==============================================================================
Error GlPipelineHandle::commonConstructor(
	GlCommandBufferHandle& commands,
	const GlShaderHandle* progsBegin, const GlShaderHandle* progsEnd)
{
	class Command: public GlCommand
	{
	public:
		GlPipelineHandle m_ppline;
		Array<GlShaderHandle, 6> m_progs;
		U8 m_progsCount;

		Command(GlPipelineHandle& ppline, 
			const GlShaderHandle* progsBegin, const GlShaderHandle* progsEnd)
		:	m_ppline(ppline)
		{
			m_progsCount = 0;
			const GlShaderHandle* prog = progsBegin;
			do
			{
				m_progs[m_progsCount++] = *prog;
			} while(++prog != progsEnd);
		}

		Error operator()(GlCommandBuffer* cmdb)
		{
			Error err = m_ppline._get().create(
				&m_progs[0], &m_progs[0] + m_progsCount,
				cmdb->getGlobalAllocator());

			GlHandleState oldState = m_ppline._setState(
				err ? GlHandleState::ERROR : GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	using Alloc = GlAllocator<GlPipeline>;
	using DeleteCommand = GlDeleteObjectCommand<GlPipeline, Alloc>;
	using Deleter = 
		GlHandleDeferredDeleter<GlPipeline, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._get().getQueue().getDevice(),
		commands._get().getGlobalAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		commands._pushBackNewCommand<Command>(*this, progsBegin, progsEnd);
	}

	return err;
}

//==============================================================================
void GlPipelineHandle::bind(GlCommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());

	class Command: public GlCommand
	{
	public:
		GlPipelineHandle m_ppline;

		Command(GlPipelineHandle& ppline)
		:	m_ppline(ppline)
		{}

		Error operator()(GlCommandBuffer* commands)
		{
			GlState& state = commands->getQueue().getState();

			if(state.m_crntPpline != m_ppline._get().getGlName())
			{
				m_ppline._get().bind();

				state.m_crntPpline = m_ppline._get().getGlName();
			}

			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
GlShaderHandle GlPipelineHandle::getAttachedProgram(GLenum type) const
{
	ANKI_ASSERT(isCreated());
	Error err = serializeOnGetter();
	if(!err)
	{
		return _get().getAttachedProgram(type);
	}
	else
	{
		return GlShaderHandle();
	}
}

} // end namespace anki
