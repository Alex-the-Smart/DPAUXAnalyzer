#include "DisplayPortAUXAnalyzerResults.h"

#include <AnalyzerHelpers.h>
#include "DisplayPortAUXAnalyzer.h"
#include "DisplayPortAUXAnalyzerSettings.h"
#include <iostream>
#include <sstream>

DisplayPortAUXAnalyzerResults::DisplayPortAUXAnalyzerResults( DisplayPortAUXAnalyzer* analyzer, DisplayPortAUXAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{

}

DisplayPortAUXAnalyzerResults::~DisplayPortAUXAnalyzerResults()
{

}

void DisplayPortAUXAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& /*channel*/, DisplayBase display_base )
{
	Frame frame = GetFrame( frame_index );
	ClearResultStrings();

	char result_str[128];
	char number_str[128];
	switch (frame.mType)
	{
	case AUXSync:
		AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
		sprintf(result_str, "%s SYNCs", number_str);
		AddResultString( "SYNC" );
		AddResultString(result_str);
		AnalyzerHelpers::GetNumberString(frame.mData2, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
		AddResultString(result_str, ", ", number_str, " bps");
		break;
	case AUXStart:
		AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
		AddResultString( "S" );
		AddResultString("S #", number_str);
		AddResultString( "START" );
		AddResultString("START #", number_str);
		break;
	case AUXData:
		AnalyzerHelpers::GetNumberString(frame.mData1, display_base, mSettings->mBitsPerTransfer, number_str, 128);
		AddResultString(number_str);
		break;
	case AUXStop:
		AddResultString( "P" );
		AddResultString( "STOP" );
		break;
	}
}

void DisplayPortAUXAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::stringstream ss;
	void* f = AnalyzerHelpers::StartFile( file );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();
	U64 num_frames = GetNumFrames();
	U32 dump_addr = 0;

	switch (export_type_user_id)
	{
	case DpAuxDMP:
		
		char number_str[128];

		for (U32 i = 0; i < num_frames; i++)
		{
			Frame frame = GetFrame(i);
			
			if (frame.mType == AUXData)
			{
				if ((dump_addr % 16) == 0)	// need to add an address
				{
					sprintf(number_str, "%08X", dump_addr);
					ss << number_str << "-> ";
				}
				else if ((dump_addr % 4) == 0)	// need to add a space
					ss << " ";

				// add data
				sprintf(number_str, "%02X", (U32) frame.mData1);
				ss << number_str;
								
				if ((dump_addr % 16) == 15)  // need to add new line and save
				{
					ss << std::endl;

					AnalyzerHelpers::AppendToFile((U8*)ss.str().c_str(), ss.str().length(), f);
					ss.str(std::string());
				}

				dump_addr++;
			}
			
			if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true)
			{
				AnalyzerHelpers::EndFile(f);
				return;
			}

		}	// for

		if (ss.str().length() != 0)
		{
			ss << std::endl;
			AnalyzerHelpers::AppendToFile((U8*)ss.str().c_str(), ss.str().length(), f);
		}

		break;

	case DpAuxTXT:
		ss << "Time [s]; Data" << std::endl;

		for (U32 i = 0; i < num_frames; i++)
		{
			Frame frame = GetFrame(i);

			//static void GetTimeString( U64 sample, U64 trigger_sample, U32 sample_rate_hz, char* result_string, U32 result_string_max_length );
			char time_str[128];
			AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128);


			ss << time_str << "; ";

			char number_str[128];
			switch (frame.mType)
			{
			case AUXSync:
				AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
				ss << number_str << " SYNCs, ";
				AnalyzerHelpers::GetNumberString(frame.mData2, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
				ss << number_str << " bps";
				break;
			case AUXStart:
				AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
				ss << "START #" << number_str;
				break;
			case AUXData:
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, mSettings->mBitsPerTransfer, number_str, 128);
				ss << number_str;
				break;
			case AUXStop:
				ss << "STOP";
				break;
			}

			ss << std::endl;

			AnalyzerHelpers::AppendToFile((U8*)ss.str().c_str(), ss.str().length(), f);
			ss.str(std::string());

			if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true)
			{
				AnalyzerHelpers::EndFile(f);
				return;
			}
		}
		break;
	}
	
	UpdateExportProgressAndCheckForCancel( num_frames, num_frames );
	AnalyzerHelpers::EndFile( f );
}

void DisplayPortAUXAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base )
{
    ClearTabularText();

	Frame frame = GetFrame(frame_index);

	char result_str[128];
	char number_str[128];
	switch (frame.mType)
	{
	case AUXSync:
		AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
		sprintf(result_str, "%s SYNCs", number_str);
		AnalyzerHelpers::GetNumberString(frame.mData2, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
		AddTabularText(result_str, ", ", number_str, " bps");
		break;
	case AUXStart:
		AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, mSettings->mBitsPerTransfer, number_str, 128);
		AddTabularText("START #", number_str);
		break;
	case AUXData:
		AnalyzerHelpers::GetNumberString(frame.mData1, display_base, mSettings->mBitsPerTransfer, number_str, 128);
		AddTabularText(number_str);
		break;
	case AUXStop:
		AddTabularText("STOP");
		break;
	}
}

void DisplayPortAUXAnalyzerResults::GeneratePacketTabularText( U64 /*packet_id*/, DisplayBase /*display_base*/ )
{

}

void DisplayPortAUXAnalyzerResults::GenerateTransactionTabularText( U64 /*transaction_id*/, DisplayBase /*display_base*/ )
{

}
