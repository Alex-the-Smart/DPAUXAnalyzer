#include "DisplayPortAUXSimulationDataGenerator.h"
#include "DisplayPortAUXAnalyzerSettings.h"

DisplayPortAUXSimulationDataGenerator::DisplayPortAUXSimulationDataGenerator()
{

}

DisplayPortAUXSimulationDataGenerator::~DisplayPortAUXSimulationDataGenerator()
{

}

void DisplayPortAUXSimulationDataGenerator::Initialize( U32 simulation_sample_rate, DisplayPortAUXAnalyzerSettings* settings )
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	mDisplayPortAUXSimulationData.SetChannel( mSettings->mInputChannel );
	mDisplayPortAUXSimulationData.SetSampleRate( simulation_sample_rate );

	mDisplayPortAUXSimulationData.SetInitialBitState( BIT_LOW );

	double half_period = 1.0 / double( mSettings->mBitRate * 2 );	// Calculate half period in seconds
	half_period *= 1000000.0;			// Convert to microseconds
	mT = UsToSamples( half_period );	// Convert to sample count
	mSimValue = 1;

	if ( mSettings->mBitsPerTransfer > 32 )
	{
		mSimValue = 0xFFFFFFFF;

	}

	mDisplayPortAUXSimulationData.Advance( U32(mT * 16) );	// Make pause of 8 bit periods
}

U32 DisplayPortAUXSimulationDataGenerator::GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( newest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while( mDisplayPortAUXSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
	{
		// Generating Precharge and Sync 0s
		for( U32 i = 0; i < 32; ++i )	// TO DO: add setting i < mSettings->mPrechargeBits
			SimWriteBit( 0 );
		
		// Generating START symbol
		SimWriteStartStop();

		// Generating simulation data
		SimWriteByte( mSimValue++ );	// Simulating counter, value 1
		SimWriteByte( mSimValue++ );	// Simulating counter, value 2
		SimWriteByte( mSimValue++ );	// Simulating counter, value 3
		SimWriteByte( mSimValue++ );	// Simulating counter, value 4

		// Generating STOP symbol
		SimWriteStartStop();

		mDisplayPortAUXSimulationData.Advance( U32(mT * 16) );	// Make pause of 8 bit periods
	}
	*simulation_channels = &mDisplayPortAUXSimulationData;	// Result
	return 1;
}

U64 DisplayPortAUXSimulationDataGenerator::UsToSamples( U64 us )
{
	return ( mSimulationSampleRateHz * us ) / 1000000;
}

U64 DisplayPortAUXSimulationDataGenerator::UsToSamples( double us )
{
	return U64(( mSimulationSampleRateHz * us ) / 1000000.0);
}

U64 DisplayPortAUXSimulationDataGenerator::SamplesToUs( U64 samples )
{
	return( samples * 1000000 ) / mSimulationSampleRateHz;
}

void DisplayPortAUXSimulationDataGenerator::SimWriteStartStop()
{
	mDisplayPortAUXSimulationData.TransitionIfNeeded(BIT_HIGH);
	mDisplayPortAUXSimulationData.Advance( U32(mT * 4) );	// Make 2 periods of 1s
	mDisplayPortAUXSimulationData.TransitionIfNeeded(BIT_LOW);
	mDisplayPortAUXSimulationData.Advance( U32(mT * 4) );	// Make 2 periods of 0s
}

void DisplayPortAUXSimulationDataGenerator::SimWriteByte( U64 value )
{
	U32 bits_per_xfer = mSettings->mBitsPerTransfer;

	for( U32 i = 0; i < bits_per_xfer; ++i )
	{
		if( mSettings->mShiftOrder == AnalyzerEnums::MsbFirst )
		{
			SimWriteBit( ( value >> (bits_per_xfer - i - 1) ) & 0x1 );
		}
	}
}

void DisplayPortAUXSimulationDataGenerator::SimWriteBit( U32 bit )
{
	BitState start_bit_state = mDisplayPortAUXSimulationData.GetCurrentBitState();
	switch( mSettings->mMode )
	{
	case Manchester:
		{
			if( mSettings->mInverted == false )
			{
				if( ( bit == 0 ) && ( start_bit_state == BIT_HIGH ) )
					mDisplayPortAUXSimulationData.Transition();
				else if( ( bit == 1 ) && ( start_bit_state == BIT_LOW ) )
					mDisplayPortAUXSimulationData.Transition(); 
			}
				mDisplayPortAUXSimulationData.Advance( U32(mT) );
				mDisplayPortAUXSimulationData.Transition();
				mDisplayPortAUXSimulationData.Advance( U32(mT) );
		}
		break;
	case FAUX:
		{
		
		}
		break;
	
	}
}
