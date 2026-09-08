
/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_COMBINEDSTACOALG_H
#define MUONCOMBINEDALGSR4_COMBINEDSTACOALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"


#include "ActsEvent/ContextUtility.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "MuonTrackEvent/MuonTag.h"

namespace MuonCombinedR4 {
    /** @brief Prototype to combine ID + MS tracks using the statistical combination
     *         The ID + MS momentum are combined using the inverse of the respective
     *         q/p covariance values of the two tracks. The energy loss in the calorimeter
     *         is eiher measured from the associated calorimeter clusters or from the 
     *         material description given in the calorimter tracking geometry. */
    class CombinedStacoAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Calculate the energy loss that is added to the MS momentum. First,
             *         it is attempted to use the energy from the associated calo cluster.
             *         If not available the momentum of the last track state is compared 
             *         to the ID pergiee momentum
             * @param idTag: Reference to the inner detector track of interest */
            double calcELoss(const MuonR4::MuonTag& idTag) const;

            /** @brief Context provider for geometry, magnetic field and calibration contexts */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Input key to the reconstructed MS track particles */
            SG::ReadHandleKey<xAOD::TrackParticleContainer> m_msTrackKey{this, "MsTracks", "MsTrackParticlesR4"};
            /** @brief Input key to the selected ID / ITk track particles */
            SG::ReadHandleKey<MuonR4::MuonTagContainer> m_idTrkKey{this, "IdTrkKey", "MuonInDetCandidates"};
            /** @brief Write key of the created STACO tags */
            SG::WriteHandleKey<MuonR4::MuonTagContainer> m_stacoKey{this, "writeKey" , "MuonTagsSTACO"};
            /** @brief Write key of the associated combined track particle container */
            SG::WriteHandleKey<xAOD::TrackParticleContainer> m_cmbTrkKey{this, "writeTrkKey", 
                                                                         "STACOTrackParticles"};
            /** @brief Upper cut on the delta eta between ID and MS track  */
            Gaudi::Property<float> m_match_dTheta{this, "maxDTheta", 2.*Gaudi::Units::deg};
            /** @brief Upper cut on the delat phi between ID and MS track */
            Gaudi::Property<float> m_match_dPhi{this, "maxDPhi", 5.*Gaudi::Units::deg};

            /** @brief Upper cut on the boundary tolerance. Ms tracks that are close 
              *        to the surface bounds are extrapolated to the ID surface if they
              *        differ. */
            Gaudi::Property<float> m_match_boundTol{this, "matchBoundol", 20. * Gaudi::Units::cm};
            /** @brief Switch toggling whether the energy loss from the associated calorimeter
             *         cluster shall be taken */
            Gaudi::Property<bool> m_useMeasELoss{this,"useMeasELoss", true};

            /** @brief Instance to the Acts logger */
            std::unique_ptr<const Acts::Logger> m_logger{};
            /** @brief Return the reference to the Acts logger */
            const Acts::Logger& logger() const { return *m_logger; }

    };
}
#endif
