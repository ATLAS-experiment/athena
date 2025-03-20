/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSystemExtensionTool.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "EventPrimitives/EventPrimitivesCovarianceHelpers.h"
#include "MuonCombinedEvent/InDetCandidate.h"
#include "MuonCombinedEvent/TagBase.h"
#include "MuonDetDescrUtils/MuonChamberLayerDescription.h"
#include "MuonIdHelpers/MuonStationIndexHelpers.h"
#include "MuonLayerEvent/MuonSystemExtension.h"
#include "MuonLayerEvent/MuonSystemExtensionCollection.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PlaneSurface.h"

namespace{
    template <class T>
    std::string to_string(const std::vector<T>& v) {
        std::stringstream ostr{};
        ostr<<"{";
        for (auto x: v)ostr<<x<<", ";
        ostr<<"}";
        return ostr.str();
    }
    /** @brief Calculate the path from the origin along the track parameters */
    double pathAlongPars(const Trk::TrackParameters& pars, const Amg::Vector3D& pos) {
        const Amg::Vector3D parDir{pars.momentum().unit()};
        return (pars.position().dot(parDir) > 0 ? 1 : -1) * (parDir.dot(pos));
    }
}
namespace Muon {




    MuonSystemExtensionTool::MuonSystemExtensionTool(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMuonSystemExtensionTool>(this);
    }

