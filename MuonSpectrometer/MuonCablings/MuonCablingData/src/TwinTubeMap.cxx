/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonCablingData/TwinTubeMap.h>

namespace Muon{
    using TubeLayer = HedgehogBoard::TubeLayer;
    TwinTubeMap::TwinTubeMap(const IMuonIdHelperSvc* idHelperSvc):
        AthMessaging{"TwinTubeMap"},
        m_idHelperSvc{idHelperSvc} {
    }
    bool TwinTubeMap::isTwinTubeLayer(const Identifier& channelId) const {
        ATH_MSG_VERBOSE("Check whether "<<m_idHelperSvc->toStringDetEl(channelId)<<" is a twin tube layer");
        const IdentifierHash detHash{m_idHelperSvc->detElementHash(channelId)};
        Storage::const_iterator itr = m_twinTubesPerRE.find(detHash);
        return itr != m_twinTubesPerRE.end();
    }
           
    Identifier TwinTubeMap::twinId(const Identifier& channelId) const {
        ATH_MSG_VERBOSE("Fetch the twin tube id for "<<m_idHelperSvc->toString(channelId));
        const IdentifierHash detHash{m_idHelperSvc->detElementHash(channelId)};
        Storage::const_iterator twinItr = m_twinTubesPerRE.find(detHash);
        if (twinItr != m_twinTubesPerRE.end()) {
            const HedgehogTray& tray{twinItr->second};
            const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
            TubeLayer in{};
            in.tube = idHelper.tube(channelId);
            in.layer = idHelper.tubeLayer(channelId);
            
            const uint8_t boardNum =  ( (in.tube-1) - (in.tube-1) % tray.nTubesPerLay) / tray.nTubesPerLay;
            if (boardNum >= tray.cards.size() || !tray.cards[boardNum]) {
                return channelId;
            }
            const TubeLayer out = tray.cards[boardNum]->twinPair(in);
            if (in != out) {                
                const Identifier twinId = idHelper.channelID(channelId,idHelper.multilayer(channelId),
                                                             out.layer, out.tube + tray.nTubesPerLay*boardNum);
                ATH_MSG_VERBOSE("The matching twin tube is "<<m_idHelperSvc->toString(twinId));
                return twinId; 
            }
        }
        return channelId;
    }
    StatusCode TwinTubeMap::addHedgeHogBoard(const Identifier& detElId, const HedgehogBoardPtr& board, const uint16_t slot) {
        ATH_MSG_DEBUG("Add new hedgehog board for "<<m_idHelperSvc->toStringDetEl(detElId)<<" "<<(*board)
                     <<", slot: "<<static_cast<int>(slot));
        const IdentifierHash detHash{m_idHelperSvc->detElementHash(detElId)};
        HedgehogTray& tray{m_twinTubesPerRE[detHash]};
        /// Check for compability
        if (!tray.nTubesPerLay) {
            const int nTubLayers = m_idHelperSvc->mdtIdHelper().tubeLayerMax(detElId);
            tray.nTubesPerLay = board->numTubesPerLayer();
            if (nTubLayers != board->numTubeLayers()) {
                ATH_MSG_FATAL("The new hedgehog board "<<(*board)<<" does not match in terms of tube layers"
                            <<nTubLayers<<" "<<m_idHelperSvc->toStringDetEl(detElId));
                return StatusCode::FAILURE;
            }
        } else if (tray.nTubesPerLay != board->numTubesPerLayer()){
            ATH_MSG_FATAL("Cannot at new hedgeog board in "<<m_idHelperSvc->toStringDetEl(detElId)
                <<"Number of tubes per layer does not match. "<<static_cast<int>(tray.nTubesPerLay)<<" vs. "
                <<static_cast<int>(board->numTubesPerLayer()));
            return StatusCode::FAILURE;
        }
        
        /// Check the board for consistency
        if (board.use_count() < 3) {
            for (uint8_t lay = 1 ; lay<= board->numTubeLayers(); ++lay) {
                for (uint8_t tube = 1; tube<=board->numTubesPerLayer(); ++tube) {
                    TubeLayer in{};
                    in.layer = lay;
                    in.tube = tube;
                    const TubeLayer out = board->twinPair(in);
                    const TubeLayer twinTwin = board->twinPair(out);
                    if (twinTwin != in) {
                        ATH_MSG_FATAL("Back & forth mapping of "<<std::endl<<(*board)<<std::endl
                                   <<" failed. Started with "<<in<<" via "<<out<<" & ended in "<<twinTwin);
                        return StatusCode::FAILURE;
                    }
                }
            }
        }
        if (tray.cards.size() <= slot) {
            tray.cards.resize(slot +1);
        }
        tray.cards[slot] = board;
        return StatusCode::SUCCESS;
    }
    double TwinTubeMap::hvDelayTime(const Identifier& channelId) const {
        ATH_MSG_VERBOSE("Fetch the hv delay time for "<<m_idHelperSvc->toString(channelId));
        const IdentifierHash detHash{m_idHelperSvc->detElementHash(channelId)};
        Storage::const_iterator twinItr = m_twinTubesPerRE.find(detHash);
        if (twinItr != m_twinTubesPerRE.end()) {
         const HedgehogTray& tray{twinItr->second};
            const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
            const uint8_t tube = idHelper.tube(channelId);
            const uint8_t boardNum =  (tube - tube % tray.nTubesPerLay) / tray.nTubesPerLay;
            if (boardNum < tray.cards.size() && tray.cards[boardNum] && tray.cards[boardNum]->hasHVDelayTime()) {
                ATH_MSG_VERBOSE("Specific delay time set to "<<tray.cards[boardNum]->hvDelayTime());
                return tray.cards[boardNum]->hvDelayTime();
            }
        }
        return m_defaultHVDelay;
    }        
    void TwinTubeMap::setDefaultHVDelay(const double hvDelay) {
        ATH_MSG_DEBUG("Set the default HV delay to "<<hvDelay<<" ns.");
        m_defaultHVDelay = hvDelay;
    }

}

     