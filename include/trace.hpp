#ifndef AUTONOMIC_FARM_TRACE_HPP
#define AUTONOMIC_FARM_TRACE_HPP


#if not defined ENABLE_TRACE
#define TRACEF( format, ... )
#define TRACE( ... )
#else
#include <cstdlib>
#define TRACEF( format, ... ) printf("[%s::%s(%d)] " format "\n", __FILE__, __FUNCTION__,  __LINE__, __VA_ARGS__ )
#define TRACE( str ) printf("[%s::%s(%d)] " str "\n", __FILE__, __FUNCTION__,  __LINE__ )
#endif


#endif //AUTONOMIC_FARM_TRACE_HPP
