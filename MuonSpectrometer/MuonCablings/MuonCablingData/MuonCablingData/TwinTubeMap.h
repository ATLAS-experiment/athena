/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCABLINGDATA_TWINTUBEMAP_H
#define MUONCABLINGDATA_TWINTUBEMAP_H

#include <AthenaBaseComps/AthMessaging.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonCablingData/HedgehogBoard.h>
#include <unordered_map> 

namespace Muon{
    /** @param The ATLAS R3 MuonSpectrometer has 2 layers with twin tube readout. In Phase II,
     *         other multilayers may be upgraded to be twin-tube layers as well. The TwinTubeMap returns
     *         for a given Mdt Identifier the associated twin-tube Identifier, if there's any at all. */
    class TwinTubeMap : public AthMessaging {
        public:
            using HedgehogBoardPtr = HedgehogBoard::HedgehogBoardPtr;
            
            TwinTubeMap(const IMuonIdHelperSvc* idHelperSvc);
            /** @brief Returns whether the multilayer is equipped with twin-tubes or not 
             *  @param channelId: Identifier of a tube in the multilayer */
            bool isTwinTubeLayer(const Identifier& channelId) const;
            /** @brief Returns the Identifier of the mapped twin tube. 
             *         If the Identifier has no twin, then the Identifier is returned.
             * @param channelId: Identifier of the tube of interest */
            Identifier twinId(const Identifier& channelId) const;
            /** @brief Returns the HV time delay for a given twin tube pair. 
             *         If there's no dedicated delay safed for the pair, the default value is returned */
            double hvDelayTime(const Identifier& channelId) const;
            /** @brief Sets the default HV delay */
            void setDefaultHVDelay(const double hvDelay);
            /** @brief Add a new hedgehog board with twin tube mapping
             *  @param detElId: Identifier of the multilayer
             *  @param board: Pointer to the HedgehogBoard instance encoding the mapping
             *  @param slot: Integer indicating which tube range is covered by the board */
            StatusCode addHedgeHogBoard(const Identifier& detElId, const HedgehogBoardPtr& board, const uint16_t slot);
        private:
            const IMuonIdHelperSvc* m_idHelperSvc{nullptr};

    	    struct HedgehogTray{
                uint8_t nTubesPerLay{0};
                std::vector<HedgehogBoardPtr> cards{};
            };
            using Storage = std::unordered_map<IdentifierHash, HedgehogTray>;
            Storage m_twinTubesPerRE{};

            double m_defaultHVDelay{0.};
    };
}
#include "AthenaKernel/CondCont.h"
CLASS_DEF( Muon::TwinTubeMap , 205882636 , 1 );
CONDCONT_DEF( Muon::TwinTubeMap , 123689198 );

#endif