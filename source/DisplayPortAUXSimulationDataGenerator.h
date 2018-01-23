#ifndef DISPLAYPORTAUX_SIMULATION_DATA_GENERATOR
#define DISPLAYPORTAUX_SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>

class DisplayPortAUXAnalyzerSettings;

class DisplayPortAUXSimulationDataGenerator
{
public:
	DisplayPortAUXSimulationDataGenerator();
	~DisplayPortAUXSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, DisplayPortAUXAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );

protected:

	U64 UsToSamples( U64 us );
	U64 UsToSamples( double us );
	U64 SamplesToUs( U64 samples );

	void SimWriteByte( U64 value );
	void SimWriteBit( U32 bit );
	void SimWriteStartStop();

	U64 mT;
	U64 mSimValue;

	DisplayPortAUXAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

	SimulationChannelDescriptor mDisplayPortAUXSimulationData; 
};

#endif //DISPLAYPORTAUX_SIMULATION_DATA_GENERATOR
