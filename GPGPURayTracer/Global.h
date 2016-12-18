#pragma once
#ifndef d_Global
	#define d_Global
	#include <cstdlib>
	#include <sstream>
	#include <assert.h>
	#define DEFAULTSCRNWIDTH (800)
	#define DEFAULTSCRNHEIGHT (800)

	#define	SAFE_RELEASE(x)	if( x ) { (x)->Release();	(x) = NULL; }
	#define SAFE_DELETE(x)	if( x ) { delete(x);		(x) = NULL; }
	static float randf()
	{
		return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}
	static float randf(float max)
	{
		return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / max));
	}
	static float randf(float min, float max)
	{
		return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	}
	// Converts a number to a wstring
	template<typename T> inline std::wstring ToWString(const T& val)
	{
		std::wostringstream stream;
		assert(stream << val);
		return stream.str();
	}
	template<typename T> inline std::string ToString(const T& val)
	{
		std::ostringstream stream;
		assert(stream << val);
		return stream.str();
	}
#endif // !d_Global

