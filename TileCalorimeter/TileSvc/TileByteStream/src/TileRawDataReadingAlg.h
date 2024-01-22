/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEBYTESTREAM_TILERAWDATAREADINDINGALG_H
#define TILEBYTESTREAM_TILERAWDATAREADINDINGALG_H

#include "TileByteStream/TileROD_Decoder.h"
#include "TileByteStream/TileHid2RESrcID.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileBeamElemContainer.h"
#include "TileEvent/TileLaserObject.h"
#include "TileEvent/TileContainer.h"
#include "TileIdentifier/TileFragHash.h"
#include "TileConditions/TileCablingSvc.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"


/** @class TileRawDataReadingAlg
 *  @brief Class for Tile raw data reading from BS
 */
class TileRawDataReadingAlg : public  AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

  private:

    template <class GetRobOperation, class FillCollOperation>
    StatusCode readDigits(const SG::WriteHandleKey<TileDigitsContainer>& digitsKey,
                          const EventContext& ctx, const TileHid2RESrcID* hid2re,
                          GetRobOperation getRobFromFragID, FillCollOperation fillCollection,
                          unsigned int offsetID=0) const;

    template <class GetRobOperation, class FillCollOperation>
    StatusCode readRawChannels(const SG::WriteHandleKey<TileRawChannelContainer>& rawChannelsKey,
                               const EventContext& ctx, const TileHid2RESrcID* hid2re,
                               GetRobOperation getRobFromFragID, FillCollOperation fillCollection,
                               TileFragHash::TYPE type=TileFragHash::OptFilterDsp) const;

    StatusCode readBeamElements(const SG::WriteHandleKey<TileBeamElemContainer>& beamElementsKey,
                                const EventContext& ctx, const TileHid2RESrcID* hid2re) const;

    StatusCode readLaserObject(const SG::WriteHandleKey<TileLaserObject>& laserObjectKey,
                               const EventContext& ctx, const TileHid2RESrcID* hid2re) const;

    StatusCode readMuonReceiver(const SG::WriteHandleKey<TileMuonReceiverContainer>& muRcvKey,
                                const EventContext& ctx) const;

    StatusCode readL2(const SG::WriteHandleKey<TileL2Container>& l2Key,
                      const EventContext& ctx) const;

    SG::WriteHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
        "TileDigitsContainer", "", "Output Tile digits container key"};

    SG::WriteHandleKey<TileDigitsContainer> m_flxDigitsContainerKey{this,
        "TileDigitsFlxContainer", "", "Output Tile FELIX digits container key"};

    SG::WriteHandleKey<TileDigitsContainer> m_muRcvDigitsContainerKey{this,
        "MuRcvDigitsContainer", "", "Output Tile TMDB digits container key"};

    SG::WriteHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,
        "TileRawChannelContainer", "", "Output Tile raw channels container key"};

    SG::WriteHandleKey<TileRawChannelContainer> m_muRcvRawChannelContainerKey{this,
        "MuRcvRawChannelContainer", "", "Output Tile TMDB raw channels container key"};

    SG::WriteHandleKey<TileBeamElemContainer> m_beamElemContainerKey{this,
        "TileBeamElemContainer", "", "Output Tile beam elements container key"};

    SG::WriteHandleKey<TileLaserObject> m_laserObjectKey{this,
        "TileLaserObject", "", "Output Tile object key"};

    SG::WriteHandleKey<TileMuonReceiverContainer> m_muonReceiverContainerKey{this,
        "TileMuonReceiverContainer", "", "Output Tile muon receiver container key"};

    SG::WriteHandleKey<TileL2Container> m_l2ContainerKey{this,
        "TileL2Container", "", "Output Tile L2 container key"};

    SG::ReadCondHandleKey<TileHid2RESrcID> m_hid2RESrcIDKey{this,
        "TileHid2RESrcID", "TileHid2RESrcID", "TileHid2RESrcID key"};

    /**
     * @brief Name of ROB data provider service
     */
    ServiceHandle<IROBDataProviderSvc> m_robSvc{this,
        "ROBDataProviderSvc", "ROBDataProviderSvc", "The ROB data provider service"};

    ToolHandle<TileROD_Decoder> m_decoder{this,
        "TileROD_Decoder", "TileROD_Decoder", "Tile ROD decoder"};

   /*
    * @brief Name of Tile cabling service
    */
     ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
         "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};

    //Switches set in initialize() based of SG keys of output object
    bool m_doDigits{false};
    bool m_doMuRcvDigits{false};
    bool m_doFlxDigits{false};
    bool m_doRawChannels{false};
    bool m_doMuRcvRawChannels{false};
    bool m_doBeamElements{false};
    bool m_doLaserObject{false};
    bool m_doMuonReceiver{false};
    bool m_doL2{false};
};

#endif
