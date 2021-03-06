// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/gr/GrObjectCache.h>
#include <anki/gr/GrObject.h>
#include <anki/util/String.h>

namespace anki
{

// Forward
class ConfigSet;
class NativeWindow;

/// @addtogroup graphics
/// @{

/// Manager initializer.
class GrManagerInitInfo
{
public:
	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;

	CString m_cacheDirectory;

	const ConfigSet* m_config = nullptr;
	NativeWindow* m_window = nullptr;
};

/// The graphics manager, owner of all graphics objects.
class GrManager
{
	friend class GrManagerImpl;

	template<typename>
	friend class GrObjectPtrDeleter;

public:
	/// Default constructor
	GrManager();

	~GrManager();

	/// Create.
	ANKI_USE_RESULT Error init(GrManagerInitInfo& init);

	/// Swap buffers
	void swapBuffers();

	/// Wait for all work to finish.
	void finish();

	/// Create a new graphics object.
	template<typename T, typename... Args>
	GrObjectPtr<T> newInstance(Args&&... args);

	/// Create a new graphics object and use the cache to avoid duplication.
	/// It's thread safe.
	template<typename T, typename TArg>
	GrObjectPtr<T> newInstanceCached(const TArg& arg);

	/// Allocate memory for dynamic buffers. The memory will be reclaimed at
	/// the begining of the N-(MAX_FRAMES_IN_FLIGHT-1) frame.
	void* allocateFrameTransientMemory(PtrSize size,
		BufferUsage usage,
		TransientMemoryToken& token,
		Error* err = nullptr);

anki_internal:
	GrAllocator<U8>& getAllocator()
	{
		return m_alloc;
	}

	GrManagerImpl& getImplementation()
	{
		return *m_impl;
	}

	const GrManagerImpl& getImplementation() const
	{
		return *m_impl;
	}

	CString getCacheDirectory() const
	{
		return m_cacheDir.toCString();
	}

	U64& getUuidIndex()
	{
		return m_uuidIndex;
	}

	void unregisterCachedObject(GrObject* ptr)
	{
		ANKI_ASSERT(ptr);
		if(ptr->getHash() != 0)
		{
			GrObjectCache& cache = m_caches[ptr->getType()];
			LockGuard<Mutex> lock(cache.getMutex());
			cache.unregisterObject(ptr);
		}
	}

private:
	GrAllocator<U8> m_alloc; ///< Keep it first to get deleted last
	UniquePtr<GrManagerImpl> m_impl;
	String m_cacheDir;
	U64 m_uuidIndex = 1;
	Array<GrObjectCache, U(GrObjectType::COUNT)> m_caches;
};

//==============================================================================
template<typename T, typename... Args>
GrObjectPtr<T> GrManager::newInstance(Args&&... args)
{
	const U64 hash = 0;
	GrObjectPtr<T> ptr(m_alloc.newInstance<T>(this, hash));
	ptr->init(args...);
	return ptr;
}

//==============================================================================
template<typename T, typename TArg>
GrObjectPtr<T> GrManager::newInstanceCached(const TArg& arg)
{
	GrObjectCache& cache = m_caches[T::CLASS_TYPE];
	U64 hash = arg.computeHash();
	ANKI_ASSERT(hash != 0);

	LockGuard<Mutex> lock(cache.getMutex());
	GrObject* ptr = cache.tryFind(hash);
	if(ptr == nullptr)
	{
		T* tptr = m_alloc.newInstance<T>(this, hash);
		tptr->init(arg);
		ptr = tptr;
		cache.registerObject(ptr);
	}
	else
	{
		ANKI_ASSERT(ptr->getType() == T::CLASS_TYPE);
	}

	return GrObjectPtr<T>(static_cast<T*>(ptr));
}
/// @}

} // end namespace anki
