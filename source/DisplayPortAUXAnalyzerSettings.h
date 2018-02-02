#ifndef DISPLAYPORTAUX_ANALYZER_SETTINGS
#define DISPLAYPORTAUX_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

enum DisplayPortAUXMode { Manchester, FAUX };
enum DisplayPortAUXTolerance { TOL25, TOL5, TOL05 };
enum DisplayPortAUXExportType { DpAuxDMP, DpAuxTXT };


class DisplayPortAUXAnalyzerSettings : public AnalyzerSettings
{
public:
	DisplayPortAUXAnalyzerSettings();
	virtual ~DisplayPortAUXAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();  //Get the settings out of the interfaces, validate them, and save them to your local settings vars.
	virtual void LoadSettings( const char* settings );  //Load your settings from a string.
	virtual const char* SaveSettings();  //Save your settings to a string.

	void UpdateInterfacesFromSettings();

	Channel mInputChannel;
	DisplayPortAUXMode mMode;
	U32 mBitRate;
	bool mInverted; //non-inverted(false): neg edge = 1; pos edge = 0; inverted(true): neg edge = 0; pos edge = 1;
	U32 mBitsPerTransfer;
	AnalyzerEnums::ShiftOrder mShiftOrder;
	U32 mSyncBitsNum;
	DisplayPortAUXTolerance mTolerance;
	U32 mAbout;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mModeInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger > mBitRateInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mInvertedInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mBitsPerTransferInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mShiftOrderInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger > mSyncBitsNumInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mToleranceInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mAboutInterface;

};
#endif //DISPLAYPORTAUX_ANALYZER SETTINGS
