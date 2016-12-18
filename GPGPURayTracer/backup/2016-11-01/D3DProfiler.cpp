#include "D3DProfiler.h"
#include "WinTimer.h"
#include <string>
#include "Global.h"

D3DProfiler::D3DProfiler()
{
}

void D3DProfiler::init(Graphics *g)
{
	this->g = g;
	timer = new WinTimer();
}

D3DProfiler::~D3DProfiler()
{
	delete timer;
}

void D3DProfiler::startProfile(const std::string& name)
{
	ProfileData& profileData = profiles[name];

	if (profileData.queryStarted)
	{
		MessageBox(NULL, "profileData query already started", "D3DProfiler Error", S_OK);
		return;
	}
	if(profileData.queryFinished)
	{
		MessageBox(NULL, "profileData query already finished", "D3DProfiler Error", S_OK);
		return;
	}

	if (profileData.disjointQuery[currFrame] == NULL)
	{
		// Create the queries
		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		desc.MiscFlags = 0;
		HRESULT hr;
		hr = g->getDevice()->CreateQuery(&desc, &profileData.disjointQuery[currFrame]);
		if (FAILED(hr))
		{
			MessageBox(NULL, "Cannot create disjoint query", "D3DProfiler Error", S_OK);
			return;
		}
		desc.Query = D3D11_QUERY_TIMESTAMP;
		hr = g->getDevice()->CreateQuery(&desc, &profileData.timestampStartQuery[currFrame]);
		if (FAILED(hr))
		{
			MessageBox(NULL, "Cannot create timestamp query", "D3DProfiler Error", S_OK);
			return;
		}
		hr = g->getDevice()->CreateQuery(&desc, &profileData.timestampEndQuery[currFrame]);
		if (FAILED(hr))
		{
			MessageBox(NULL, "Cannot create timestamp end query", "D3DProfiler Error", S_OK);
			return;
		}
	}

	// Start a disjoint query first
	g->getDeviceContext()->Begin(profileData.disjointQuery[currFrame]);

	// Insert the start timestamp    
	g->getDeviceContext()->End(profileData.timestampStartQuery[currFrame]);

	profileData.queryStarted = true;
}
void D3DProfiler::stopProfile(const std::string& name)
{
	ProfileData& profileData = profiles[name];
	if (!profileData.queryStarted)
	{
		MessageBox(NULL, "profileData query not started", "D3DProfiler Error", S_OK);
		return;
	}
	if (profileData.queryFinished)
	{
		MessageBox(NULL, "profileData query already finished", "D3DProfiler Error", S_OK);
		return;
	}

	// Insert the end timestamp    
	g->getDeviceContext()->End(profileData.timestampEndQuery[currFrame]);

	// End the disjoint query
	g->getDeviceContext()->End(profileData.disjointQuery[currFrame]);

	profileData.queryStarted = FALSE;
	profileData.queryFinished = TRUE;
}
void D3DProfiler::report(FILE *file)
{
	currFrame = (currFrame + 1) % QUERYLATENCY;

	float queryTime = 0.0f;
	fputs("Initiating query reports\n", file);
	// Iterate over all of the profiles
	ProfileMap::iterator iter;
	for (iter = profiles.begin(); iter != profiles.end(); iter++)
	{
		ProfileData& profile = (*iter).second;
		if (profile.queryFinished == FALSE)
			continue;

		profile.queryFinished = FALSE;

		if (profile.disjointQuery[currFrame] == NULL)
			continue;

		timer->update();

		// Get the query data
		UINT64 startTime = 0;
		while (g->getDeviceContext()->GetData(profile.timestampStartQuery[currFrame], &startTime, sizeof(startTime), 0) != S_OK);

		UINT64 endTime = 0;
		while (g->getDeviceContext()->GetData(profile.timestampEndQuery[currFrame], &endTime, sizeof(endTime), 0) != S_OK);

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
		while (g->getDeviceContext()->GetData(profile.disjointQuery[currFrame], &disjointData, sizeof(disjointData), 0) != S_OK);

		timer->update();
		queryTime += timer->getDeltaMicroSecondsF();

		float time = 0.0f;
		if (disjointData.Disjoint == FALSE)
		{
			UINT64 delta = endTime - startTime;
			float frequency = static_cast<float>(disjointData.Frequency);
			time = (delta / frequency) * 1000.0f;
		}
		std::string output = "query[" + (*iter).first + "]: " + ToString(queryTime) + " ms\n";
		//printf("query [%s]: %d ms\n", (*iter).first, queryTime);
		fputs(output.c_str(), file);
	}
	if (queryTime > 0.0)
	{
		std::string output = "Time spent waiting for queries:" + ToString(queryTime) + " ms\n";
		fputs(output.c_str(), file);
	}
}