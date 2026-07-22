/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_BEAMSPOTPREPARATORALG_H
#define MUONCOMBINEDALGSR4_BEAMSPOTPREPARATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "xAODTracking/VertexContainer.h"
#include "ActsEvent/AuxiliaryMeasurementHandler.h"
#include  "ActsEvent/ContextUtility.h"


namespace MuonCombinedR4 {
    /** @brief Data preparation algorithm to convert the beamspot position 
     *         into an auxiliary measurement which can be used in the combined
     *         muon fit as an extra constaint at the vertex */
    class BeamSpotPreparatorAlg : public AthReentrantAlgorithm{
        public:
            /** @brief copy the constructor from the parent */
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            /** @copydoc AthReentrantAlgorithm::initialize() */
            virtual StatusCode initialize() override final;
            /** @copydoc AthReentrantAlgorithm::execute */
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Handle the beam spot auxiliary measurement */
            ActsTrk::AuxiliaryMeasurementHandler m_beamSpotHandle{this};
            /** @brief Context provider for geometry, magnetic field and calibration contexts */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Data dependency on the vertex container */
            SG::ReadHandleKey<xAOD::VertexContainer> m_vertexKey{this, "VertexContainer", "PrimaryVertices"};
            /** @brief Optional dependency on the beamspot container */
            SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotDataKey", "BeamSpotData"};
            /** @brief Switch to toggle whether the beam spot or the primary vertex will
             *         be used as beamspot measuremenmt */
            Gaudi::Property<bool> m_useBeamSpot{this, "useBeamSpot", true};
            /** @brief  Key under which the auxiliary container will be registered in store gate*/
            Gaudi::Property<std::string> m_writeKey{this, "WriteKey", "BeamSpotMeasurements"};
            /** @brief Extra scal factor on the radial beam spot covariance position */
            Gaudi::Property<double> m_sigmaScaleR{this, "sigmaScaleR", 1.};
            /** @brief Extra scale factor on the longitudinal beam spot covariance position */
            Gaudi::Property<double> m_sigmaScaleZ{this, "sigmaScaleZ", 1.};

    };
}


#endif