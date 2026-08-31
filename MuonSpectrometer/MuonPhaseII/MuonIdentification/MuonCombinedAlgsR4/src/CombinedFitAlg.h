/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_COMBINEDFITALG_H
#define MUONCOMBINEDALGSR4_COMBINEDFITALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"


#include "ActsEvent/ContextUtility.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "MuonTrackEvent/MuonTag.h"

#include "xAODMuonViews/FillContainer.h"

namespace MuonCombinedR4 {

     class CombinedFitAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:

            struct DataShip{
                /** @brief The prematched combined muon tags */
                const MuonR4::MuonTagContainer* preMatchedTags{};
                /** @brief The output combined tag collection */
                xAOD::FillContainer<MuonR4::MuonTagContainer, void*> combinedTags{};
                /** @brief The geometry context to align the surfaces during the fit */
                Acts::GeometryContext tgContext{Acts::GeometryContext::dangerouslyDefaultConstruct()};
                /** @brief The magnetic field context */
                Acts::MagneticFieldContext mfContext{};
                /** @brief The calibration context */
                Acts::CalibrationContext calContext{};
            };

            StatusCode prepareDataShip(const EventContext& ctx, DataShip& ship) const;
            /** @brief Context provider for geometry, magnetic field and calibration contexts */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Use STACO as input to the MuidCo chain as the track pair is already done upstream */
            SG::ReadHandleKey<MuonR4::MuonTagContainer> m_stacoKey{this, "stacoKey", "MuonTagsSTACO"};

            /** @brief Write key of the created combined tags */
            SG::WriteHandleKey<MuonR4::MuonTagContainer> m_outTagKey{this, "writeKey" , "MuonTagsMuidCo"};

    };
}


#endif