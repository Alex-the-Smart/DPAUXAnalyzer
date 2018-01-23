#ifndef DISPLAYPORTAUX_ANALYZER
#define DISPLAYPORTAUX_ANALYZER

#ifdef WIN32
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT
	#define __cdecl
	#define __stdcall
	#define __fastcall
#endif

#include "Analyzer.h"
#include "DisplayPortAUXAnalyzerResults.h"
#include "DisplayPortAUXSimulationDataGenerator.h"

enum DisplayPortAUXFrameType { AUXSync, AUXStart, AUXData, AUXStop };

class DisplayPortAUXAnalyzerSettings;


class ANALYZER_EXPORT DisplayPortAUXAnalyzer : public Analyzer2
{
public:
	DisplayPortAUXAnalyzer();
	virtual ~DisplayPortAUXAnalyzer();
	virtual void SetupResults();
	virtual void WorkerThread();
	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();
	virtual bool NeedsRerun();
	virtual const char* GetAnalyzerName() const;
	
#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'DisplayPortAUXAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class
protected:

	void ProcessManchesterData();
	void SynchronizeManchester();
	void ProcessFaux();
	void SynchronizeFaux();
	void SaveBit( U64 location, U32 value );
	void Invalidate();
	AnalyzerChannelData* mDisplayPortAUX;

	std::auto_ptr< DisplayPortAUXAnalyzerSettings > mSettings;
	std::auto_ptr< DisplayPortAUXAnalyzerResults > mResults;

	DisplayPortAUXSimulationDataGenerator mSimulationDataGenerator;

	U32 mSampleRateHz;

	bool mSimulationInitilized;

	U32 mT;
	U32 mTError;
	std::vector< std::pair< U64, U64 > > mBitsForNextByte; //value, location
	std::vector<U64> mUnsyncedLocations;
	bool mSynchronized;
	U32 mIgnoreBitCount;
#pragma warning( pop )
};
extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName( );
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );
#endif //DISPLAYPORTAUX_ANALYZER
