#pragma once
#include "ITimer.h"
class WinTimer :
	public ITimer
{
public: //public functions
	WinTimer();
	~WinTimer() {}
	void	update()					override;
	float	getElapsedSecondsF() 		const override;
	double	getElapsedSecondsD()		const override;
	signed long long	getElapsedSecondsI()		const override;
	float	getDeltaSecondsF()			const override;
	double	getDeltaSecondsD()			const override;
	signed long long	getDeltaSecondsI()			const override;
	float	getElapsedMilliSecondsF()	const override;
	double	getElapsedMilliSecondsD()	const override;
	signed long long	getElapsedMilliSecondsI()	const override;
	float	getDeltaMilliSecondsF()		const override;
	double	getDeltaMilliSecondsD()		const override;
	signed long long	getDeltaMilliSecondsI()		const override;
	float	getElapsedMicroSecondsF()	const override;
	double	getElapsedMicroSecondsD()	const override;
	signed long long	getElapsedMicroSecondsI()	const override;
	float	getDeltaMicroSecondsF()		const override;
	double	getDeltaMicroSecondsD()		const override;
	signed long long	getDeltaMicroSecondsI()		const override;

private: //private variables
	signed long long	startTime;
	signed long long	frequency;
	double	frequencyD;
	signed long long	elapsed;
	signed long long	delta;
	float	elapsedF;
	float	deltaF;
	double	elapsedD;
	double	deltaD;
	signed long long	elapsedSeconds;
	signed long long	deltaSeconds;
	float	elapsedSecondsF;
	float	deltaSecondsF;
	double	elapsedSecondsD;
	double	deltaSecondsD;
	signed long long	elapsedMilliseconds;
	signed long long	deltaMilliseconds;
	float	elapsedMillisecondsF;
	float	deltaMillisecondsF;
	double	elapsedMillisecondsD;
	double	deltaMillisecondsD;
	signed long long	elapsedMicroseconds;
	signed long long	deltaMicroseconds;
	float	elapsedMicrosecondsF;
	float	deltaMicrosecondsF;
	double	elapsedMicrosecondsD;
	double	deltaMicrosecondsD;
};

