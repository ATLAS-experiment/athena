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
#include <ActsGeometryInterfaces/IDetectorElement.h>
#include "GeoModelHelpers/TransformToStringConverter.h"

#include "Acts/ActsVersion.hpp"
#include "Acts/Geometry/CutoutCylinderVolumeBounds.hpp"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Visualization/GeometryView3D.hpp"
#include "Acts/Detector/DetectorVolume.hpp"
#include "Acts/Detector/PortalGenerators.hpp"
#include "Acts/Material/HomogeneousVolumeMaterial.hpp"
#include "Acts/Navigation/DetectorVolumeFinders.hpp"
#include "Acts/Navigation/InternalNavigation.hpp"
#include "Acts/Navigation/NavigationDelegates.hpp"
#include "Acts/Navigation/NavigationState.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Detector/MultiWireStructureBuilder.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Plugins/GeoModel/GeoModelMaterialConverter.hpp"
#include "Acts/Plugins/GeoModel/GeoModelToDetectorVolume.hpp"

#include <GeoModelKernel/GeoSimplePolygonBrep.h>
#include <GeoModelKernel/GeoShapeShift.h>
#include <GeoModelKernel/GeoShapeSubtraction.h>
#include <GeoModelKernel/GeoShapeUnion.h>
#include "GeoModelHelpers/getChildNodesWithTrf.h"
#include "ActsGeometryInterfaces/GeometryDefs.h"
#include <set>
#include <climits>



using MuonChamberSet = MuonGMR4::MuonDetectorManager::MuonChamberSet;
using DetectorVolume = Acts::Experimental::DetectorVolume;
namespace ActsTrk {

    using volumePtr= std::shared_ptr<DetectorVolume>;
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

    void MuonDetectorBuilderTool::getChamberPassives(const ActsGeometryContext* gctx, const MuonGMR4::MuonChamber* chamber, std::vector<volumePtr>& passiveVolumes) const {
        for(const MuonGMR4::MuonReadoutElement* ele : chamber->readOutElements()){
            const GeoVFullPhysVol* readOutVol = ele->getMaterialGeom();
            PVConstLink parentVolume = readOutVol->getParent();
            const Amg::Transform3D stationTransform = parentVolume->getX();
            //Loop through the parent, skipping the fullphysvol which are the readOutElements
            const std::vector<GeoChildNodeWithTrf> children = getChildrenWithRef(parentVolume, false);
            for(const GeoChildNodeWithTrf& childNode : children){
                if(typeid(*(childNode.volume)) != typeid(GeoFullPhysVol)){
                    ATH_MSG_VERBOSE("Processing " << childNode.nodeName);
                    const GeoShape* shape = childNode.volume->getLogVol()->getShape();
                    if(shape->typeID() == GeoSimplePolygonBrep::getClassTypeID() or 
                        shape->typeID() == GeoShapeUnion::getClassTypeID() or
                        shape->typeID() == GeoShapeShift::getClassTypeID() or
                        shape->typeID() == GeoShapeSubtraction::getClassTypeID()){
                        //Skip these for now until https://github.com/acts-project/acts/pull/3713 is merged in
                        continue;
                    }
                    const GeoMaterial* geoMaterial = childNode.volume->getLogVol()->getMaterial();
                    const Acts::Material aMat = Acts::GeoModel::geoMaterialConverter(*geoMaterial);
                    std::shared_ptr<Acts::HomogeneousVolumeMaterial> material = std::make_shared<Acts::HomogeneousVolumeMaterial>(aMat);
                    volumePtr passiveVolume = Acts::GeoModel::convertDetectorVolume(gctx->context(), *shape, "PASSIVE_"+childNode.nodeName+std::to_string(passiveVolumes.size()), childNode.transform * stationTransform, {});
                    passiveVolume->assignVolumeMaterial(material);
                    passiveVolume->assignGeometryId(Acts::GeometryIdentifier{}.setVolume(30).setSensitive(passiveVolumes.size()));
                    passiveVolumes.push_back(passiveVolume);
                }
            }
        }
    }

