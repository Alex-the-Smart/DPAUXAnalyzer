#pragma warning(push, 0)
#include <sstream>
#include <ios>
#include <algorithm>
#pragma warning(pop)

#include "DisplayPortAUXAnalyzer.h"
#include "DisplayPortAUXAnalyzerSettings.h"  
#include <AnalyzerChannelData.h>


DisplayPortAUXAnalyzer::DisplayPortAUXAnalyzer()
:	mSettings( new DisplayPortAUXAnalyzerSettings() ),
	Analyzer2(),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

DisplayPortAUXAnalyzer::~DisplayPortAUXAnalyzer()
{
	KillThread();
}

void DisplayPortAUXAnalyzer::SetupResults()
{
	mResults.reset( new DisplayPortAUXAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void DisplayPortAUXAnalyzer::WorkerThread()
{
	mDisplayPortAUX = GetAnalyzerChannelData( mSettings->mInputChannel );

	if (mSettings->mSyncBitsNum == 0)	// if settings was stored with previous library version, which have no such parameter
		mSettings->mSyncBitsNum = 16;	// set to default value

	mSampleRateHz = this->GetSampleRate();

	double half_peroid = 1.0 / double( mSettings->mBitRate * 2 );	// Calculate half period in seconds
	half_peroid *= 1000000.0;										// Convert to microseconds
	mT = U32( ( mSampleRateHz * half_peroid ) / 1000000.0 );		// Convert to sample count
	switch( mSettings->mTolerance )
	{
	case TOL25:
		mTError = mT / 2;
		break;
	case TOL5:
		mTError = mT / 10;
		break;
	case TOL05:
		mTError = mT / 100;
		break;
	}
	if( mTError < 3 )
		mTError = 3;
	//mTError = mT / 2;
	mSynchronized = false;

	mDisplayPortAUX->AdvanceToNextEdge();
//	mBitsForNextByte.clear();
//	mUnsyncedLocations.clear();
//	mIgnoreBitCount = mSettings->mBitsToIgnore;

	U64 packet_num = 0;

	U64 edge_location;
	U64 next_edge_location;
	U64 edge_distance;

	Frame frame;

	for( ; ; )
	{
		// Look for valid SYNC sequence
		U32 sync_count = 0;
		frame.mStartingSampleInclusive = mDisplayPortAUX->GetSampleNumber();
		while (mSynchronized == false)
		{
			CheckIfThreadShouldExit();
			edge_location = mDisplayPortAUX->GetSampleNumber();
			mDisplayPortAUX->AdvanceToNextEdge();
			next_edge_location = mDisplayPortAUX->GetSampleNumber();
			edge_distance = next_edge_location - edge_location;
			if ((edge_distance > (mT - mTError)) && (edge_distance < (mT + mTError))) // short = consecutive equal bits (assuming 0s)
			{
				if (sync_count == 0)
					frame.mStartingSampleInclusive = edge_location;
				sync_count++;	// counting short periods
			}
			else if ((edge_distance > ((5 * mT) - mTError)) && (edge_distance < ((5 * mT) + mTError)) && (sync_count>=(2*mSettings->mSyncBitsNum))) // long = possible START symbol
			{
				BitState current_bit_state = mDisplayPortAUX->GetBitState();
				frame.mEndingSampleInclusive = edge_location + mT;
				if (current_bit_state == BIT_LOW)
				{
					edge_location = mDisplayPortAUX->GetSampleNumber();
					mDisplayPortAUX->AdvanceToNextEdge();
					next_edge_location = mDisplayPortAUX->GetSampleNumber();
					edge_distance = next_edge_location - edge_location;
					if ((edge_distance > ((5 * mT) - mTError)) && (edge_distance < ((5 * mT) + mTError))) // long = START symbol, next data is 0.
					{
						mSynchronized = true;
						mResults->AddMarker(next_edge_location-mT, AnalyzerResults::Start, mSettings->mInputChannel);

						// report SYNC frame
						frame.mData1 = sync_count / 2;
						frame.mData2 = ( mSampleRateHz * sync_count / 2 ) / (frame.mEndingSampleInclusive - frame.mStartingSampleInclusive - mT);
						frame.mType = AUXSync;
						frame.mFlags = 0;
						mResults->AddFrame(frame);

						// report START symbol
						frame.mStartingSampleInclusive = frame.mEndingSampleInclusive + 1;
						frame.mEndingSampleInclusive = next_edge_location - mT;
						frame.mData1 = ++packet_num;
						frame.mType = AUXStart;
						frame.mFlags = 0;
						mResults->AddFrame(frame);

						mResults->CommitResults();
						ReportProgress(frame.mEndingSampleInclusive);
					}
					else if((edge_distance >((4 * mT) - mTError)) && (edge_distance < ((4 * mT) + mTError))) // long = START symbol, next data is 1.
					{
						mSynchronized = true;
						mResults->AddMarker(next_edge_location, AnalyzerResults::Start, mSettings->mInputChannel);
						
						// report SYNC frame
						frame.mData1 = sync_count / 2;
						frame.mData2 = (mSampleRateHz * sync_count / 2) / (frame.mEndingSampleInclusive - frame.mStartingSampleInclusive - mT);
						frame.mType = AUXSync;
						frame.mFlags = 0;
						mResults->AddFrame(frame);

						// report START symbol
						frame.mStartingSampleInclusive = frame.mEndingSampleInclusive + 1;
						frame.mEndingSampleInclusive = next_edge_location;
						frame.mData1 = ++packet_num;
						frame.mType = AUXStart;
						frame.mFlags = 0;
						mResults->AddFrame(frame);

						mResults->CommitResults();
						ReportProgress(frame.mEndingSampleInclusive);

						// check and skip 1st half-period of data bit
						edge_location = mDisplayPortAUX->GetSampleNumber();
						mDisplayPortAUX->AdvanceToNextEdge();
						next_edge_location = mDisplayPortAUX->GetSampleNumber();
						edge_distance = next_edge_location - edge_location;
						if (!((edge_distance > (mT - mTError)) && (edge_distance < (mT + mTError)))) // if not short, next bit is invalid
						{
							mSynchronized = false;
							mResults->AddMarker(next_edge_location, AnalyzerResults::ErrorDot, mSettings->mInputChannel);
						}
					}
					else
					{
						sync_count = 0;	// invalid START
					}
				}
				else
				{
					sync_count = 0;	// invalid START
				}

			}
			else
			{
				sync_count = 0; // long, but not START symbol; reset counter
			}
		}

		// Collect data
		frame.mEndingSampleInclusive = mDisplayPortAUX->GetSampleNumber() - mT;	// preparing frame margin in advance
		while (mSynchronized == true)
		{
			CheckIfThreadShouldExit();
			// Get data byte
			U32 value = 0;
			frame.mStartingSampleInclusive = frame.mEndingSampleInclusive + 1;
			for (U32 i = 0; i < 8; ++i)	// Collect 8 bit data
			{
				// Collecting current bit
				BitState current_bit_state = mDisplayPortAUX->GetBitState();
				edge_location = mDisplayPortAUX->GetSampleNumber();
				value <<= 1;
				if ((mSettings->mInverted == false) && (current_bit_state == BIT_LOW))	//neg edge, one
				{
					value |= 1;
					mResults->AddMarker(edge_location, AnalyzerResults::One, mSettings->mInputChannel);
				}
				else if ((mSettings->mInverted == true) && (current_bit_state == BIT_HIGH))	//pos edge, inverted, one
				{
					value |= 1;
					mResults->AddMarker(edge_location, AnalyzerResults::One, mSettings->mInputChannel);
				}
				else
					mResults->AddMarker(edge_location, AnalyzerResults::Zero, mSettings->mInputChannel);	// another cases represents zero

				if (i < 7)	// need advance for first 7 bits only
				{
					mDisplayPortAUX->AdvanceToNextEdge();
					next_edge_location = mDisplayPortAUX->GetSampleNumber();
					edge_distance = next_edge_location - edge_location;

					if ((edge_distance > (mT - mTError)) && (edge_distance < (mT + mTError)))	// consecutive equal bits, need advance to next edge
					{
						edge_location = mDisplayPortAUX->GetSampleNumber();
						mDisplayPortAUX->AdvanceToNextEdge();
						next_edge_location = mDisplayPortAUX->GetSampleNumber();
						edge_distance = next_edge_location - edge_location;
						if (!((edge_distance > (mT - mTError)) && (edge_distance < (mT + mTError))))	// wrong interval
						{
							mResults->AddMarker(next_edge_location, AnalyzerResults::ErrorDot, mSettings->mInputChannel);	//ErrorDot
							mSynchronized = false;
							break;
						}
					}
					else if (!((edge_distance > ((2 * mT) - mTError)) && (edge_distance < ((2 * mT) + mTError))))	// wrong interval
					{
						mResults->AddMarker(next_edge_location, AnalyzerResults::ErrorDot, mSettings->mInputChannel);	//ErrorDot
						mSynchronized = false;
						break;
					}
				}
			}	// Collect 8 bit data
			
			if (mSynchronized == true)	// if valid byte collected
			{
				frame.mEndingSampleInclusive = mDisplayPortAUX->GetSampleNumber() + mT;

				frame.mData1 = value;
				//frame.mData2 = some_more_data_we_collected;
				frame.mType = AUXData;
				frame.mFlags = 0;

	/*			if (such_and_such_error == true)
					frame.mFlags |= SUCH_AND_SUCH_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;
				if (such_and_such_warning == true)
					frame.mFlags |= SUCH_AND_SUCH_WARNING_FLAG | DISPLAY_AS_WARNING_FLAG;
	*/
				mResults->AddFrame(frame);
				mResults->CommitResults();
				ReportProgress(frame.mEndingSampleInclusive);

				// check for potential STOP symbol


				mDisplayPortAUX->AdvanceToNextEdge();
				next_edge_location = mDisplayPortAUX->GetSampleNumber();
				edge_distance = next_edge_location - edge_location;

				if ((edge_distance > (mT - mTError)) && (edge_distance < (mT + mTError)))	// consecutive equal bits, need advance to next edge
				{
					edge_location = mDisplayPortAUX->GetSampleNumber();
					mDisplayPortAUX->AdvanceToNextEdge();
					next_edge_location = mDisplayPortAUX->GetSampleNumber();
					edge_distance = next_edge_location - edge_location;
					if (!((edge_distance >(mT - mTError)) && (edge_distance < (mT + mTError))))	// not a data bit interval
					{
						if ((edge_distance > ((4 * mT) - mTError)) && (edge_distance < ((4 * mT) + mTError))) // potential STOP. TO DO: Check for bit state
						{
							//check here. it might no transition at the end
							mSynchronized = false;
							if (!mDisplayPortAUX->WouldAdvancingCauseTransition(4 * mT - mTError))	// check 2nd half of STOP symbol
							{
								frame.mStartingSampleInclusive = frame.mEndingSampleInclusive + 1;
								frame.mEndingSampleInclusive = next_edge_location + 4 * mT;
								frame.mType = AUXStop;
								mResults->AddMarker(frame.mStartingSampleInclusive, AnalyzerResults::Stop, mSettings->mInputChannel);
								mResults->AddFrame(frame);

							}
							else // STOP error
							{
								mResults->AddMarker(next_edge_location, AnalyzerResults::ErrorDot, mSettings->mInputChannel);
							}
						}
						else
						{
							mResults->AddMarker(next_edge_location, AnalyzerResults::ErrorDot, mSettings->mInputChannel);	//ErrorDot
							mSynchronized = false;
						}
						
					}
				}
				else if (!((edge_distance >((2 * mT) - mTError)) && (edge_distance < ((2 * mT) + mTError))))	// not a data bit interval
				{
					if ((edge_distance >((5 * mT) - mTError)) && (edge_distance < ((5 * mT) + mTError))) // potential STOP. TO DO: Check for bit state
					{
						//check here. it might no transition at the end
						mSynchronized = false;
						if (!mDisplayPortAUX->WouldAdvancingCauseTransition(4 * mT - mTError))	// check 2nd half of STOP symbol
						{
							frame.mStartingSampleInclusive = frame.mEndingSampleInclusive + 1;
							frame.mEndingSampleInclusive = next_edge_location + 4 * mT;
							frame.mType = AUXStop;
							mResults->AddMarker(frame.mStartingSampleInclusive, AnalyzerResults::Stop, mSettings->mInputChannel);
							mResults->AddFrame(frame);

						}
						else // STOP error
						{
							mResults->AddMarker(next_edge_location, AnalyzerResults::ErrorDot, mSettings->mInputChannel);
						}
					}
					else
					{
						mResults->AddMarker(next_edge_location, AnalyzerResults::ErrorDot, mSettings->mInputChannel);	//ErrorDot
						mSynchronized = false;
					}
				}

				mResults->CommitResults();
				ReportProgress(frame.mEndingSampleInclusive);

			}
		}

		CheckIfThreadShouldExit();

	}
}


U32 DisplayPortAUXAnalyzer::GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( newest_sample_requested, sample_rate, simulation_channels );
}

U32 DisplayPortAUXAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 8;
}

bool DisplayPortAUXAnalyzer::NeedsRerun()
{
	return false;
}

const char gAnalyzerName[] = "Display Port AUX";  //your analyzer must have a unique name

const char* DisplayPortAUXAnalyzer::GetAnalyzerName() const
{
	return gAnalyzerName;
}

const char* GetAnalyzerName()
{
	return gAnalyzerName;
}

Analyzer* CreateAnalyzer()
{
	return new DisplayPortAUXAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
