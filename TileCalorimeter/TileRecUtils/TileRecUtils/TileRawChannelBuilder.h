/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_ITILERAWCHANNELBUILDER_H
#define TILERECUTILS_ITILERAWCHANNELBUILDER_H

/********************************************************************
 *
 * NAME:     TileRawChanelBuilder
 * PACKAGE:  offline/TileCalorimeter/TileRecUtils
 *
 * AUTHOR :  K. Gellerstedt
 * CREATED:  Nov 17 2003
 *
 * PURPOSE:  Interface for builder algorithms
 *
 *  Input: TileDigitsContainer
 *  Output: Container with TileRawChannels
 *  Parameters:
 *    TileRawChannelContainer - Name of output container
 *   
 ********************************************************************/

// Tile includes
#include "TileIdentifier/TileFragHash.h"
#include "TileIdentifier/TileRawChannelUnit.h"
#include "TileEvent/TileMutableRawChannelContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileDigitsCollection.h"
#include "TileEvent/TileDQstatus.h"
#include "TileRecUtils/ITileRawChannelTool.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileConditions/TileCondToolTiming.h"
#include "TileConditions/TileCondIdTransforms.h"
#include "TileConditions/TileCablingSvc.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"

//Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/WriteHandleKey.h"

#include <utility>
#include <vector>
#include <memory>

class TileDigits;
class TileRawChannel;
class TileID;
class TileHWID;
class TileInfo;
class StoreGateSvc;

typedef std::vector<std::pair<TileRawChannel*, const TileDigits*> > Overflows_t;

class TileRawChannelBuilder: public AthAlgTool {
  public:
    TileRawChannelBuilder(const std::string& type, const std::string& name,
        const IInterface* parent);

    virtual ~TileRawChannelBuilder();

    // Algorithm tool virtual methods
    virtual StatusCode initialize();
    //virtual StatusCode execute();
    virtual StatusCode finalize();

    /**
     * Create container in SG with name given by
     * parameter (m_rawChannelContainerKey)
     */
    virtual StatusCode createContainer(const EventContext& ctx);

    /**
     * Builder virtual method to be implemented by subclasses
     * @param digits Pointer to TileDigitsContainer
     *
     */
    virtual TileRawChannel* rawChannel(const TileDigits* digits, const EventContext& ctx);

    /**
     * Commit RawChannelContiner in SG and make const
     */
    virtual StatusCode commitContainer(const EventContext& ctx);

    void initLog(const EventContext& ctx);
    void endLog();

    // process all digits from one collection and store results in internal container
    StatusCode build (const TileDigitsCollection* collection, const EventContext& ctx);

   /**
     * AlgTool InterfaceID
     */
    static const InterfaceID& interfaceID();

    static double correctAmp(double phase, bool of2 = true); //!< Amplitude correction factor according to the time when using weights for tau=0 without iterations

    static double correctTime( double phase, bool of2 = true); //!< Time correction factor

    static int CorruptedData(int ros, int drawer, int channel, int gain,
			     const std::vector<float> & digits, float &dmin, float &dmax, float ADCmaxMinusEps, float ADCmaskValueMinusEps);

    static const char* BadPatternName(float ped);

    void resetDrawer();
    void resetOverflows(void);
    Overflows_t& getOverflowedChannels(void);
    std::string getTileRawChannelContainerID(void);

  protected:
    // FIXME: Get rid of this abomination.
    friend class TileHid2RESrcID;

    // properties
    // DQ status.
    SG::ReadHandleKey<TileDQstatus> m_DQstatusKey{this, "TileDQstatus", 
                                                  "TileDQstatus", 
                                                  "TileDQstatus key"};

    // DSP container.  Only used of m_useDSP is set.
    SG::ReadHandleKey<TileRawChannelContainer> m_DSPContainerKey
      {this, "DSPContainer", "",  "DSP Container key"};