    void MuonDetectorBuilderTool::processPassiveNodes(const ActsGeometryContext* gctx, const GeoChildNodeWithTrf& node, const std::string& name, std::vector<volumePtr>& passiveVolumes, GeoTrf::Transform3D transform) const {
        std::vector<GeoChildNodeWithTrf> children = getChildrenWithRef(node.volume, false);
        for(const GeoChildNodeWithTrf& childNode : children){
            ATH_MSG_DEBUG("Child transform " << GeoTrf::toString(childNode.transform) << " Parent transform " << GeoTrf::toString(transform));
            ATH_MSG_DEBUG("Combined transform " << GeoTrf::toString(transform * childNode.transform));
            ATH_MSG_DEBUG("Child name " << name+"/"+childNode.nodeName);
            processPassiveNodes(gctx, childNode, name+"/"+childNode.nodeName, passiveVolumes, transform * childNode.transform);
        }
        if (!children.empty()) return;
        ATH_MSG_VERBOSE("Drawing volume named "<<name);
        const GeoShape* shape = node.volume->getLogVol()->getShape();
        if(shape->typeID() == GeoSimplePolygonBrep::getClassTypeID() or 
            shape->typeID() == GeoShapeUnion::getClassTypeID() or
            shape->typeID() == GeoShapeShift::getClassTypeID() or
            shape->typeID() == GeoShapeSubtraction::getClassTypeID()){
            //Skip these for now until https://github.com/acts-project/acts/pull/3713 is merged in
            return;
        }
        const GeoMaterial* geoMaterial = node.volume->getLogVol()->getMaterial();
        const Acts::Material aMat = Acts::GeoModel::geoMaterialConverter(*geoMaterial);
        std::shared_ptr<Acts::HomogeneousVolumeMaterial> material = std::make_shared<Acts::HomogeneousVolumeMaterial>(aMat);
        ATH_MSG_DEBUG("FINAL TRANSFORM " << GeoTrf::toString(transform));
        volumePtr volume = Acts::GeoModel::convertDetectorVolume(gctx->context(), *shape, name + "_" + std::to_string(passiveVolumes.size()), transform, {});
        volume->assignGeometryId(Acts::GeometryIdentifier{}.setVolume(30).setSensitive(passiveVolumes.size()));
        volume->assignVolumeMaterial(material);
        passiveVolumes.push_back(volume);
    }

