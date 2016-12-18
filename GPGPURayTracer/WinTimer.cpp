#include "WinTimer.h"
#include <Windows.h>
#include <assert.h>

WinTimer::WinTimer()
{
	//force this thread to keep the same logical processor. (neccessary to avoid problems with the query performance counter)
	HANDLE handle = GetCurrentThread();
	assert(SetThreadAffinityMask(handle, 1) != 0);

	// get frequency
	LARGE_INTEGER largeInt;
	assert(QueryPerformanceFrequency(&largeInt));
	frequency = largeInt.QuadPart;
	frequencyD = static_cast<double>(frequency);

	// start
	assert(QueryPerformanceCounter(&largeInt));
	startTime = largeInt.QuadPart;
	elapsed = largeInt.QuadPart - startTime;
	elapsedF = static_cast<float>(elapsed);
	elapsedSeconds = elapsed / frequency;
	elapsedSecondsD = elapsed / frequencyD;
	elapsedSecondsF = static_cast<float>(elapsedSecondsD);
	elapsedMilliseconds = static_cast<INT64>(elapsedSecondsD * 1000);
	elapsedMillisecondsD = elapsedSecondsD * 1000;
	elapsedMillisecondsF = static_cast<float>(elapsedMillisecondsD);
	elapsedMicroseconds = static_cast<INT64>(elapsedMillisecondsD * 1000);
	elapsedMicrosecondsD = elapsedMillisecondsD * 1000;
	elapsedMicrosecondsF = static_cast<float>(elapsedMillisecondsD);

	delta = 0;
	deltaF = 0;
	deltaMilliseconds = 0;
	deltaMillisecondsF = 0;
	deltaMicroseconds = 0;
	deltaMicrosecondsF = 0;
}

void WinTimer::update()
{
	LARGE_INTEGER largeInt;
	assert(QueryPerformanceFrequency(&largeInt));
	signed long long currentTime = largeInt.QuadPart - startTime;
	delta = currentTime - elapsed;

	deltaF = static_cast<float>(deltaF);
	deltaSeconds = delta / frequency;
	deltaSecondsD = delta / frequencyD;
	deltaSecondsF = static_cast<float>(deltaSecondsD);
	deltaMillisecondsD = deltaSecondsD * 1000;
	deltaMilliseconds = static_cast<INT64>(deltaMillisecondsD);
	deltaMillisecondsF = static_cast<float>(deltaMillisecondsD);
	deltaMicrosecondsD = deltaMillisecondsD * 1000;
	deltaMicroseconds = static_cast<INT64>(deltaMicrosecondsD);
	deltaMicrosecondsF = static_cast<float>(deltaMicrosecondsD);

	elapsed = currentTime;
	elapsedF = static_cast<float>(elapsed);
	elapsedSeconds = elapsed / frequency;
	elapsedSecondsD = elapsed / frequencyD;
	elapsedSecondsF = static_cast<float>(elapsedSecondsD);
	elapsedMilliseconds = static_cast<INT64>(elapsedSecondsD * 1000);
	elapsedMillisecondsD = elapsedSecondsD * 1000;
	elapsedMillisecondsF = static_cast<float>(elapsedMillisecondsD);
	elapsedMicroseconds = static_cast<INT64>(elapsedMillisecondsD * 1000);
	elapsedMicrosecondsD = elapsedMillisecondsD * 1000;
	elapsedMicrosecondsF = static_cast<float>(elapsedMillisecondsD);
}

float	WinTimer::getElapsedSecondsF() const { return elapsedSecondsF; }
double	WinTimer::getElapsedSecondsD() const { return elapsedSecondsD; }
__int64	WinTimer::getElapsedSecondsI() const { return elapsedSeconds; }
float	WinTimer::getDeltaSecondsF() const { return deltaSecondsF; }
double	WinTimer::getDeltaSecondsD() const { return deltaSecondsD; }
__int64	WinTimer::getDeltaSecondsI() const { return deltaSeconds; }
float	WinTimer::getElapsedMilliSecondsF() const { return elapsedMillisecondsF; }
double	WinTimer::getElapsedMilliSecondsD() const { return elapsedMillisecondsD; }
__int64	WinTimer::getElapsedMilliSecondsI() const { return elapsedMilliseconds; }
float	WinTimer::getDeltaMilliSecondsF() const { return deltaMillisecondsF; }
double	WinTimer::getDeltaMilliSecondsD() const { return deltaMillisecondsD; }
__int64	WinTimer::getDeltaMilliSecondsI() const { return deltaMilliseconds; }
float	WinTimer::getElapsedMicroSecondsF() const { return elapsedMicrosecondsF; }
double	WinTimer::getElapsedMicroSecondsD() const { return elapsedMicrosecondsD; }
__int64	WinTimer::getElapsedMicroSecondsI() const { return elapsedMicroseconds; }
float	WinTimer::getDeltaMicroSecondsF() const { return deltaMicrosecondsF; }
double	WinTimer::getDeltaMicroSecondsD() const { return deltaMicrosecondsD; }
__int64	WinTimer::getDeltaMicroSecondsI() const { return deltaMicroseconds; }