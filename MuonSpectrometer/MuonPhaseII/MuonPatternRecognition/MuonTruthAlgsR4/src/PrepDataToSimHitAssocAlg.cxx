/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PrepDataToSimHitAssocAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "xAODMuonPrepData/sTgcStripCluster.h"
#include "xAODMuonPrepData/sTgcPadHit.h"
#include "xAODMuonPrepData/sTgcWireHit.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"

#include "xAODMuonViews/ChamberViewer.h"

#include "Acts/Utilities/Helpers.hpp"
#include <span>
#include <cassert>
namespace{
    /** @brief phi channel decorator of the rpcs && tgcs  */
    using ChVec_t = std::vector<std::uint16_t>;
    using ChVec_t = std::vector<std::uint16_t>;
    /// @brief Declare the secondary phi and eta channels matched to the SDO
    static const SG::ConstAccessor<ChVec_t> acc_phiChannel{"SDO_phiChannels"};
    static const SG::ConstAccessor<ChVec_t> acc_etaChannel{"SDO_etaChannels"};
    static const SG::ConstAccessor<ChVec_t> acc_padChannel{"SDO_padChannels"};
}

namespace MuonR4{

 
    StatusCode PrepDataToSimHitAssocAlg::initialize() {
        ATH_CHECK(m_simHitsKey.initialize());
        ATH_CHECK(m_prdHitKey.initialize());
        ATH_CHECK(m_decorKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    }
    template <typename PrdType_t>
        const xAOD::MuonSimHit*
            PrepDataToSimHitAssocAlg::truthMatchPrd(Viewer_t<xAOD::MuonSimHitContainer>& simHits,
                                                    const xAOD::MuonMeasurement* measurement) const {

        const auto* prd = dynamic_cast<const  PrdType_t*>(measurement);
        // const MuonGMR4::MuonReadoutElement* re = prd->readoutElement();
        for (const xAOD::MuonSimHit* hit : simHits) {
            /// Match via the identifier
            const Identifier hitId = hit->identify();
            if (hitId == prd->identify()) {
                return hit;
            }
            if constexpr(!std::is_same_v<PrdType_t, xAOD::MdtDriftCircle>) {
                if (prd->readoutElement()->layerHash(hitId) != prd->layerHash()) {
                    continue;
                }
            }
            if constexpr(std::is_same_v<PrdType_t, xAOD::TgcStrip> ||
                         std::is_same_v<PrdType_t, xAOD::RpcMeasurement>) {
                if (( prd->measuresPhi() && Acts::rangeContainsValue(acc_phiChannel(*hit), prd->channelNumber())) ||
                    (!prd->measuresPhi() && Acts::rangeContainsValue(acc_etaChannel(*hit), prd->channelNumber()))) {
                    return hit;
                }
            } else if constexpr (std::is_same_v<PrdType_t, xAOD::MMCluster> ||
                                 std::is_same_v<PrdType_t, xAOD::sTgcStripCluster>) {
                for (auto ch : prd->stripNumbers()) {
                    if (Acts::rangeContainsValue(acc_etaChannel(*hit), ch)) {
                        return hit;
                    }
                }
            } else if constexpr(std::is_same_v<PrdType_t, xAOD::sTgcWireHit>) {
                if (Acts::rangeContainsValue(acc_phiChannel(*hit), prd->channelNumber())){
                    return hit;
                }
            } else if constexpr(std::is_same_v<PrdType_t, xAOD::sTgcPadHit>) {
                if (Acts::rangeContainsValue(acc_padChannel(*hit), prd->channelNumber())){
                    return hit;
                }
            }
        }
        return nullptr;
    }
    
    StatusCode PrepDataToSimHitAssocAlg::execute(const EventContext & ctx) const {
        const xAOD::MuonSimHitContainer* simHits{nullptr};
        const xAOD::MuonMeasurementContainer* measurements{nullptr};
        ATH_CHECK(SG::get(simHits, m_simHitsKey, ctx));
        ATH_CHECK(SG::get(measurements, m_prdHitKey, ctx));

        if (measurements->empty()){
            return StatusCode::SUCCESS;
        }

        xAOD::ChamberViewer prdViewer{*measurements};
        xAOD::ChamberViewer simHitViewer{*simHits, m_idHelperSvc.get(), xAOD::ChamberView::Mode::DetElement};
        SG::WriteDecorHandle<xAOD::MuonMeasurementContainer, LinkType> decorHandle{m_decorKey, ctx};
        /** Loop over the measurements */
        do {
            const Identifier chambId = prdViewer.at(0)->identify();
            const IdentifierHash viewHash = m_idHelperSvc->detElementHash(chambId);
            /** Setup a default empty link */
            decorHandle(*prdViewer.at(0)) = LinkType{};

            ///
            if ((simHitViewer.size() == 0 || m_idHelperSvc->detElementHash(simHitViewer.at(0)->identify()) > viewHash)  && 
                 !simHitViewer.loadView(chambId)) {
                ATH_MSG_DEBUG("No simHit view for " << m_idHelperSvc->toStringDetEl(chambId));
                continue;
            } else if (m_idHelperSvc->detElementHash(simHitViewer.at(0)->identify())  < viewHash \
                      && !simHitViewer.next([viewHash, this](const xAOD::MuonSimHit* hit){
                        return m_idHelperSvc->detElementHash(hit->identify()) == viewHash;
                      })) {
                continue;
            }
            ATH_MSG_VERBOSE("Container size "<<simHits->size()<<" viewer size: "<<simHitViewer.size()
                            <<" view hash: "<<viewHash);
            for (const xAOD::MuonMeasurement* measurement : prdViewer) {
                /** Define the place holder for the closest simHit */
                const xAOD::MuonSimHit* bestSimHit{nullptr};
                switch (measurement->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType:{
                        bestSimHit = truthMatchPrd<xAOD::MdtDriftCircle>(simHitViewer, measurement);
                        break;
                    } case xAOD::UncalibMeasType::RpcStripType: {
                        bestSimHit = truthMatchPrd<xAOD::RpcMeasurement>(simHitViewer, measurement);
                        break;
                    } case xAOD::UncalibMeasType::TgcStripType: {
                        bestSimHit = truthMatchPrd<xAOD::TgcStrip>(simHitViewer, measurement);
                        break;
                    } case xAOD::UncalibMeasType::MMClusterType: {
                        bestSimHit = truthMatchPrd<xAOD::MMCluster>(simHitViewer, measurement);
                        break;
                    } case xAOD::UncalibMeasType::sTgcStripType:{
                        const auto* prd = static_cast<const xAOD::sTgcMeasurement*>(measurement);
                        switch (prd->channelType()) {
                            using enum sTgcIdHelper::sTgcChannelTypes;
                            case Strip:{
                                bestSimHit = truthMatchPrd<xAOD::sTgcStripCluster>(simHitViewer, measurement);
                                break;
                            }
                            case Wire:{
                                bestSimHit = truthMatchPrd<xAOD::sTgcWireHit>(simHitViewer, measurement);
                                break;
                            }
                            case Pad:{
                                bestSimHit = truthMatchPrd<xAOD::sTgcPadHit>(simHitViewer, measurement);
                                break;
                            }
                        }
                        break;
                    } default: {
                        ATH_MSG_FATAL("Non muon measurement is parsed");
                        return StatusCode::FAILURE;
                    }
                }
                if (!bestSimHit) {
                    continue;
                }
                
                decorHandle(*measurement) = LinkType{*simHits, bestSimHit->index()};
            }
        } while (prdViewer.next());
        
        return StatusCode::SUCCESS;
    }
}