    Acts::Experimental::DetectorComponent MuonDetectorBuilderTool::construct(const Acts::GeometryContext& context) const{
        ATH_MSG_DEBUG("Building Muon Detector Volume");
        const MuonChamberSet chambers = m_detMgr->getAllChambers();
        const ActsGeometryContext* gctx = context.get<const ActsGeometryContext* >();
        std::vector< volumePtr > detectorVolumeBoundingVolumes{};
        std::vector< volumePtr > detectorVolumePassiveVolumes{};

        detectorVolumeBoundingVolumes.reserve(chambers.size());
        std::vector<surfacePtr> surfaces = {};
        std::pair<std::vector<volumePtr>, std::vector<surfacePtr>> readoutElements; 

        auto portalGenerator = Acts::Experimental::defaultPortalAndSubPortalGenerator();
        unsigned int numChambers = chambers.size();

        for(const MuonGMR4::MuonChamber* chamber : chambers){
            unsigned int num = 0;
            //Gather the passives in each chamber
            getChamberPassives(gctx, chamber, detectorVolumePassiveVolumes);
            std::string chamberName = to_string(chamber->detectorType())+"_Name"+std::to_string(chamber->stationName())+"_Eta"+std::to_string(chamber->stationEta())+"_Phi"+std::to_string(chamber->stationPhi());
			std::shared_ptr<Acts::TrapezoidVolumeBounds> bounds = chamber->bounds();
			readoutElements = constructElements(*gctx, *chamber, std::make_pair(numChambers,num));
			volumePtr detectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(
                portalGenerator, gctx->context(), chamberName,
                                                  chamber->localToGlobalTrans(*gctx), 
                bounds, readoutElements.second, readoutElements.first, 
                Acts::Experimental::tryRootVolumes(), Acts::Experimental::tryAllPortalsAndSurfaces());

			detectorVolume->assignGeometryId(Acts::GeometryIdentifier{}.setLayer(numChambers--));
            if(m_dumpDetectorVolumes){
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

        ATH_MSG_VERBOSE("Number of detector volumes: "<< detectorVolumeBoundingVolumes.size());
        ATH_MSG_VERBOSE("Number of chamber passives volumes: "<< detectorVolumePassiveVolumes.size());
        std::vector<GeoChildNodeWithTrf> childNodes = getChildrenWithRef(m_detMgr->getTreeTop(0), false);
        std::vector<std::string> skipNodes{"MuonBarrel", "NSW", "MuonEndcap_sideA", "MuonEndcap_sideC","TGCSystem"};
        for (const GeoChildNodeWithTrf& node : childNodes){
            //These are taken care of in the chambers
            if(std::find(skipNodes.begin(), skipNodes.end(), node.nodeName) != skipNodes.end()){
                ATH_MSG_VERBOSE("Skipping volume "<<node.nodeName);
                continue;
            }
            ATH_MSG_VERBOSE("Processing passive node "<<node.nodeName);
            processPassiveNodes(gctx, node, node.nodeName, detectorVolumePassiveVolumes, node.transform);
        }
        ATH_MSG_VERBOSE("Number of total passive volumes: "<< detectorVolumePassiveVolumes.size());

        if (m_dumpPassive){
            ATH_MSG_VERBOSE("Writing passiveVolumes.obj");
            Acts::ObjVisualization3D helper;
            for (const auto& vol : detectorVolumePassiveVolumes){
                Acts::GeometryView3D::drawDetectorVolume(helper, *vol, gctx->context());
            }     
            helper.write("passiveVolumes.obj");
            helper.clear();
        }

        //Add the passive volumes to the end of detectorVolumeBoundingVolumes
        detectorVolumeBoundingVolumes.insert(detectorVolumeBoundingVolumes.end(), detectorVolumePassiveVolumes.begin(), detectorVolumePassiveVolumes.end());

        std::unique_ptr<Acts::CutoutCylinderVolumeBounds> msBounds = std::make_unique<Acts::CutoutCylinderVolumeBounds>(0, 4000, 14500, 22500, 3200);
        volumePtr msDetectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(
                    portalGenerator, gctx->context(), "Muon Spectrometer Envelope", 
                    Acts::Transform3::Identity(), std::move(msBounds), surfaces, 
                    detectorVolumeBoundingVolumes, Acts::Experimental::tryRootVolumes(), 
                    Acts::Experimental::tryAllPortalsAndSurfaces());
        msDetectorVolume->assignGeometryId(Acts::GeometryIdentifier{}.setVolume(15));

        if (m_dumpDetector) {
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
        detectorVolumeBoundingVolumes.push_back(msDetectorVolume);
        return Acts::Experimental::DetectorComponent{
        {detectorVolumeBoundingVolumes},
        portalContainer,
        {{msDetectorVolume}, Acts::Experimental::tryRootVolumes()}};
    }

std::pair<std::vector<volumePtr>,std::vector<surfacePtr>> MuonDetectorBuilderTool::constructElements(const ActsGeometryContext& gctx, const MuonGMR4::MuonChamber& mChamber, std::pair<unsigned int, unsigned int> chId) const{
    
	std::vector<volumePtr> readoutDetectorVolumes = {};
	std::vector<surfacePtr> readoutSurfaces = {}; 
	if(!m_buildSensitives) return std::make_pair(readoutDetectorVolumes, readoutSurfaces);
	std::pair<std::vector<volumePtr>, std::vector<surfacePtr>> pairElements;
	Acts::GeometryIdentifier::Value surfId{1};
	Acts::GeometryIdentifier::Value mdtId{1};

	MuonGMR4::MuonChamber::ReadoutSet readoutElements = mChamber.readOutElements();

	for(const MuonGMR4::MuonReadoutElement* ele : readoutElements){
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
       mlCfg.mlBinning = {Acts::Experimental::ProtoBinning(Acts::BinningValue::binY, Acts::AxisBoundaryType::Bound,                   
           -mdtBounds->values()[1], mdtBounds->values()[1], std::lround(2*mdtBounds->values()[1]/parameters.tubePitch), 0u), Acts::Experimental::ProtoBinning(Acts::BinningValue::binZ, Acts::AxisBoundaryType::Bound,                   
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
			ATH_MSG_FATAL("No detector type supported");
		}
	}

	pairElements = std::make_pair(readoutDetectorVolumes, readoutSurfaces);

	return pairElements;

	}

}
