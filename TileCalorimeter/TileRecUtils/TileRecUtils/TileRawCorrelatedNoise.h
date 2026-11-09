/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_TILERAWCORRELATEDNOISE_H
#define TILERECUTILS_TILERAWCORRELATEDNOISE_H

/********************************************************************
 *
 * NAME:     TileRawCorrelatedNoise 
 * PACKAGE:  offline/TileCalorimeter/TileRecUtils
 *
 * AUTHOR :  F. Veloso
 * CREATED:  12.07.2010
 *
 ********************************************************************/

#include "TileEvent/TileDigitsContainer.h"
#include "TileConditions/TileCondToolNoiseSample.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

#include <memory>


class TileRawCorrelatedNoise: public AthAlgorithm {

  public:
    // constructor
    TileRawCorrelatedNoise(const std::string& name, ISvcLocator* pSvcLocator);
    // destructor
    virtual ~TileRawCorrelatedNoise();

    virtual StatusCode initialize();
    virtual StatusCode execute(const EventContext& ctx);
    virtual StatusCode finalize();

  private:

    SG::ReadHandleKey<TileDigitsContainer> m_inputDigitsContainerKey{this, "TileDigitsContainer", 
                                                                    "TileDigitsCnt", 
                                                                    "Input Tile digits container key"};

    SG::WriteHandleKey<TileDigitsContainer> m_outputDigitsContainerKey{this, "OutputTileDigitsContainer", 
                                                                      "NewDigitsContainer", 
                                                                      "Output Tile digits container key"};

    // RMS threshold
    Gaudi::Property<float> m_nRMS_threshold{this, "nRMSThreshold", 2., "Threshold in units of sample 3 RMS"};

    // file names
    Gaudi::Property<std::string> m_alphaMatrixFilePrefix{this, "AlphaMatrixFilePrefix", "AlphaMatrix",
        "Prefix of the alpha matrix file names"};
    Gaudi::Property<std::string> m_sample3RMSFilePrefix{this, "Sample3RMSFilePrefix", "RMS",
        "Prefix of the sample 3 RMS file names"};
    Gaudi::Property<std::string> m_meanFilePrefix{this, "MeanFilePrefix", "Mean",
        "Prefix of the mean file names"};

    // matrices
    struct AlphaMatrix {
      float m[4][64][48][48];
    };
    std::unique_ptr<AlphaMatrix> m_alphaMatrix;
    float m_meanSamples[4][64][48][7]{};
    float m_sample3RMS[4][64][48]{};

    Gaudi::Property<bool> m_useMeanFiles{this, "UseMeanFiles", true, "Use mean files"};
    Gaudi::Property<bool> m_pmtOrder{this, "PMTOrder", false,
        "Read matrices in PMT order (but save them in channel order)"};

    ToolHandle<TileCondToolNoiseSample> m_tileToolNoiseSample{this,
        "TileCondToolNoiseSample", "TileCondToolNoiseSample", "Tile sample noise tool"};

};
#endif
