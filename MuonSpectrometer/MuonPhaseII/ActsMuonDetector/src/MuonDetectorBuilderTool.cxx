/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonDetectorBuilderTool.h"

#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonChamber.h>


#include "Acts/ActsVersion.hpp"
#include "Acts/Geometry/CutoutCylinderVolumeBounds.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Detector/DetectorVolume.hpp"
#include "Acts/Detector/PortalGenerators.hpp"
#include "Acts/Navigation/DetectorVolumeFinders.hpp"
#include "Acts/Navigation/InternalNavigation.hpp"
#include "Acts/Navigation/NavigationDelegates.hpp"
#include "Acts/Navigation/NavigationState.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Detector/MultiWireStructureBuilder.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

#include<climits>



using MuonChamberSet = MuonGMR4::MuonDetectorManager::MuonChamberSet;
namespace ActsTrk {

    using volumePtr= std::shared_ptr<Acts::Experimental::DetectorVolume>;
    using surfacePtr = std::shared_ptr<Acts::Surface>;
    using StripLayerPtr = GeoModel::TransientConstSharedPtr<MuonGMR4::StripLayer>;

    MuonDetectorBuilderTool::MuonDetectorBuilderTool( const std::string& type, const std::string& name, const IInterface* parent ):
        AthAlgTool(type, name, parent){
            declareInterface<IDetectorVolumeBuilderTool>(this);
    }

    StatusCode MuonDetectorBuilderTool::initialize() {
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_MSG_DEBUG("ACTS version is: v"<< Acts::VersionMajor << "." << Acts::VersionMinor << "." << Acts::VersionPatch << " [" << Acts::CommitHash << "]");

        return StatusCode::SUCCESS;
    }

