#ifndef DISPLAYPORTAUX_ANALYZER_RESULTS
#define DISPLAYPORTAUX_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class DisplayPortAUXAnalyzer;
class DisplayPortAUXAnalyzerSettings;

class DisplayPortAUXAnalyzerResults : public AnalyzerResults
{
public:
	DisplayPortAUXAnalyzerResults( DisplayPortAUXAnalyzer* analyzer, DisplayPortAUXAnalyzerSettings* settings );
	virtual ~DisplayPortAUXAnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

protected: //functions

protected:  //vars
	DisplayPortAUXAnalyzerSettings* mSettings;
	DisplayPortAUXAnalyzer* mAnalyzer;
};


#endif //DISPLAYPORTAUX_ANALYZER RESULTS
