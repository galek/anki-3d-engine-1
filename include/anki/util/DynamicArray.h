// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/NonCopyable.h>
#include <anki/util/Functions.h>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// Dynamic array with manual destruction. It doesn't hold the allocator and
/// that makes it compact. At the same time that requires manual destruction.
/// Used in permanent classes.
template<typename T>
class DynamicArray : public NonCopyable
{
public:
	using Value = T;
	using Iterator = Value*;
	using ConstIterator = const Value*;
	using Reference = Value&;
	using ConstReference = const Value&;

	DynamicArray()
		: m_data(nullptr)
		, m_size(0)
	{
	}

	/// Move.
	DynamicArray(DynamicArray&& b)
		: DynamicArray()
	{
		move(b);
	}

	~DynamicArray()
	{
		ANKI_ASSERT(
			m_data == nullptr && m_size == 0 && "Requires manual destruction");
	}

	/// Move.
	DynamicArray& operator=(DynamicArray&& b)
	{
		move(b);
		return *this;
	}

	Reference operator[](const PtrSize n)
	{
		ANKI_ASSERT(n < m_size);
		return m_data[n];
	}

	ConstReference operator[](const PtrSize n) const
	{
		ANKI_ASSERT(n < m_size);
		return m_data[n];
	}

	/// Make it compatible with the C++11 range based for loop.
	Iterator getBegin()
	{
		return &m_data[0];
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator getBegin() const
	{
		return &m_data[0];
	}

	/// Make it compatible with the C++11 range based for loop.
	Iterator getEnd()
	{
		return &m_data[0] + m_size;
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator getEnd() const
	{
		return &m_data[0] + m_size;
	}

	/// Make it compatible with the C++11 range based for loop.
	Iterator begin()
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator begin() const
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop.
	Iterator end()
	{
		return getEnd();
	}

	/// Make it compatible with the C++11 range based for loop.
	ConstIterator end() const
	{
		return getEnd();
	}

	/// Get first element.
	Reference getFront()
	{
		return m_data[0];
	}

	/// Get first element.
	ConstReference getFront() const
	{
		return m_data[0];
	}

	/// Get last element.
	Reference getBack()
	{
		return m_data[m_size - 1];
	}

	/// Get last element.
	ConstReference getBack() const
	{
		return m_data[m_size - 1];
	}

	/// Get last element.
	ConstReference back() const
	{
		return m_data[m_size - 1];
	}

	PtrSize getSize() const
	{
		return m_size;
	}

	PtrSize getByteSize() const
	{
		return m_size * sizeof(Value);
	}

	Bool isEmpty() const
	{
		return m_size == 0;
	}

	PtrSize getSizeInBytes() const
	{
		return m_size * sizeof(Value);
	}

	/// Create the array.
	template<typename TAllocator>
	void create(TAllocator alloc, PtrSize size)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0);
		destroy(alloc);

		if(size > 0)
		{
			m_data = alloc.template newArray<Value>(size);
			m_size = size;
		}
	}

	/// Create the array.
	template<typename TAllocator>
	void create(TAllocator alloc, PtrSize size, const Value& v)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0);

		if(size > 0)
		{
			m_data = alloc.template newArray<Value>(size, v);
			m_size = size;
		}
	}

	/// Grow the array.
	template<typename TAllocator>
	void resize(TAllocator alloc, PtrSize size)
	{
		ANKI_ASSERT(size > 0);
		DynamicArray newArr;
		newArr.create(alloc, size);

		PtrSize minSize = min<PtrSize>(size, m_size);
		for(U i = 0; i < minSize; i++)
		{
			newArr[i] = std::move((*this)[i]);
		}

		destroy(alloc);
		move(newArr);
	}

	/// Destroy the array.
	template<typename TAllocator>
	void destroy(TAllocator alloc)
	{
		if(m_data)
		{
			ANKI_ASSERT(m_size > 0);
			alloc.deleteArray(m_data, m_size);

			m_data = nullptr;
			m_size = 0;
		}

		ANKI_ASSERT(m_data == nullptr && m_size == 0);
	}

protected:
	Value* m_data;
	U32 m_size;

	void move(DynamicArray& b)
	{
		ANKI_ASSERT(m_data == nullptr && m_size == 0
			&& "Cannot move before destroying");
		m_data = b.m_data;
		b.m_data = nullptr;
		m_size = b.m_size;
		b.m_size = 0;
	}
};

/// Dynamic array with automatic destruction. It's the same as DynamicArray but
/// it holds the allocator in order to perform automatic destruction. Use it for
/// temp operations and on transient classes.
template<typename T>
class DynamicArrayAuto : public DynamicArray<T>
{
public:
	using Base = DynamicArray<T>;
	using Value = T;

	template<typename TAllocator>
	DynamicArrayAuto(TAllocator alloc)
		: Base()
		, m_alloc(alloc)
	{
	}

	/// Move.
	DynamicArrayAuto(DynamicArrayAuto&& b)
		: DynamicArrayAuto()
	{
		move(b);
	}

	~DynamicArrayAuto()
	{
		Base::destroy(m_alloc);
	}

	/// Move.
	DynamicArrayAuto& operator=(DynamicArrayAuto&& b)
	{
		move(b);
		return *this;
	}

	/// Create the array.
	void create(PtrSize size)
	{
		Base::create(m_alloc, size);
	}

	/// Create the array.
	void create(PtrSize size, const Value& v)
	{
		Base::create(m_alloc, size, v);
	}

	/// Grow the array.
	void resize(PtrSize size)
	{
		Base::resize(m_alloc, size);
	}

private:
	GenericMemoryPoolAllocator<T> m_alloc;

	void move(DynamicArrayAuto& b)
	{
		Base::move(b);
		m_alloc = b.m_alloc;
	}
};

/// Array with preallocated memory.
template<typename T>
class WeakArray : public DynamicArray<T>
{
public:
	using Base = DynamicArray<T>;
	using Value = T;

	WeakArray()
		: Base()
	{
	}

	WeakArray(T* mem, PtrSize size)
		: Base()
	{
		if(size)
		{
			ANKI_ASSERT(mem);
		}

		Base::m_data = mem;
		Base::m_size = size;
	}

	/// Copy.
	WeakArray(const WeakArray& b)
		: Base::m_data(b.m_data)
		, Base::m_size(b.m_size)
	{
	}

	/// Move.
	WeakArray(WeakArray&& b)
	{
		*this = std::move(b);
	}

	~WeakArray()
	{
#if ANKI_ASSERTIONS
		Base::m_data = nullptr;
		Base::m_size = 0;
#endif
	}

	/// Copy.
	WeakArray& operator=(const WeakArray& b)
	{
		Base::m_data = b.m_data;
		Base::m_size = b.m_size;
		return *this;
	}

	/// Move.
	WeakArray& operator=(WeakArray&& b)
	{
		Base::m_data = b.m_data;
		b.m_data = nullptr;
		Base::m_size = b.m_size;
		b.m_size = 0;
		return *this;
	}
};
/// @}

} // end namespace anki
