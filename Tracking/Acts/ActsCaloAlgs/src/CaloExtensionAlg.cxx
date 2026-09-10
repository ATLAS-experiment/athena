/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#include "CaloExtensionAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "ActsInterop/Logger.h"

#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Tolerance.hpp"

#include "Acts/Propagator/ActorList.hpp"
#include "Acts/Propagator/StandardAborters.hpp"
#include "Acts/Propagator/SurfaceCollector.hpp"
#include "Acts/Propagator/MaterialInteractor.hpp"

#include "Acts/Utilities/AngleHelpers.hpp"

#include "Acts/Utilities/VectorHelpers.hpp"
#include "Acts/Utilities/Result.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Utilities/Logger.hpp"


#include "egammaUtils/CandidateMatchHelpers.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "FourMomUtils/P4Helpers.h"

using namespace Acts::VectorHelpers;
using namespace Acts::UnitLiterals;
using namespace Acts::AngleHelpers;

namespace {
    inline float clusterEta(const xAOD::CaloCluster& cluster) {
        return xAOD::EgammaHelpers::isFCAL(&cluster) ? 
                cluster.eta() : cluster.etaBE(2);
     
    }
    /** @brief Pack eta & phi into the point type expected by the Acts multi axis,
     *         which is a std::array<double, DIM> */
    inline std::array<double, 2> pack(const double eta, const double phi) {
        return std::array{eta, phi};
    }
}

namespace ActsTrk{
    StatusCode CaloExtensionAlg::initialize() {
        ATH_CHECK(m_clusterSelector.retrieve(EnableTool{!m_clusterSelector.empty()}));
        ATH_CHECK(m_trackSelector.retrieve(EnableTool{!m_trackSelector.empty()}));
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());

