#include "RsrcPtr.h"
#include "util/Exception.h"
#include "util/Assert.h"
#include <boost/checked_delete.hpp>


//==============================================================================
// Constructor                                                                 =
//==============================================================================
template<typename Type>
RsrcPtr<Type>::RsrcPtr(const RsrcPtr& a):
	hook(a.hook)
{
	if(hook)
	{
		++hook->referenceCounter;
	}
}


//==============================================================================
// Constructor                                                                 =
//==============================================================================
template<typename Type>
RsrcPtr<Type>::~RsrcPtr()
{
	unload();
}


//==============================================================================
// operator*                                                                   =
//==============================================================================
template<typename Type>
Type& RsrcPtr<Type>::operator*() const
{
	ASSERT(hook != NULL);
	return *hook->resource;
}


//==============================================================================
// operator->                                                                  =
//==============================================================================
template<typename Type>
Type* RsrcPtr<Type>::operator->() const
{
	ASSERT(hook != NULL);
	return hook->resource;
}


//==============================================================================
// loadRsrc                                                                    =
//==============================================================================
template<typename Type>
void RsrcPtr<Type>::loadRsrc(const char* filename)
{
	ASSERT(hook == NULL);
	hook = &ResourceManagerSingleton::get().load<Type>(filename);
}


//==============================================================================
// unload                                                                      =
//==============================================================================
template<typename Type>
void RsrcPtr<Type>::unload()
{
	if(hook != NULL)
	{
		ResourceManagerSingleton::get().unload<Type>(*hook);
		hook = NULL;
	}
}


//==============================================================================
// getRsrcName                                                                 =
//==============================================================================
template<typename Type>
const std::string& RsrcPtr<Type>::getRsrcName() const
{
	ASSERT(hook != NULL);
	return hook->uuid;
}