    Acts::Experimental::DetectorComponent MuonDetectorBuilderTool::construct(const Acts::GeometryContext& context) const{
        ATH_MSG_DEBUG("Building Muon Detector Volume");
        const MuonChamberSet chambers = m_detMgr->getAllChambers();
        using ChamberIdKey = std::pair<Muon::MuonStationIndex::ChIndex, unsigned int>;
        using EnvelopeIdentifierMap = std::map<ChamberIdKey, unsigned int>;
        using ChamberIdentifierMap = std::map<unsigned int, unsigned int>;
        EnvelopeIdentifierMap volIds{};
        ChamberIdentifierMap chamberIds{};
        const ActsGeometryContext* gctx = context.get<const ActsGeometryContext* >();
        std::vector< std::shared_ptr< Acts::Experimental::DetectorVolume > > detectorVolumeBoundingVolumes{};
        detectorVolumeBoundingVolumes.reserve(chambers.size());
        std::vector<surfacePtr> surfaces = {};
        std::pair<std::vector<volumePtr>, std::vector<surfacePtr>> readoutElements; 

        auto portalGenerator = Acts::Experimental::defaultPortalGenerator();

        for(const MuonGMR4::MuonChamber* chamber : chambers){
          unsigned int volNumb = volIds[std::make_pair(chamber->chamberIndex(), chamber->stationPhi())];
          if (!volNumb) volNumb = volIds.size();

          unsigned int chNumb = (++chamberIds[volNumb]);

          std::shared_ptr<Acts::TrapezoidVolumeBounds> bounds = chamber->bounds();
          readoutElements = constructElements(*gctx, *chamber, std::make_pair(volNumb,chNumb));
          volumePtr detectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(
                portalGenerator, gctx->context(), std::to_string(chamber->stationName())+"_"+std::to_string(chamber->stationEta())+"_"+std::to_string(chamber->stationPhi()), chamber->localToGlobalTrans(*gctx), 
                bounds, readoutElements.second, readoutElements.first, Acts::Experimental::tryAllSubVolumes(), Acts::Experimental::tryAllPortalsAndSurfaces());
          detectorVolume->assignGeometryId(Acts::GeometryIdentifier{}.setLayer(volNumb).setVolume(chNumb));
          if(m_dumpVisual){
                //If we want to view each volume independently                
                const MuonGMR4::MuonChamber::ReadoutSet readOut = chamber->readOutElements();

                Acts::ObjVisualization3D helper;
                Acts::GeometryView3D::drawDetectorVolume(helper, *detectorVolume, gctx->context());
                helper.write(m_idHelperSvc->toStringDetEl(readOut[0]->identify())+".obj");
                helper.clear();
          }
          detectorVolumeBoundingVolumes.push_back(std::move(detectorVolume));
          readoutElements.first.clear();
          readoutElements.second.clear();
        }
        std::unique_ptr<Acts::CutoutCylinderVolumeBounds> msBounds = std::make_unique<Acts::CutoutCylinderVolumeBounds>(0, 4000, 145000, 220000, 3200);
        volumePtr msDetectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(
                    portalGenerator, gctx->context(), "Envelope", 
                    Acts::Transform3::Identity(), std::move(msBounds), surfaces, 
                    detectorVolumeBoundingVolumes, Acts::Experimental::tryAllSubVolumes(), 
                    Acts::Experimental::tryAllPortalsAndSurfaces());
        msDetectorVolume->assignGeometryId(Acts::GeometryIdentifier{}.setVolume(15));

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

std::pair<std::vector<volumePtr>,std::vector<surfacePtr>> MuonDetectorBuilderTool::constructElements(const ActsGeometryContext& gctx, const MuonGMR4::MuonChamber& mChamber, std::pair<unsigned int, unsigned int> chId) const{
    
  std::vector<volumePtr> readoutDetectorVolumes = {};
  std::vector<surfacePtr> readoutSurfaces = {}; 
  std::pair<std::vector<volumePtr>, std::vector<surfacePtr>> pairElements;
  Acts::GeometryIdentifier::Value surfId{1};
  Acts::GeometryIdentifier::Value mdtId{1};

  MuonGMR4::MuonChamber::ReadoutSet readoutElements = mChamber.readOutElements();

  for(const  MuonGMR4::MuonReadoutElement* ele : readoutElements){

    if(ele->detectorType() == DetectorType::Mdt){
    
      ATH_MSG_VERBOSE("Building MultiLayer for MDT Detector Volume");
      const MuonGMR4::MdtReadoutElement* mdtReadoutEle = static_cast<const MuonGMR4::MdtReadoutElement*>(ele);
      const MuonGMR4::MdtReadoutElement::parameterBook& parameters{mdtReadoutEle->getParameters()};
      std::vector<surfacePtr> surfaces = {};       
  
      //loop over the tubes to get the surfaces  
      for(unsigned int lay=1; lay<=mdtReadoutEle->numLayers(); ++lay){
          for(unsigned int tube=1; tube<=mdtReadoutEle->numTubesInLay(); ++tube){
              const IdentifierHash measHash{mdtReadoutEle->measurementHash(lay,tube)};
              if (!mdtReadoutEle->isValid(measHash)) continue;         
              surfacePtr surface = mdtReadoutEle->surfacePtr(measHash);             
              surface->assignGeometryId(Acts::GeometryIdentifier{}.setLayer(chId.first).setVolume(chId.second).setBoundary(mdtId).setSensitive(++surfId)); 
              surfaces.push_back(surface);            

            }
       }
       
       //Get the transformation to the chamber's frame
       const Amg::Vector3D toChamber = mChamber.globalToLocalTrans(gctx)*mdtReadoutEle->center(gctx);
      
       const Amg::Vector3D boxCenter = toChamber;
                                     
       const Acts::Transform3 mdtTransform = mChamber.localToGlobalTrans(gctx) * Amg::Translation3D(boxCenter);       

       Acts::Experimental::MultiWireStructureBuilder::Config mlCfg;        
       
       mlCfg.name = "MDT_MultiLayer" + std::to_string(mdtReadoutEle->multilayer()) + "_" + std::to_string(mdtReadoutEle->stationName())+ "_"+std::to_string(mdtReadoutEle->stationEta())+"_"+std::to_string(mdtReadoutEle->stationPhi());
       mlCfg.mlSurfaces = surfaces;
       mlCfg.transform = mdtTransform;      
       std::unique_ptr<Acts::TrapezoidVolumeBounds> mdtBounds = std::make_unique<Acts::TrapezoidVolumeBounds>(parameters.shortHalfX, parameters.longHalfX, parameters.halfY, parameters.halfHeight);
       mlCfg.mlBounds= mdtBounds->values();
       mlCfg.mlBinning = {Acts::Experimental::ProtoBinning(Acts::binY, Acts::detail::AxisBoundaryType::Bound,                   
           -mdtBounds->values()[1], mdtBounds->values()[1], std::lround(2*mdtBounds->values()[1]/parameters.tubePitch), 0u), Acts::Experimental::ProtoBinning(Acts::binZ, Acts::detail::AxisBoundaryType::Bound,                   
           -mdtBounds->values()[2], mdtBounds->values()[2], std::lround(2*mdtBounds->values()[2]/parameters.tubePitch), 0u)};
                     
       Acts::Experimental::MultiWireStructureBuilder mdtBuilder(mlCfg);      
       volumePtr mdtVolume = mdtBuilder.construct(gctx.context()).volumes[0];
       mdtVolume->assignGeometryId(Acts::GeometryIdentifier{}.setLayer(chId.first).setVolume(chId.second).setBoundary(mdtId++));

       
       readoutDetectorVolumes.push_back(mdtVolume); 
       
    }else if(ele->detectorType() == DetectorType::Rpc){

      ATH_MSG_VERBOSE("Building RPC plane surface");
      const MuonGMR4::RpcReadoutElement* rpcReadoutEle = static_cast<const MuonGMR4::RpcReadoutElement*>(ele);
      const MuonGMR4::RpcReadoutElement::parameterBook& parameters{rpcReadoutEle->getParameters()};
      //loop over the layers to get the surfaces
      for(const StripLayerPtr& layer : parameters.layers){
        if (!layer) continue;
        const IdentifierHash layerHash = layer->hash();
        surfacePtr rpcSurface = rpcReadoutEle->surfacePtr(layerHash);
        rpcSurface->assignGeometryId(Acts::GeometryIdentifier{}.setLayer(chId.first).setVolume(chId.second).setSensitive(++surfId));
        readoutSurfaces.push_back(rpcSurface);        
      }


    }else if(ele->detectorType() == DetectorType::Tgc){

      ATH_MSG_VERBOSE("Building TGC plane surface ");
      const MuonGMR4::TgcReadoutElement* tgcReadoutEle = static_cast<const MuonGMR4::TgcReadoutElement*>(ele);
      const MuonGMR4::TgcReadoutElement::parameterBook& parameters{tgcReadoutEle->getParameters()};
      //loop over the layers to get the surfaces
      for(const StripLayerPtr& layerPtr : parameters.sensorLayouts){
        if (!layerPtr) continue;
        const IdentifierHash layerHash = layerPtr->hash();
        surfacePtr tgcSurface = tgcReadoutEle->surfacePtr(layerHash);
        tgcSurface->assignGeometryId(Acts::GeometryIdentifier{}.setLayer(chId.first).setVolume(chId.second).setSensitive(++surfId));

        readoutSurfaces.push_back(tgcSurface);

      }

    }else if(ele->detectorType() == DetectorType::sTgc){

      ATH_MSG_VERBOSE("Building Stgc strip layers");
      const MuonGMR4::sTgcReadoutElement* sTgcReadoutEle = static_cast<const MuonGMR4::sTgcReadoutElement*>(ele);
      const MuonGMR4::sTgcReadoutElement::parameterBook& paramaters{sTgcReadoutEle->getParameters()};
      //loop over the layers to get the surfaces
      for(const MuonGMR4::StripLayer& layer : paramaters.stripLayers){
        const IdentifierHash layerHash = layer.hash();
        surfacePtr stgcSurface = sTgcReadoutEle->surfacePtr(layerHash);
        stgcSurface->assignGeometryId(Acts::GeometryIdentifier{}.setLayer(chId.first).setVolume(chId.second).setSensitive(++surfId));

        readoutSurfaces.push_back(stgcSurface);
      }
      }else if(ele->detectorType() == DetectorType::Mm){
        ATH_MSG_VERBOSE("Building Mmg layers");
        const MuonGMR4::MmReadoutElement* mmReadoutEle = static_cast<const MuonGMR4::MmReadoutElement*>(ele);
        const MuonGMR4::MmReadoutElement::parameterBook& parameters{mmReadoutEle->getParameters()};
        //loop over the layers to get the surfaces
        for(const StripLayerPtr& layer : parameters.layers){
          const IdentifierHash layerHash = layer->hash();
          surfacePtr mmSurface = mmReadoutEle->surfacePtr(layerHash);
          mmSurface->assignGeometryId(Acts::GeometryIdentifier{}.setLayer(chId.first).setVolume(chId.second).setSensitive(++surfId));

          readoutSurfaces.push_back(mmSurface);
        }

      }else{
        ATH_MSG_VERBOSE("No detector type supported");

      }

  }

  pairElements = std::make_pair(readoutDetectorVolumes, readoutSurfaces);

return pairElements;

}

}
