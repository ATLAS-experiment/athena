/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILETBREC_TILERAWCHANNELGAINFILTER_H
#define TILETBREC_TILERAWCHANNELGAINFILTER_H

#include "TileEvent/TileRawChannelContainer.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

class TileHWID;

/**
 @class TileRawChannelGainFilter
 @brief This algorithm copies TileRawChannel from input container to output container
 */
class TileRawChannelGainFilter: public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~TileRawChannelGainFilter() = default;

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;
    StatusCode finalize() override;

  private:

    SG::ReadHandleKey<TileRawChannelContainer> m_inputContainerKey{this,
        "InputRawChannelContainer", "TileRawChannelCnt", "Input Tile raw channel container key"};

    SG::WriteHandleKey<TileRawChannelContainer> m_outputContainerKey{this,
        "OutputRawChannelContainer", "TileRawChannelFiltered", "Output Tile raw channel container key"};

    const TileHWID* m_tileHWID{nullptr};
    enum TileOverflowPedestal{OVERFLOW_PEDESTAL_MIN = 20000, OVERFLOW_PEDESTAL_MAX = 40000};
};

#endif // TILETBREC_TILERAWCHANNELGAINFILTER_H
