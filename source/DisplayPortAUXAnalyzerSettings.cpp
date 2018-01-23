#include "DisplayPortAUXAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>

#pragma warning(disable: 4800) //warning C4800: 'U32' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4996) //warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

DisplayPortAUXAnalyzerSettings::DisplayPortAUXAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mMode( Manchester ),
	mBitRate( 1000000 ),
	mInverted( false ),
	mBitsPerTransfer( 8 ),
	mShiftOrder( AnalyzerEnums::MsbFirst ),
	mBitsToIgnore( 0 ),
	mTolerance( TOL25 ),
	mAbout( 0 )
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "Display Port AUX", "Display Port AUX" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mModeInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mModeInterface->SetTitleAndTooltip( "Mode", "Specify the Display Port AUX Mode" );
	mModeInterface->AddNumber( Manchester, "Manchester", "" );
	mModeInterface->AddNumber( FAUX, "Fast AUX (not supported)", "" );
	mModeInterface->SetNumber( mMode );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );
	mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/s)",  "Specify the bit rate in bits per second." );
	mBitRateInterface->SetMax( 100000000 );
	mBitRateInterface->SetMin( 1 );
	mBitRateInterface->SetInteger( mBitRate );

	mInvertedInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mInvertedInterface->SetTitleAndTooltip( "", "Specify the DisplayPortAUX edge polarity (Normal DisplayPortAUX mode only)" );
	mInvertedInterface->AddNumber( false, "negative edge is binary one", "" );
	mInvertedInterface->SetNumber( mInverted );

	mBitsPerTransferInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mBitsPerTransferInterface->SetTitleAndTooltip( "", "Select the number of bits per frame" ); 
	mBitsPerTransferInterface->AddNumber( 8, "8 Bits per Transfer", "" );
	mBitsPerTransferInterface->SetNumber( mBitsPerTransfer );

	mShiftOrderInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mShiftOrderInterface->SetTitleAndTooltip( "", "Select if the most significant bit or least significant bit is transmitted first" );
	mShiftOrderInterface->AddNumber( AnalyzerEnums::MsbFirst, "Most Significant Bit Sent First", "" );
	mShiftOrderInterface->SetNumber( mShiftOrder );

/*
	mNumBitsIgnoreInterface.reset( new AnalyzerSettingInterfaceInteger() );
	mNumBitsIgnoreInterface->SetTitleAndTooltip( "Preamble bits to ignore",  "Specify the number of preamble bits to ignore." );
	mNumBitsIgnoreInterface->SetMax( 0 );
	mNumBitsIgnoreInterface->SetMin( 0 );
	mNumBitsIgnoreInterface->SetInteger( mBitsToIgnore );
*/
	mToleranceInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mToleranceInterface->SetTitleAndTooltip( "Tolerance", "Specify the Display Port AUX Tolerance as a percentage of period" );
	mToleranceInterface->AddNumber( TOL25, "25% of period (default)", "Maximum allowed tolerance, +- 50% of one half period" );
	mToleranceInterface->AddNumber( TOL5, "5% of period", "Required more than 10x over sampling" );
	mToleranceInterface->AddNumber( TOL05, "0.5% of period", "Requires more than 200x over sampling" );
	mToleranceInterface->SetNumber( mTolerance );

	mAboutInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mAboutInterface->SetTitleAndTooltip("About Ananlyzer", "Here is some info about this analyzer");
	mAboutInterface->AddNumber(0, "DP AUX Analyzer v1.0 '2018", "Display Port AUX Analyzer ver. 1.0 '2018");
	mAboutInterface->AddNumber(1, "Written by Alex Onisimov", "Written by Alex Onisimov");
	mAboutInterface->AddNumber(2, "onisimov@tut.by", "Email onisimov@tut.by");
	mAboutInterface->AddNumber(3, "Contains some code by Saleae", "Some Manchester Analyzer code was used");
	mAboutInterface->SetNumber(mAbout);


	AddInterface( mInputChannelInterface.get() );
	AddInterface( mModeInterface.get() );
	AddInterface( mBitRateInterface.get() );
	AddInterface( mInvertedInterface.get() );
	AddInterface( mBitsPerTransferInterface.get() );
	AddInterface( mShiftOrderInterface.get() );