        ATH_CHECK(m_clusterContainerKey.initialize());
        ATH_CHECK(m_trackParticleContainerKey.initialize());
        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_extensionDecorKey.initialize());
        ATH_CHECK(m_caloDetDescrMgrKey.initialize(m_clusterSelector.isEnabled()));
        ATH_CHECK(m_caloExtensionKey.initialize());

        // Here we extract the geometry identifiers of the 4 calo volumes in the ACTS geometry
        // It seems more robust to do the matching with the volume name, since the geometry ID
        // can change if details of the geometry change.
        std::vector<std::pair<std::string, CaloSample>> volIndexNames{};
        for (std::size_t id = 0 ; id < Acts::toUnderlying(CaloSample::Unknown); ++id){
            const auto smpId = static_cast<CaloSample>(id);
            volIndexNames.emplace_back(std::make_pair(CaloSampling::getSamplingName(smpId), smpId));
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - "<<volIndexNames.back().first<<" -> "
                <<volIndexNames.back().second);
        }
        /**  */
        const Acts::TrackingVolume* caloExit = m_trackingGeometrySvc->getEnvelope(SystemEnvelope::CaloExit);
        
        caloExit->visitVolumes([&](const Acts::TrackingVolume *vol) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Check volume: "<<vol->volumeName()<<".");
            auto smpItr = std::ranges::find_if(volIndexNames, [&](const auto& sampleID) {
                return vol->volumeName().starts_with(sampleID.first);
            });
            if (smpItr != volIndexNames.end()) {
                m_geoLayerIds[vol->geometryId()] = smpItr->second;
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Assign geometryID "<<vol->geometryId()
                     <<"volume: "<<vol->volumeName()<<", to "<<smpItr->second<<".");
            }
        });
      
       
        return StatusCode::SUCCESS;
    }
    CaloExtensionAlg::SortedCluster_t 
       CaloExtensionAlg::selectAndSort(const xAOD::CaloClusterContainer& clusters,
                                       const CaloDetDescrManager* detMgr) const{
        const std::size_t nEtaBins = std::ceil((2. * m_maxClustEta) / m_broadDeltaEta);
        const std::size_t nPhiBins = std::ceil((2.*std::numbers::pi) / m_broadDeltaPhi);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Create grid to sort the clusters with "<<nEtaBins
                     <<" bins in eta and "<<nPhiBins<<" bins in phi.");
        SortedCluster_t grid{EtaAxis_t{-m_maxClustEta, m_maxClustEta,nEtaBins},
                            PhiAxis_t{-std::numbers::pi, std::numbers::pi, nPhiBins}};
       
        std::size_t selected{0ul};
        for (const xAOD::CaloCluster* cluster : clusters) {
            if(cluster->et() < m_minClustEt ||
               std::abs(clusterEta(*cluster)) > m_maxClustEta ||
              (m_clusterSelector.isEnabled()  && !m_clusterSelector->passSelection(cluster, *detMgr))) {
              continue;
            }
            ClusterVec_t& bin = grid.atPosition(pack(clusterEta(*cluster), cluster->phi()));
            bin.push_back(cluster);
            ++selected;
        }
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Selected "<<selected
                      <<" out of "<<clusters.size()<<" calo clusters.");
        return grid;   
    }

    std::unique_ptr<CaloExtension> CaloExtensionAlg::propagateToCaloExit(const EventContext& ctx,
                                                                         const xAOD::TrackParticle* track) const{
  
        const Acts::TrackingVolume* caloExit = m_trackingGeometrySvc->getEnvelope(SystemEnvelope::CaloExit);
       
        auto extension = std::make_unique<CaloExtension>(track);
        /// Retrieve the last track parameters with a measurement state
        auto lastTrackPars = extension->lastParameters();
        if (!lastTrackPars) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - The track does not have any Acts::BoundTrack parameters");
            return nullptr;
        }
        using SurfaceRecordOptions = IExtrapolationTool::SurfaceRecordOptions;
        SurfaceRecordOptions propOpts{caloExit, IExtrapolationTool::VolumeAbort::atExit};
        propOpts.recordMaterial = true;
        propOpts.recordPassive = true;
        propOpts.recordSensitive = true;

        auto surfaceRecord = m_extrapolationTool->propagateAndRecord(ctx, *lastTrackPars, propOpts);
        if (!surfaceRecord.ok()) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Propagation through calorimeter did not succeed");
            return nullptr;
        }
        /// Loop over the recorded bound track parameters to append them onto the surface
        for (Acts::BoundTrackParameters& record : *surfaceRecord) {
            if (&record.referenceSurface()  == &(lastTrackPars->referenceSurface())) {
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Append calorimeter parameters: "
                <<record<<",\n surface: "<<record.referenceSurface().bounds()<<".");
            extension->appendParameters(std::move(record));
        }
        if (extension->empty()){
            extension.reset();
        }
        return extension;
    }
    StatusCode CaloExtensionAlg::execute(const EventContext& ctx) const {

        const xAOD::TrackParticleContainer* idTracks{nullptr};
        const xAOD::CaloClusterContainer* caloClusters{nullptr};
        const CaloDetDescrManager* detMgr{nullptr};
        /** Retrieve the input */
        ATH_CHECK(SG::get(idTracks, m_trackParticleContainerKey,ctx));
        ATH_CHECK(SG::get(caloClusters, m_clusterContainerKey, ctx));
        ATH_CHECK(SG::get(detMgr, m_caloDetDescrMgrKey, ctx));
        
        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
        /** Prepare the clusters to match */
        const SortedCluster_t coneClusters = selectAndSort(*caloClusters, detMgr);
        /** The binning of the cluster grid, used to look up the bins of interest */
        const auto& clusterAxes = coneClusters.multiAxis();

        SG::WriteHandle writeHandle{m_caloExtensionKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<CaloExtensionContainer>()));
        
        using Link_t = ElementLink<CaloExtensionContainer>;
        SG::WriteDecorHandle<xAOD::TrackParticleContainer, Link_t> decorHandle{m_extensionDecorKey, ctx};
        
        /** Loop over the track particles */
        for (const xAOD::TrackParticle* track : *idTracks) {
            Link_t& extensionLink = decorHandle(*track);
            if (track->pt() < m_trackPt ||
                (m_trackSelector.isEnabled() && !m_trackSelector->accept(*track))){
                continue;
            }
            auto extension = propagateToCaloExit(ctx, track);
            if (!extension) {
                continue;
            }
            // Loop over the neighbour bins corresponding to the track eta, phi
            // and match the clusters in that bin to the track
            const auto trackPos = pack(track->eta(), track->phi());
            for (const auto binIdx : clusterAxes.getNeighborHoodIndices(
                                        clusterAxes.getLocalBinsFromPoint(trackPos))){
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<"() - Try to match "<<coneClusters.at(binIdx).size()<<
                             " clusters from bin "<<binIdx<<". Central bin "
                <<clusterAxes.getGlobalBinFromPoint(trackPos));
                matchClusters(tgContext, coneClusters.at(binIdx), *extension);
            }

            extensionLink = Link_t{writeHandle.cptr(), writeHandle->size()};
            writeHandle->push_back(std::move(extension));
        }

        return StatusCode::SUCCESS;
    }
    void CaloExtensionAlg::matchClusters(const Acts::GeometryContext& tgContext,
                                         std::span<const xAOD::CaloCluster* const> clusterContainer,
                                         CaloExtension& caloExtension) const {
        
        const auto exitPars = caloExtension.lastTrackParameters();
        
        for (const xAOD::CaloCluster* matchMe : clusterContainer) {
            if (!checkBroadCriteria(tgContext, *matchMe, *exitPars)) {
                continue;
            }
            for (const Acts::BoundTrackParameters& recordedPars: caloExtension.parameters()) {
                const Acts::Surface& surf = recordedPars.referenceSurface();

                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Try to match "<<recordedPars
                    <<",\nsurface:"<<surf.geometryId()<<", bounds: "<<surf.bounds());
                const Acts::GeometryIdentifier volId = surf.geometryId().withSensitive(0).withBoundary(0);
                
                auto layerItr = m_geoLayerIds.find(volId);
                if (layerItr == m_geoLayerIds.end()){
                    continue;
                }
                const float dEta = clusterEta(*matchMe) - Acts::VectorHelpers::eta(recordedPars);
                const float dPhi = P4Helpers::deltaPhi(matchMe->phi(), recordedPars.phi());
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - dEta: "<<dEta<<", dPhi: "<<dPhi<<".");
                if (std::abs(dEta) < m_narrowDeltaEta && 
                    std::abs(dPhi) < m_narrowDeltaPhi) {
                    caloExtension.associateCluster(matchMe);
                    break;
                }
            }
        }
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" -  Associated: "<<caloExtension.associatedClusters().size()
                      <<" clusters: "<<clusterContainer.size()<<".");
    }
    bool CaloExtensionAlg::checkBroadCriteria(const Acts::GeometryContext& tgContext,
                                              const xAOD::CaloCluster& cluster,
                                              const Acts::BoundTrackParameters& lastTrkPars) const {
        using namespace CandidateMatchHelpers; 
        // Get Cluster parameters.
        const double etaClust = clusterEta(cluster);
        const bool isEndCap = !xAOD::EgammaHelpers::isBarrel(&cluster);

        const Amg::Vector3D globTrkPos = lastTrkPars.position(tgContext);
        const double trkEta = Acts::VectorHelpers::eta(lastTrkPars);
        // Calculate the eta/phi of the cluster as would be seen from the perigee
        // position of the Track.
        const Amg::Vector3D globalClusterPosWrtPerigee = approxXYZwrtPoint(cluster, globTrkPos, isEndCap);

        auto passDeltaPhi = [&]() -> bool {
            using namespace P4Helpers;
            const double clusterPhi = globalClusterPosWrtPerigee.phi();
            const double trkPhi = lastTrkPars.phi();
            if (std::abs(deltaPhi(trkPhi, clusterPhi)) < m_broadDeltaPhi) {
                return true;
            }
            // Calculate the possible rotation of the track.
            // Once assuming the cluster Et being the better estimate (e.g big brem).
            const double phiRotRescaled = PhiROT(cluster.et(), trkEta, 
                                                 lastTrkPars.charge(), 
                                                 globTrkPos.perp(), isEndCap);

             // DeltaPhi between the track and the cluster accounting for rotation assuming
            if (std::abs(deltaPhi(clusterPhi, deltaPhi(trkPhi, phiRotRescaled))) < m_broadDeltaPhi) {
                return true;
            }
            // And also assuming the track Pt being correct.
            const double phiRotTrack = PhiROT(lastTrkPars.transverseMomentum(), trkEta, 
                                              lastTrkPars.charge(), globTrkPos.perp(), isEndCap);

            
            // DeltaPhi between the track and the cluster accounting for rotation.
            if (std::abs(deltaPhi(clusterPhi, deltaPhi(trkPhi, phiRotTrack))) < m_broadDeltaPhi) {
                return true;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - broad dPhi matching fails with track phi: "<<
                trkPhi<<", phiRotCluster: "<<phiRotRescaled<<", phiRotTrack: "<<phiRotTrack<<", "
                "cluster phi: "<<cluster.phi()<<", phi corrected: "<<clusterPhi);
            return false;
        };

        if (!passDeltaPhi()) {
            return false;
        }
        /// Broad eta check
        if (std::abs(etaClust - trkEta) < m_broadDeltaEta || 
            std::abs(globalClusterPosWrtPerigee.eta() - trkEta) < m_broadDeltaEta) {
            return true;
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" broad dEta matching fails with track eta: "
            <<trkEta<<", cluster eta: "<<etaClust<<", corrected eta: "<<globalClusterPosWrtPerigee.eta());
        return false;
    }

}