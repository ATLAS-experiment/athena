/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonDetectorBuilderTool.h"

#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>


#include "Acts/Geometry/CutoutCylinderVolumeBounds.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Detector/DetectorVolume.hpp"
#include "Acts/Detector/PortalGenerators.hpp"
#include "Acts/Navigation/DetectorVolumeFinders.hpp"
#include "Acts/Navigation/SurfaceCandidatesUpdaters.hpp"
#include "Acts/Navigation/NavigationDelegates.hpp"
#include "Acts/Navigation/NavigationState.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"




using MuonChamberSet = MuonGMR4::MuonDetectorManager::MuonChamberSet;
namespace ActsTrk {

    MuonDetectorBuilderTool::MuonDetectorBuilderTool( const std::string& type, const std::string& name, const IInterface* parent ):
        AthAlgTool(type, name, parent){
            declareInterface<IDetectorVolumeBuilderTool>(this);
    }

    StatusCode MuonDetectorBuilderTool::initialize() {
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_idHelperSvc.retrieve());
        return StatusCode::SUCCESS;
    }

    Acts::Experimental::DetectorComponent MuonDetectorBuilderTool::construct(const Acts::GeometryContext& context) const {
        ATH_MSG_DEBUG("Building Muon Detector Volume");
        const MuonChamberSet chambers = m_detMgr->getAllChambers();

        const ActsGeometryContext* gctx = context.get<const ActsGeometryContext* >();
        std::vector< std::shared_ptr< Acts::Experimental::DetectorVolume > > detectorVolumeBoundingVolumes{};
        detectorVolumeBoundingVolumes.reserve(chambers.size());
        std::vector<std::shared_ptr<Acts::Surface> > surfaces = {};

        auto portalGenerator = Acts::Experimental::defaultPortalGenerator();
        for(const MuonGMR4::MuonChamber* chamber : chambers) {
            std::shared_ptr<Acts::TrapezoidVolumeBounds> bounds = chamber->bounds();
            std::shared_ptr<Acts::Experimental::DetectorVolume> detectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(
                portalGenerator, gctx->context(), std::to_string(chamber->stationName())+"_"+
                                                  std::to_string(chamber->stationEta())+"_"+
                                                  std::to_string(chamber->stationPhi()), 
                                                   chamber->localToGlobalTrans(*gctx), 
                bounds, Acts::Experimental::tryAllPortals());
            if(m_dumpVisual){
                //If we want to view each volume independently
                const MuonGMR4::MuonChamber::ReadoutSet readOut = chamber->readOutElements();
                Acts::ObjVisualization3D helper;
                Acts::GeometryView3D::drawDetectorVolume(helper, *detectorVolume, gctx->context());
                helper.write(m_idHelperSvc->toStringDetEl(readOut[0]->identify())+".obj");
                helper.clear();
            }
            detectorVolumeBoundingVolumes.push_back(std::move(detectorVolume));
        }
        std::unique_ptr<Acts::CutoutCylinderVolumeBounds> msBounds = std::make_unique<Acts::CutoutCylinderVolumeBounds>(0, 4000, 145000, 220000, 3200);
        std::shared_ptr<Acts::Experimental::DetectorVolume> msDetectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(
                    portalGenerator, gctx->context(), "Envelope", 
                    Acts::Transform3::Identity(), std::move(msBounds), surfaces, 
                    detectorVolumeBoundingVolumes, Acts::Experimental::tryAllSubVolumes(), 
                    Acts::Experimental::tryAllPortalsAndSurfaces());

        if (m_dumpVisual) {
            ATH_MSG_VERBOSE("Writing detector.obj");
            Acts::ObjVisualization3D helper;
            Acts::GeometryView3D::drawDetectorVolume(helper, *msDetectorVolume, gctx->context());
            helper.write("detector.obj");
            helper.clear();
        }

        Acts::Experimental::DetectorComponent::PortalContainer portalContainer;
        for (auto [ip, p] : Acts::enumerate(msDetectorVolume->portalPtrs())) {
            portalContainer[ip] = p;
        }

        return Acts::Experimental::DetectorComponent{
        {msDetectorVolume},
        portalContainer,
        {{msDetectorVolume}, Acts::Experimental::tryRootVolumes()}};
    }
}