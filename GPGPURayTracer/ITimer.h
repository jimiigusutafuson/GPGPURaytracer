#pragma once
class ITimer
{
public:
	virtual ~ITimer() {}
	//seconds
	virtual float	getElapsedSecondsF() const = 0;
	virtual double	getElapsedSecondsD() const = 0;
	virtual __int64	getElapsedSecondsI() const = 0;
	virtual float	getDeltaSecondsF() const = 0;
	virtual double	getDeltaSecondsD() const = 0;
	virtual __int64	getDeltaSecondsI() const = 0;
	//milliseconds
	virtual float	getElapsedMilliSecondsF() const = 0;
	virtual double	getElapsedMilliSecondsD() const = 0;
	virtual __int64	getElapsedMilliSecondsI() const = 0;
	virtual float	getDeltaMilliSecondsF() const = 0;
	virtual double	getDeltaMilliSecondsD() const = 0;
	virtual __int64	getDeltaMilliSecondsI() const = 0;
	//microseconds
	virtual float	getElapsedMicroSecondsF() const = 0;
	virtual double	getElapsedMicroSecondsD() const = 0;
	virtual __int64	getElapsedMicroSecondsI() const = 0;
	virtual float	getDeltaMicroSecondsF() const = 0;
	virtual double	getDeltaMicroSecondsD() const = 0;
	virtual __int64	getDeltaMicroSecondsI() const = 0;
	//utility
	virtual void	update() = 0;
};

