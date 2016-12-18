#include <string>
#include "Graphics.h"
#include "ITimer.h"
class D3DProfiler
{
protected:
	Graphics *g;
	static const UINT64 QUERYLATENCY = 5;

	struct ProfileData
	{
		ID3D11Query *disjointQuery[QUERYLATENCY];
		ID3D11Query *timestampStartQuery[QUERYLATENCY];
		ID3D11Query *timestampEndQuery[QUERYLATENCY];
		bool queryStarted;
		bool queryFinished;

		ProfileData() : queryStarted(FALSE), queryFinished(FALSE)
		{
			for (unsigned int i = 0; i < QUERYLATENCY; i++)
				disjointQuery[i] = NULL;
		}
	};
	typedef std::map<std::string, ProfileData> ProfileMap;

	ProfileMap profiles;
	UINT64 currFrame;

	ITimer *timer;
public:
	D3DProfiler();
	~D3DProfiler();
	void init(Graphics *g);
	void startProfile(const std::string& name);
	void stopProfile(const std::string& name);
	void report(FILE *file);
};