//	AddInterface( mNumBitsIgnoreInterface.get() );
	AddInterface( mToleranceInterface.get() );
	AddInterface( mAboutInterface.get() );

	AddExportOption(DpAuxDMP, "Export as HEX dump");
	AddExportExtension(DpAuxDMP, "HEX dump", "dmp");

	
	AddExportOption( DpAuxTXT, "Export as text/csv file" );
	AddExportExtension( DpAuxTXT, "text", "txt" );
	AddExportExtension( DpAuxTXT, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "Display Port AUX", false );
}

DisplayPortAUXAnalyzerSettings::~DisplayPortAUXAnalyzerSettings()
{

}

bool DisplayPortAUXAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mMode = DisplayPortAUXMode( U32( mModeInterface->GetNumber() ) );
	mBitRate = mBitRateInterface->GetInteger();
	mInverted = bool( U32( mInvertedInterface->GetNumber() ) );
	mBitsPerTransfer = U32( mBitsPerTransferInterface->GetNumber() );
	mShiftOrder =  AnalyzerEnums::ShiftOrder( U32( mShiftOrderInterface->GetNumber() ) );
//	mBitsToIgnore = mNumBitsIgnoreInterface->GetInteger();
	mTolerance = DisplayPortAUXTolerance( U32( mToleranceInterface->GetNumber() ) );
	mAbout = U32( mAboutInterface->GetNumber() );
	ClearChannels();
	AddChannel( mInputChannel, "Display Port AUX", true );

	return true;
}
void DisplayPortAUXAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	const char* name_string;	//the first thing in the archive is the name of the protocol analyzer that the data belongs to.
	text_archive >> &name_string;
	if( strcmp( name_string, "SaleaeDisplayPortAUXAnalyzer" ) != 0 )
		AnalyzerHelpers::Assert( "SaleaeDisplayPortAUXAnalyzer: Provided with a settings string that doesn't belong to us;" );

	text_archive >> mInputChannel;
	text_archive >> *(U32*)&mMode;
	text_archive >> mBitRate;
	text_archive >> mInverted;
	text_archive >> mBitsPerTransfer;
	text_archive >> *(U32*)&mShiftOrder;
//	text_archive >> mBitsToIgnore;

	DisplayPortAUXTolerance tolerance;
	if( text_archive >> *(U32*)&tolerance )
		mTolerance = tolerance;

	text_archive >> mAbout;

	ClearChannels();
	AddChannel( mInputChannel, "Display Port AUX", true );

	UpdateInterfacesFromSettings();
}

const char* DisplayPortAUXAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << "SaleaeDisplayPortAUXAnalyzer";
	text_archive << mInputChannel;
	text_archive << U32( mMode );
	text_archive << mBitRate;
	text_archive << mInverted;
	text_archive << mBitsPerTransfer;
	text_archive << U32( mShiftOrder );
//	text_archive << mBitsToIgnore;
	text_archive << U32( mTolerance );
	text_archive << mAbout;

	return SetReturnString( text_archive.GetString() );
}

void DisplayPortAUXAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mModeInterface->SetNumber( mMode );
	mBitRateInterface->SetInteger( mBitRate );
	mInvertedInterface->SetNumber( mInverted );
	mBitsPerTransferInterface->SetNumber( mBitsPerTransfer );
	mShiftOrderInterface->SetNumber( mShiftOrder );
//	mNumBitsIgnoreInterface->SetInteger( mBitsToIgnore );
	mToleranceInterface->SetNumber( mTolerance );
	mAboutInterface->SetNumber(mAbout);
}
