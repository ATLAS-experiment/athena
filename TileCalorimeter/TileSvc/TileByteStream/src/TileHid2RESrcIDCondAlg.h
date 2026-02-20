/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILEHID2RESRCIDCONDALG_H
#define TILEBYTESTREAM_TILEHID2RESRCIDCONDALG_H

#include "TileByteStream/TileHid2RESrcID.h"
#include "TileCalibBlobObjs/TileCalibDrawerInt.h"
#include "TileConditions/ITileCondProxy.h"

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

class TileHWID;

/**
 * @class TileHid2RESrcIDCondAlg
 * @brief Condition algorithm to prepare TileHid2RESrcID conditions object and put it into conditions store
 */
class TileHid2RESrcIDCondAlg: public AthCondAlgorithm {
  public:

    using AthCondAlgorithm::AthCondAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:

    Gaudi::Property<bool> m_forHLT{this, "ForHLT", false, "Produce TileHid2RESrcID conditions object for HLT"};

  /**
   * @brief Name of output TileHid2RESrcID
   */
    SG::WriteCondHandleKey<TileHid2RESrcID> m_hid2ReSrcIdKey{this,
        "TileHid2RESrcID", "TileHid2RESrcID", "Output TileHid2RESrcID conditions object"};

    Gaudi::Property<unsigned int> m_fullTileRODs{this, "FullTileMode", 320000,
        "Run from which to take the cabling (for the moment, either 320000 - full 2017 mode (default) - or 0 - 2016 mode)"};

    Gaudi::Property<std::vector<std::string> > m_ROD2ROBmap{this,
        "ROD2ROBmap",{},"","OrderedSet<std::string>"};

    /**
     * @brief Name of ROB data provider service
     */
    ServiceHandle<IROBDataProviderSvc> m_robSvc{this,
        "ROBDataProviderSvc", "ROBDataProviderSvc", "The ROB data provider service"};

    /**
     * @brief Tool to get Tile ROD status (e.g.: mapping from BS frag ID to drawer ID)
     */
    ToolHandle<ITileCondProxy<TileCalibDrawerInt> > m_rodStatusProxy{this,
       "RODStatusProxy", "", "Tile ROD status proxy tool"};

    const TileHWID* m_tileHWID{nullptr};
    bool m_initFromEvent{false};
    bool m_initFromDB{false};
};


#endif // TILEBYTESTREAM_TILEHID2RESRCIDCONDALG_H