  // name of TDS container with output TileRawChannels
    SG::WriteHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,"TileRawChannelContainer","TileRawChannelFiltered",
                                                                         "Output Tile raw channels container key"};

    // RawChannelContainer
    std::unique_ptr<TileMutableRawChannelContainer> m_rawChannelCnt;

    // parameters for RawChannelContainer
    TileFragHash::TYPE m_rChType{TileFragHash::Default};
    TileRawChannelUnit::UNIT m_rChUnit{TileRawChannelUnit::ADCcounts};
    unsigned int m_bsflags{0};
    // parameter to determine which sample to start with the analysis
    Gaudi::Property<int> m_firstSample{this, "FirstSample", 0, "First sample to start with the analysis"};
    // Should energy be calibrated
    Gaudi::Property<bool> m_calibrateEnergy{this, "calibrateEnergy", false, "Calibrate energy"};

    // Should time be calibrated (delta added from laser run)
    Gaudi::Property<bool> m_correctTime{this, "correctTime", false, "Correct time"};


    // Use DSP noise correction for incomplete containers
    Gaudi::Property<bool> m_useDSP{this, "UseDSPCorrection", true, "Use DSP noise correction for incomplete containers"};

    // thresholds for parabolic amplitude correction (used in optimal filter without iterations)
    Gaudi::Property<float> m_ampMinThresh{this, "AmpMinForAmpCorrection", 15.0,
        "Correct amplitude if it's above amplitude threshold (in ADC counts)"};
    Gaudi::Property<float> m_timeMinThresh{this, "TimeMinForAmpCorrection", -12.5,
        "Correct amplitude if time is above time min threshold"};
    Gaudi::Property<float> m_timeMaxThresh{this, "TimeMaxForAmpCorrection", 12.5,
        "Correct amplitude if time is below time max threshold"};

    // runType used to override value of trigType in event
    Gaudi::Property<int> m_runType{this, "RunType", 0, "Run type used to override value of trigType in event"};

    // Tile objects
    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};

    ToolHandleArray<ITileRawChannelTool> m_noiseFilterTools{this,
        "NoiseFilterTools", {}, "Tile noise filter tools"};

    ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,
        "TileCondToolEmscale", "TileCondToolEmscale", "Tile EM scale calibration tool"};

    ToolHandle<TileCondToolTiming> m_tileToolTiming{this,
        "TileCondToolTiming", "TileCondToolTiming", "Tile timing tool"};

    ToolHandle<TileCondIdTransforms> m_tileIdTransforms{this,
        "TileCondIdTransforms", "TileCondIdTransforms",
        "Tile tool to tranlate hardware identifier to the drawerIdx, channel, and adc"};

    /**
     * @brief Name of Tile cabling service
     */
    ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
        "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};

    const TileCablingService* m_cabling{nullptr}; //!< TileCabling instance

    Gaudi::Property<std::vector<int>> m_demoFragIDs{this,
          "DemoFragIDs", {}, "List of Tile frag IDs with new electronics (demonstrator)"};

    int m_trigType{0};
    bool m_idophys{false};   // Phys fitting
    bool m_idolas{false}; // Laser fitting
    bool m_idoped{false}; // Ped run, phys fitting
    bool m_idocis{false}; // CIS fitting
    int m_cischan{-1}; // which channel is fired by CIS
    double m_capdaq{100.}; // Capacitor

    unsigned int m_evtCounter{0};
    unsigned int m_chCounter{0};

    int m_nChL{0};
    int m_nChH{0};
    double m_RChSumL{0.};
    double m_RChSumH{0.};

    Overflows_t m_overflows;
    Gaudi::Property<int> m_dataPoollSize{this, "DataPoolSize", -1, "Data pool size"};

    static const int MAX_CHANNELS = 48;
    static const int MAX_DMUS = 16;
    int m_error[MAX_CHANNELS];
    int m_lastDrawer{-1};
    bool m_badDrawer{false};

    bool m_notUpgradeCabling{true};

    // TileInfo
    Gaudi::Property<std::string> m_infoName{this, "TileInfoName", "TileInfo", "Name of TileInfo object"};
    const TileInfo* m_tileInfo{nullptr};
    int m_i_ADCmax{0};
    float m_f_ADCmax{0.f};
    int m_i_ADCmaxPlus1{0};
    float m_f_ADCmaxPlus1{0.f};
    float m_ADCmaxMinusEps{0.f};
    float m_ADCmaskValueMinusEps{0.f}; //!< indicates channels which were masked in background dataset

private:
    // find all bad patterns in a drawer and fill internal static arrays
    void fill_drawer_errors(const EventContext& ctx,
                            const TileDigitsCollection* collection);

};

#endif
