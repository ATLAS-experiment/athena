/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CombinedStacoAlg.h"

#include "xAODMuonViews/FillContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

#include "ActsEvent/Decoration.h"
#include "ActsEvent/CaloExtension.h"
#include "Acts/Utilities/VectorHelpers.hpp"
#include "FourMomUtils/P4Helpers.h"

namespace {
    constexpr Acts::HashedString caloExitParKey = Acts::hashString("@CaloExit");

    using StacoCont_t = xAOD::FillContainer<MuonR4::MuonTagContainer, void*>;

    using TrackCont_t = xAOD::FillContainer<xAOD::TrackParticleContainer,
                                            xAOD::TrackParticleAuxContainer>;
}

namespace MuonCombinedR4 {
    StatusCode CombinedStacoAlg::initialize() {
        ATH_CHECK(m_msTrackKey.initialize());
        ATH_CHECK(m_idTrkKey.initialize());
        ATH_CHECK(m_stacoKey.initialize());
        ATH_CHECK(m_cmbTrkKey.initialize());
        ATH_CHECK(m_ctxProvider.initialize());
        return StatusCode::SUCCESS;
    }

    double CombinedStacoAlg::calcELoss(const MuonR4::MuonTag& idTag) const {
         const xAOD::TrackParticle* idTrk = idTag.idTrack();
        const ActsTrk::CaloExtension* caloExt = ActsTrk::getCaloExtension(*idTrk);
        double eLoss{0.};
        if (m_useMeasELoss && caloExt) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Attempt to fetch the energy loss from "
                           <<caloExt->associatedClusters().size()<<" associated clusters. ");
            for (const xAOD::CaloCluster* clust : caloExt->associatedClusters()) {
                eLoss += clust->e();
            }
            if (!caloExt->associatedClusters().empty()) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Measured E-loss: "<<eLoss);
                return eLoss;
            }
        }
                   
        auto idActsTrk = ActsTrk::getActsTrack(*idTrk);
        auto caloExitPars = idTag.extrapolatedParsID(caloExitParKey);

        const Acts::BoundTrackParameters idPerigeePars = idActsTrk->createParametersAtReference();
        /// Aproximate the energy loss as the fitted loss between perigee and the calorimeter exit.
        eLoss = idPerigeePars.absoluteMomentum() - caloExitPars->absoluteMomentum();

            
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Track has experienced "<<eLoss<<" [GeV] energy loss. "
                        <<(idPerigeePars.absoluteMomentum() - 
                        ActsTrk::lastTrackParameters(*idTrk)->absoluteMomentum())<<" [GeV] were lost in the calorimeter");
        if(eLoss< 0) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - ID track has negative E-loss "<<eLoss<<" [GeV]");
        }
        return eLoss;
    }
    StatusCode CombinedStacoAlg::execute(const EventContext& ctx) const {
        TrackCont_t stacoTracks{};
        StacoCont_t stacoTags{};

        const MuonR4::MuonTagContainer* idTracks{nullptr};
        const xAOD::TrackParticleContainer *msTracks{nullptr};
        ATH_CHECK(SG::get(idTracks, m_idTrkKey, ctx));
        ATH_CHECK(SG::get(msTracks, m_msTrackKey, ctx));
   
        for (const MuonR4::MuonTag* idTag : *idTracks) {
            auto caloExitPars = idTag->extrapolatedParsID(caloExitParKey);
            if (!caloExitPars) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Track has no calo exit pars");
                continue;
            }
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Calorimeter exit parameters: "<<(*caloExitPars));
            const double eLoss = calcELoss(*idTag);

            const xAOD::TrackParticle* idTrk = idTag->idTrack();

            /// Now loop over the tags to find the matching candidates
            for (const xAOD::TrackParticle* saTag : *msTracks) {
                const auto msTrk = ActsTrk::getActsTrack(*saTag);
                const Acts::BoundTrackParameters msPerigeePars = msTrk->createParametersAtReference();
                /// Basic parameter match
                using namespace P4Helpers;
                if (std::abs(Acts::VectorHelpers::eta(msPerigeePars) -
                             Acts::VectorHelpers::eta(*caloExitPars)) >  m_match_dEta ||
                    std::abs(deltaPhi(msPerigeePars.phi(), caloExitPars->phi())) > m_match_dPhi) {
                    continue;
                }
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Ms track "<<msPerigeePars<<", rel:"<<
                std::sqrt((*msPerigeePars.covariance())(Acts::eBoundQOverP, Acts::eBoundQOverP)) / 
                msPerigeePars.qOverP());
                /// Blindly add the energy loss to the momentum
                const double msQoverP =  std::copysign(1./(msPerigeePars.absoluteMomentum() + eLoss), msPerigeePars.qOverP());
                const double msCov = 1./(*msPerigeePars.covariance())(Acts::eBoundQOverP, Acts::eBoundQOverP);
                const double idCov = 1./(*caloExitPars->covariance())(Acts::eBoundQOverP, Acts::eBoundQOverP);
                const double combCov = msCov + idCov;
                const double combinedQoverP = (caloExitPars->qOverP()*idCov + msQoverP * msCov) / combCov;
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - ID momentum: "<<caloExitPars->absoluteMomentum()
                    <<"("<<(idCov/combCov)<<"), MS momentum: "<< msPerigeePars.absoluteMomentum()<<", E-loss: "<<eLoss<<" --> "
                    <<" MS momentum + ELoss: "<<std::abs(1./msQoverP)<<"("<< (msCov / combCov)<<") ---> Combined Momentum: "
                    <<std::abs(1./combinedQoverP));
                auto combinedTrk = stacoTracks->push_back(std::make_unique<xAOD::TrackParticle>());
                combinedTrk->setDefiningParameters(idTrk->d0(), idTrk->z0(), idTrk->phi0(), 
                                                   idTrk->theta(), combinedQoverP / Gaudi::Units::GeV);

                auto stacoTag  = stacoTags->push_back(std::make_unique<MuonR4::MuonTag>());
                stacoTag->setAuthor(xAOD::Muon::Author::STACO);
                stacoTag->setIdTrack(idTrk);
                stacoTag->setCbTrack(combinedTrk);
                stacoTag->setMsTrack(saTag);
                stacoTag->setSegments(msTrk->component<std::vector<const xAOD::MuonSegment*>>("muonSegLinks"));
            }
        }

        ATH_CHECK(stacoTags.record(m_stacoKey, ctx));
        ATH_CHECK(stacoTracks.record(m_cmbTrkKey, ctx));
        
        return StatusCode::SUCCESS;
    }
}