    StatusCode MuonSystemExtensionTool::initialize() {
        ATH_CHECK(m_caloExtensionTool.retrieve());
        ATH_CHECK(m_extrapolator.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        if (!initializeGeometry()) return StatusCode::FAILURE;

        return StatusCode::SUCCESS;
    }

    bool MuonSystemExtensionTool::initializeGeometry() {
        // initialize reference surfaces hash vectors per region and sector

        // first build the barrel, we need different reference planes for all sectors
        ATH_MSG_DEBUG("Initializing barrel ");
        for (unsigned int sector = 1; sector <= 16; ++sector) {
            // get rotation into sector frame
            double sectorPhi = m_sectorMapping.sectorPhi(sector);
            const Amg::Transform3D sectorRotation(Amg::getRotateZ3D(sectorPhi));
            if (!initializeGeometryBarrel(sector, sectorRotation)) return false;
            if (!initializeGeometryEndcap(sector, MuonStationIndex::EndcapA, sectorRotation)) return false;
            if (!initializeGeometryEndcap(sector, MuonStationIndex::EndcapC, sectorRotation)) return false;
        }

        return true;
    }

    bool MuonSystemExtensionTool::initializeGeometryEndcap(int sector, MuonStationIndex::DetectorRegionIndex regionIndex,
                                                           const Amg::Transform3D& sectorRotation) {
        ATH_MSG_DEBUG("Initializing endcap: sector " << sector << " " << MuonStationIndex::regionName(regionIndex));

        SurfaceVec& surfaces = m_referenceSurfaces[regionIndex][sector - 1];
        MuonChamberLayerDescription chamberLayerDescription;

        static const std::vector<MuonStationIndex::StIndex> layers = {MuonStationIndex::EI, MuonStationIndex::EE, MuonStationIndex::EM,
                                                         MuonStationIndex::EO};

        for (const MuonStationIndex::StIndex& stLayer : layers) {
            // calculate reference position from radial position of the layer
            MuonStationIndex::LayerIndex layer = MuonStationIndex::toLayerIndex(stLayer);
            MuonChamberLayerDescriptor layerDescriptor = chamberLayerDescription.getDescriptor(sector, regionIndex, layer);
            // reference transform + surface
            Amg::Transform3D trans{Amg::getTranslateZ3D(layerDescriptor.referencePosition) *sectorRotation};  //*Amg::AngleAxis3D(xToZRotation,Amg::Vector3D(0.,1.,0.)
            std::unique_ptr<Trk::PlaneSurface> surface = std::make_unique<Trk::PlaneSurface>(trans);
            // sanity checks
            if (msgLvl(MSG::VERBOSE)) {
                ATH_MSG_VERBOSE("initializeGeometryEndcap() --  sector " << sector << " layer " << MuonStationIndex::layerName(layer) << " surface "
                                            <<Amg::toString(surface->transform())
                                            << " center " << Amg::toString(surface->center()) << " theta " << surface->normal().theta() 
                                            << " normal:  " <<Amg::toString(surface->normal()));
                
                
            }

            MuonLayerSurface data(std::move(surface), sector, regionIndex, layer);
            surfaces.push_back(std::move(data));


        }
        ATH_MSG_VERBOSE(" Total number of surfaces " << surfaces.size());
        return true;
    }

    bool MuonSystemExtensionTool::initializeGeometryBarrel(int sector, const Amg::Transform3D& sectorRotation) {
        MuonChamberLayerDescription chamberLayerDescription;

        SurfaceVec& surfaces = m_referenceSurfaces[MuonStationIndex::Barrel][sector - 1];
        constexpr double xToZRotation = -M_PI_2;

        for (unsigned int stationLayer = MuonStationIndex::BI; stationLayer <= MuonStationIndex::BE; ++stationLayer) {
            // skip BEE if in small sectors, not installed
            if (stationLayer == MuonStationIndex::BE && MuonStationIndexHelpers::isSmall(sector)) continue;

            // calculate reference position from radial position of the laeyr
            MuonStationIndex::LayerIndex layer = MuonStationIndex::toLayerIndex((MuonStationIndex::StIndex)(stationLayer));
            MuonChamberLayerDescriptor layerDescriptor = chamberLayerDescription.getDescriptor(sector, MuonStationIndex::Barrel, layer);
            Amg::Vector3D positionInSector(layerDescriptor.referencePosition, 0., 0.);
            Amg::Vector3D globalPosition = sectorRotation * positionInSector;

            // reference transform + surface
            Amg::Transform3D trans(sectorRotation * Amg::getRotateY3D(xToZRotation));
            trans.pretranslate(globalPosition);
            std::unique_ptr<Trk::PlaneSurface> surface = std::make_unique<Trk::PlaneSurface>(trans);
            // sanity checks
            if (msgLvl(MSG::VERBOSE)) {
                double sectorPhi = m_sectorMapping.sectorPhi(sector);
                ATH_MSG_VERBOSE(" sector " << sector << " layer " << MuonStationIndex::layerName(layer) << " phi " << sectorPhi
                                           << " ref theta " << globalPosition.theta() << " phi " << globalPosition.phi() << " r "
                                           << globalPosition.perp() << " pos " << Amg::toString(globalPosition)
                                           << " lpos3d " << Amg::toString(surface->transform().inverse() * globalPosition) 
                                           << " normal: " << Amg::toString(surface->normal()));
            }
            MuonLayerSurface data(std::move(surface), sector, MuonStationIndex::Barrel, layer);
            surfaces.push_back(std::move(data));


        }
        return true;
    }

    bool MuonSystemExtensionTool::muonSystemExtension(const EventContext& ctx, SystemExtensionCache& cache) const {
        /// Get the calo extension
        if (!cache.candidate->getCaloExtension()) {
            if (!cache.extensionContainer) {
                std::unique_ptr<Trk::CaloExtension> caloExtension =
                        m_caloExtensionTool->caloExtension(ctx, cache.candidate->indetTrackParticle());
                if (!caloExtension || !caloExtension->muonEntryLayerIntersection()) {
                    ATH_MSG_VERBOSE("Failed to create the calo extension for "<<cache.candidate->toString());
                    return false;
                }
                cache.candidate->setExtension(caloExtension);
            } else {
                const Trk::CaloExtension* caloExtension = m_caloExtensionTool->caloExtension(cache.candidate->indetTrackParticle(),
                                                                    *cache.extensionContainer);
                if (!caloExtension || !caloExtension->muonEntryLayerIntersection()) {
                    ATH_MSG_VERBOSE("Failed to create the calo extension for "<<cache.candidate->toString());
                    return false;
                }
                cache.candidate->setExtension(caloExtension);
            }
        }

        if (!cache.createSystemExtension) {
            ATH_MSG_VERBOSE("No system extension is required for "<<cache.candidate->toString());
            return true;
        }
        // get entry parameters, use it as current parameter for the extrapolation
        const Trk::TrackParameters* currentPars = cache.candidate->getCaloExtension()->muonEntryLayerIntersection();

        // get reference surfaces
        ATH_MSG_VERBOSE(" getSurfacesForIntersection " << currentPars);
        SurfaceVec surfaces = getSurfacesForIntersection(*currentPars, cache);

        // store intersections
        std::vector<MuonSystemExtension::Intersection> intersections;

        // garbage collection
        std::vector<std::shared_ptr<Trk::TrackParameters> > trackParametersVec;

        // loop over reference surfaces
        for (const Muon::MuonLayerSurface& it : surfaces) {
            // extrapolate to next layer
            const Trk::Surface& surface = *it.surfacePtr;
            ATH_MSG_VERBOSE(" startPars: "<<m_printer->print(*currentPars));
            ATH_MSG_VERBOSE(" destination: sector " << it.sector << "  " << MuonStationIndex::regionName(it.regionIndex) << "  "
                                                    << MuonStationIndex::layerName(it.layerIndex) << " phi  " << surface.center().phi()
                                                    << " r " << surface.center().perp() << " z " << surface.center().z());

            std::unique_ptr<Trk::TrackParameters> exPars{m_extrapolator->extrapolate(ctx, *currentPars, surface, 
                                                                                     Trk::alongMomentum, false, Trk::muon)};
            if (!exPars) {
                ATH_MSG_VERBOSE("extrapolation failed, trying next layer ");
                continue;
            }
            ATH_MSG_DEBUG("Extrapolated in event "<<ctx.eventID().event_number()<<" track @ "<<m_printer->print(*exPars));

            //    reject intersections with very big uncertainties (parallel to surface)
            const double sigma_lx = Amg::error(*exPars->covariance(), Trk::locX);
            const double sigma_ly = Amg::error(*exPars->covariance(), Trk::locY);
            if (!Amg::hasPositiveDiagElems(*exPars->covariance()) || std::max(sigma_lx, sigma_ly) > 1. * Gaudi::Units::meter){
                ATH_MSG_DEBUG("Reject bad extrapolation "<<m_printer->print(*exPars));
                continue;
            }

            // create shared pointer and add to garbage collection
            std::shared_ptr<Trk::TrackParameters> sharedPtr{std::move(exPars)};
            trackParametersVec.emplace_back(sharedPtr);

            intersections.emplace_back(sharedPtr, it);
            constexpr double TenCm = 10. * Gaudi::Units::cm;
            if(std::hypot(sigma_lx, sigma_ly) < std::max(TenCm, 10. * std::hypot(Amg::error(*currentPars->covariance(), Trk::locX),
                                                                                 Amg::error(*currentPars->covariance(), Trk::locY)))) {
                // only update the parameters if errors don't blow up
                currentPars = sharedPtr.get();
            } else {
                ATH_MSG_DEBUG("Extrapolation reached at "<<m_printer->print(*sharedPtr)<<" has larger uncertainties than "<<
                              m_printer->print(*currentPars));
            }
        }
        ATH_MSG_DEBUG("Completed extrapolation: destinations " << surfaces.size() << " intersections " << intersections.size());
        if (intersections.empty()){
            ATH_MSG_DEBUG("No system extensions are made for "<<cache.candidate->toString()
                         <<" will accept the candidate: "<< (!cache.requireSystemExtension ? "si": "no"));
            return !cache.requireSystemExtension;
        }
        cache.candidate->setExtension(std::make_unique<MuonSystemExtension>(cache.candidate->getCaloExtension()->muonEntryLayerIntersection(), 
                                                                            std::move(intersections)));
        return true;
    }
    MuonSystemExtensionTool::SurfaceVec MuonSystemExtensionTool::getSurfacesForIntersection(const Trk::TrackParameters& muonEntryPars,
                                                                                            const SystemExtensionCache& cache) const {
        // if in endcaps pick endcap surfaces
        const double eta = muonEntryPars.position().eta();
        MuonStationIndex::DetectorRegionIndex regionIndex = MuonStationIndex::Barrel;
        if (eta < -1.05) regionIndex = MuonStationIndex::EndcapC;
        if (eta > 1.05) regionIndex = MuonStationIndex::EndcapA;

        // in barrel pick primary sector
        const double phi = muonEntryPars.position().phi();
        std::vector<int> sectors;
        m_sectorMapping.getSectors(phi, sectors);
        SurfaceVec surfaces;
        /// Look whether one of the sectors has actually a recorded hit
        if (cache.useHitSectors) {
            const auto map_itr = cache.sectorsWithHits->find(regionIndex);
            if (map_itr == cache.sectorsWithHits->end()) {
                ATH_MSG_DEBUG("No hits in detector region " << Muon::MuonStationIndex::regionName(regionIndex));
                return surfaces;
            }
            std::vector<int>::const_iterator sec_itr = std::find_if(
                sectors.begin(), sectors.end(), [&map_itr](const int& sector) -> bool { return map_itr->second.count(sector); });
            if (sec_itr == sectors.end()) {
                ATH_MSG_DEBUG("No hits found for sector " << m_sectorMapping.getSector(phi) << " in MuonStation "
                                                          << Muon::MuonStationIndex::regionName(regionIndex));
                return surfaces;
            }
        }

        for (const int sector : sectors) {
            surfaces.insert(surfaces.end(), m_referenceSurfaces[regionIndex][sector - 1].begin(),
                            m_referenceSurfaces[regionIndex][sector - 1].end());
        }
        std::stable_sort(surfaces.begin(), surfaces.end(), 
                         [&muonEntryPars](const MuonLayerSurface& s1, const MuonLayerSurface& s2) {
                            return std::abs(pathAlongPars(muonEntryPars,s1.surfacePtr->center())) <
                                   std::abs(pathAlongPars(muonEntryPars,s2.surfacePtr->center()));
                            
                         });
        if (msgLvl(MSG::VERBOSE)) {
            for (auto& s1 : surfaces) {
                ATH_MSG_VERBOSE("Surface "<<Muon::MuonStationIndex::layerName(s1.layerIndex)
                              <<", center: "<<Amg::toString(s1.surfacePtr->center())
                              <<", pathAlongPars "<<pathAlongPars(muonEntryPars,s1.surfacePtr->center())
                              <<std::endl<<(*s1.surfacePtr));

        	}
        }
        return surfaces;
    }
    bool MuonSystemExtensionTool::muonLayerInterSections(const EventContext& ctx,
                                                         const MuonCombined::TagBase& cmbTag,
                                                         SystemExtensionCache& cache) const{

        const Trk::Track* cmbTrk = cmbTag.primaryTrack();
        if (!cmbTrk){
            ATH_MSG_WARNING("A combined tag without any track? Please check "<<cmbTag.toString());
            return false;
        }
        /// Get the proper surfaces for the intersections
        const Trk::TrackParameters* entryPars = cache.candidate->getCaloExtension()->muonEntryLayerIntersection();
        std::vector<const Trk::TrackStateOnSurface*> cmbParVec{};

        ATH_MSG_VERBOSE("Calo entry parameters "<<m_printer->print(*entryPars));
        std::vector<Muon::MuonSystemExtension::Intersection> intersections{};
        Muon::MuonLayerSurface lastSurf{};
        
        const Trk::TrackStates::const_iterator end_itr = cmbTrk->trackStateOnSurfaces()->end();
        for (Trk::TrackStates::const_iterator itr = cmbTrk->trackStateOnSurfaces()->begin(); itr != end_itr; ++itr) {
            const Trk::TrackStateOnSurface* msTSOS{*itr};
            ATH_MSG_VERBOSE("Track state "<<m_printer->print(*msTSOS));

            if (!msTSOS->measurementOnTrack()) {
                continue;
            } 
            const Identifier measId = m_edmHelperSvc->getIdentifier(*msTSOS->measurementOnTrack());
            const Trk::TrackParameters& msPars{*msTSOS->trackParameters()};
            /** The parameters are not muon paramaters or the parameters are before the entrance parameters */
            if (!m_idHelperSvc->isMuon(measId) || pathAlongPars(msPars, msPars.position() - entryPars->position())< 0.){
                continue;
            }
               
            Muon::MuonStationIndex::DetectorRegionIndex regionIdx = m_idHelperSvc->regionIndex(measId);
            Muon::MuonStationIndex::LayerIndex layerIdx = m_idHelperSvc->layerIndex(measId);
            if (layerIdx == Muon::MuonStationIndex::BarrelExtended) {
                regionIdx = Muon::MuonStationIndex::DetectorRegionIndex::Barrel;
                layerIdx = Muon::MuonStationIndex::Inner;
            }

            const int sector = m_sectorMapping.getSector(msTSOS->measurementOnTrack()->associatedSurface().center().phi());
       
            /// The track parameters belong the same surface. Do nothing
            if (lastSurf.layerIndex == layerIdx && lastSurf.regionIndex == regionIdx && lastSurf.sector == sector) {
                continue;
            }
            const SurfaceVec& refSurfaces = m_referenceSurfaces[regionIdx][sector - 1];
            SurfaceVec::const_iterator surfItr = std::find_if(refSurfaces.begin(), refSurfaces.end(),
                                                    [&layerIdx](const MuonLayerSurface& surf){
                                                    return surf.layerIndex == layerIdx;
                                                    });
            if (surfItr == refSurfaces.end()) {
                ATH_MSG_WARNING("Failed to find a reference surface matching to combined parameters "<<m_printer->print(*msTSOS)
                                <<", sector: "<<sector <<", layer: "<<Muon::MuonStationIndex::layerName(layerIdx)
                                <<", region index: "<<Muon::MuonStationIndex::regionName(regionIdx));
                continue;
            }
            lastSurf = (*surfItr);
            const Trk::Surface& target{*lastSurf.surfacePtr};
                
            /// Check whether there're measurement parameters that are closer to the target surface
            for (Trk::TrackStates::const_iterator closePars = itr+1; closePars != end_itr; ++closePars) {
                const Trk::TrackStateOnSurface * tsosInChamb{*closePars};
                if (!tsosInChamb->measurementOnTrack()) continue; 
                
                const Trk::TrackParameters& chPars{*tsosInChamb->trackParameters()};
                if (std::abs(pathAlongPars(chPars, chPars.position() - target.center())) >
                    std::abs(pathAlongPars(*msTSOS->trackParameters(), 
                                            msTSOS->trackParameters()->position() - target.center()))) {
                    break;
                }
                itr =  closePars -1;
                msTSOS = *itr;
            }
                
            std::unique_ptr<Trk::TrackParameters> exPars{m_extrapolator->extrapolate(ctx, *msTSOS->trackParameters(), 
                                                                                     target, Trk::anyDirection, false, Trk::muon)};
            if (!exPars) {
                ATH_MSG_VERBOSE(__LINE__<<" - Failed to extrapolate track @ "<<m_printer->print(*msTSOS)
                                <<", sector: "<<sector
                                <<", layer: "<<Muon::MuonStationIndex::layerName(layerIdx)
                                <<", region index: "<<Muon::MuonStationIndex::regionName(regionIdx)
                                <<" to surface "<<std::endl<<target<<std::endl
                                <<", sector: "<<lastSurf.sector
                                <<", layer: "<<Muon::MuonStationIndex::layerName(lastSurf.layerIndex)
                                <<", region index: "<<Muon::MuonStationIndex::regionName(lastSurf.regionIndex)
                                <<" pathAlongPars "<<pathAlongPars(*msTSOS->trackParameters(), target.center())
                                <<", dir: "<<Amg::toString(msTSOS->trackParameters()->momentum().unit()));
                continue;
            }
            intersections.emplace_back(std::move(exPars), lastSurf);
            
        }
        
        ATH_MSG_VERBOSE("Selected "<<intersections.size()<<" intersections");
        
        if (intersections.empty()) {
            ATH_MSG_DEBUG("Failed to find the intersections for the combined track");
            return false;
        }
        cache.candidate->setExtension(std::make_unique<MuonSystemExtension>(entryPars, std::move(intersections)));

        return true;
    }
}  // namespace Muon
