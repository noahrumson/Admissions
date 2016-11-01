#ifndef BUILD_H_INCLUDED
#define BUILD_H_INCLUDED

#ifdef _WIN32
#	ifdef _WIN64
#		define BUILD_64
#	else
#		define BUILD_32
#	endif
#endif

#endif