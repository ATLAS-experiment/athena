/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_TILERAWCHANNELNNMAKER_H
#define TILERECUTILS_TILERAWCHANNELNNMAKER_H
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileNNEmulator.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include <atomic>

class TileHWID;

/**
 *  @class TileRawChannelNNMaker
 *  @brief Reconstructs Tile raw channel amplitudes with the neural network of the
 *         HL-LHC TilePPr firmware, emulated bit-exactly
 */
class TileRawChannelNNMaker: public AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~TileRawChannelNNMaker() override = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

  private:

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
        "TileDigitsContainer", "TileDigitsCnt", "Input Tile digits container key"};

    SG::WriteHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,
        "TileRawChannelContainer", "TileRawChannelNN", "Output Tile raw channels container key"};

    Gaudi::Property<std::string> m_weightsFile{this,
        "WeightsFile", "TileRecUtils/TileRawChannelNN_w9_v1.json",
        "PathResolver location of the network weights JSON"};

    const TileHWID* m_tileHWID{nullptr};
    TileNNEmulator m_nn;
    double m_ampPerCode{0.0};
    mutable std::atomic<unsigned int> m_nMissingGain{0};
    mutable std::atomic<unsigned int> m_nBadNSamples{0};
};

#endif // TILERECUTILS_TILERAWCHANNELNNMAKER_H
