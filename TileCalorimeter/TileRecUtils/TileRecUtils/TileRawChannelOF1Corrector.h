/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef TILERECUTILS_TILERAWCHANNELOF1CORRECTOR_H
#define TILERECUTILS_TILERAWCHANNELOF1CORRECTOR_H

// Tile includes
#include "TileRecUtils/ITileRawChannelTool.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileConditions/ITileCondToolDspThreshold.h"
#include "TileConditions/TileSampleNoise.h"
#include "TileConditions/TileCondToolOfc.h"
#include "TileConditions/TileCondToolTiming.h"
#include "TileConditions/TileEMScale.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"


// forward declarations
class TileRawChannel;
class TileRawChannelContainer;
class TileRawChannelCollection;
class TileHWID;

/**
 @class TileRawChannelOF1Corrector
 @brief This tool correct TileRawChannels amplitudes which came from OF1 DSP if pedestal changed
 */
class TileRawChannelOF1Corrector: public extends<AthAlgTool, ITileRawChannelTool> {

  public:

    /** AlgTool like constructor */
    TileRawChannelOF1Corrector(const std::string& type, const std::string& name, const IInterface* parent);

    /** Virtual destructor */
    virtual ~TileRawChannelOF1Corrector() {};

    /** AlgTool initialize method.*/
    virtual StatusCode initialize() override;
    /** AlgTool finalize method */
    virtual StatusCode finalize() override;

    /** Correct TileRawChannel amplitudes if pedestal changed */
    virtual StatusCode process (TileMutableRawChannelContainer& rchCont, const EventContext &ctx) const override;


  private:

    const TileHWID* m_tileHWID; //!< Pointer to TileHWID

    /**
     * @brief Name of TileSampleNoise in condition store
     */
    SG::ReadCondHandleKey<TileSampleNoise> m_sampleNoiseKey{this,
        "TileSampleNoise", "TileSampleNoise", "Input Tile sample noise"};
    /**
     * @brief Name of online TileSampleNoise in condition store
     */
    SG::ReadCondHandleKey<TileSampleNoise> m_onlineSampleNoiseKey{this,
        "TileOnlineSampleNoise", "TileOnlineSampleNoise", "Input online Tile sample noise"};

    ToolHandle<ITileCondToolOfc> m_tileCondToolOfc{this,
        "TileCondToolOfc", "TileCondToolOfcCoolOF1", "Tile OFC tool"};

    ToolHandle<TileCondToolTiming> m_tileToolTiming{this,
        "TileCondToolTiming", "TileCondToolOnlineTiming", "Tile timing tool"};

   /**
    * @brief Name of TileEMScale in condition store
    */
    SG::ReadCondHandleKey<TileEMScale> m_emScaleKey{this,
        "TileEMScale", "TileEMScale", "Input Tile EMS calibration constants"};

    ToolHandle<ITileCondToolDspThreshold> m_tileDspThreshold{this,
        "TileCondToolDspThreshold", "TileCondToolDspThreshold", "Tile DSP thresholds tool"};

    
    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
        "TileDigitsContainer", "TileDigitsCnt", "Input Tile digits container key"};


    Gaudi::Property<bool> m_zeroAmplitudeWithoutDigits{this, "ZeroAmplitudeWithoutDigits", true,
        "Set amplitude to zero for channels without digits"};
    Gaudi::Property<bool> m_correctPedestalDifference{this, "CorrectPedestalDifference", true,
        "Correct amplitude for difference between online and offline pedestals"};
};

#endif // TILERECUTILS_TILERAWCHANNELOF1CORRECTOR_H
