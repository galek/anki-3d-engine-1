// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/System.h>
#include <anki/Config.h>
#include <cstdio>

#if ANKI_POSIX
#include <unistd.h>
#include <signal.h>
#elif ANKI_OS == ANKI_OS_WINDOWS
#include <Windows.h>
#else
#error "Unimplemented"
#endif

// For print backtrace
#if ANKI_POSIX && ANKI_OS != ANKI_OS_ANDROID
#include <execinfo.h>
#endif

namespace anki
{

//==============================================================================
U32 getCpuCoresCount()
{
#if ANKI_POSIX
	return sysconf(_SC_NPROCESSORS_ONLN);
#elif ANKI_OS == ANKI_OS_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#else
#error "Unimplemented"
#endif
}

//==============================================================================
void printBacktrace()
{
#if ANKI_POSIX && ANKI_OS != ANKI_OS_ANDROID
	void* array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	backtrace_symbols_fd(array, size, 2);
#else
	printf("TODO\n");
#endif
}

} // end namespace anki
