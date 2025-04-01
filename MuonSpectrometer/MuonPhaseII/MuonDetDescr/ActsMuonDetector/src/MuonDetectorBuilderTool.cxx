/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonDetectorBuilderTool.h"

#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <MuonReadoutGeometryR4/TgcReadoutElement.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
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
#include "Acts/Material/HomogeneousSurfaceMaterial.hpp"
#include "Acts/Material/MaterialSlab.hpp"
#include "Acts/Navigation/DetectorVolumeFinders.hpp"
#include "Acts/Navigation/InternalNavigation.hpp"
#include "Acts/Navigation/NavigationDelegates.hpp"
#include "Acts/Navigation/NavigationState.hpp"
#include "Acts/Visualization/ObjVisualization3D.hpp"
#include "Acts/Detector/MultiWireStructureBuilder.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Plugins/GeoModel/GeoModelMaterialConverter.hpp"
#include "Acts/Plugins/GeoModel/GeoModelToDetectorVolume.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/ConvexPolygonBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include <GeoModelKernel/GeoSimplePolygonBrep.h>
#include <GeoModelKernel/GeoShapeSubtraction.h>
#include <GeoModelKernel/GeoBox.h>
#include <GeoModelKernel/GeoTrd.h>
#include "GeoModelHelpers/getChildNodesWithTrf.h"
#include "ActsGeometryInterfaces/GeometryDefs.h"
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <set>
#include <climits>
#include <format>
#include <GeoModelHelpers/StringUtils.h>



using MuonChamberSet = MuonGMR4::MuonDetectorManager::MuonChamberSet;
using DetectorVolume = Acts::Experimental::DetectorVolume;
namespace ActsTrk {

    inline std::string objFileName(std::string str) {
        str = GeoStrUtils::replaceExpInString(str, " ", "_");
        str = GeoStrUtils::replaceExpInString(str, "-", "M");
        return std::format("{:}.obj", str);
    }

    using volumePtr= std::shared_ptr<DetectorVolume>;
    using surfacePtr = std::shared_ptr<Acts::Surface>;
    using StripLayerPtr = GeoModel::TransientConstSharedPtr<MuonGMR4::StripLayer>;

    MuonDetectorBuilderTool::MuonDetectorBuilderTool( const std::string& type, const std::string& name, const IInterface* parent ):
        base_class(type, name, parent){}

    StatusCode MuonDetectorBuilderTool::initialize() {
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_MSG_DEBUG("ACTS version is: v"<< Acts::VersionMajor << "." << Acts::VersionMinor << "." << Acts::VersionPatch << " [" << Acts::CommitHash << "]");

        return StatusCode::SUCCESS;
    }

    Acts::Experimental::DetectorComponent MuonDetectorBuilderTool::construct(const Acts::GeometryContext& context) const{
        ATH_MSG_DEBUG("Building Muon Detector Volume");
        const MuonChamberSet chambers = m_detMgr->getAllChambers();
        const ActsGeometryContext* gctx = context.get<const ActsGeometryContext* >();
        std::vector< volumePtr > detectorVolumeBoundingVolumes{};
        std::vector< volumePtr > detectorVolumePassiveVolumes{};

        detectorVolumeBoundingVolumes.reserve(chambers.size());
        std::vector<surfacePtr> surfaces{};
        std::vector<surfacePtr> materialSurfaces{};

        auto portalGenerator = Acts::Experimental::defaultPortalAndSubPortalGenerator();
        unsigned int numChambers = chambers.size();

        BlendedBoundSet materialBounds{};
        for(const MuonGMR4::Chamber* chamber : chambers){
            unsigned int num = 0;
            //Gather the passives in each chamber
            surfacePtr material{getChamberMaterial(*chamber, chamber->localToGlobalTrans(*gctx), materialSurfaces.size(), materialBounds)};
            std::shared_ptr<Acts::TrapezoidVolumeBounds> bounds = chamber->bounds();
            materialSurfaces.push_back(material);
            std::pair<std::vector<volumePtr>, std::vector<surfacePtr>> readoutElements = 
                constructElements(*gctx, *chamber, std::make_pair(numChambers,num));
            readoutElements.second.push_back(material);
            volumePtr detectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(portalGenerator, 
                                                                                            gctx->context(), 
                                                                                            chamber->identString(),
                                                                                            chamber->localToGlobalTrans(*gctx), 
                                                                                            bounds, readoutElements.second, 
                                                                                            readoutElements.first, 
                                                                                            Acts::Experimental::tryAllSubVolumes(), 
                                                                                            Acts::Experimental::tryAllPortalsAndSurfaces());

            detectorVolume->assignGeometryId(Acts::GeometryIdentifier{}.withLayer(numChambers--));
            if(m_dumpDetectorVolumes){
                //If we want to view each volume independently                
                Acts::ObjVisualization3D helper;
                Acts::GeometryView3D::drawDetectorVolume(helper, *detectorVolume, gctx->context());
                helper.write(objFileName(chamber->identString()));
                helper.clear();
            }
            detectorVolumeBoundingVolumes.push_back(std::move(detectorVolume));
            readoutElements.first.clear();
            readoutElements.second.clear();
        }

        if(m_dumpMaterialSurfaces){
            Acts::ObjVisualization3D helper;
            for (const auto& surf : materialSurfaces){
                Acts::GeometryView3D::drawSurface(helper, *surf, gctx->context());
            }
            helper.write("materialSurfaces.obj");
            helper.clear();
        }

        ATH_MSG_VERBOSE("Number of detector volumes: "<< detectorVolumeBoundingVolumes.size());
        ATH_MSG_VERBOSE("Number of chamber passives volumes: "<< detectorVolumePassiveVolumes.size());
        std::vector<GeoChildNodeWithTrf> childNodes = getChildrenWithRef(m_detMgr->getTreeTop(0), false);
        std::set<std::string> skipNodes{"MuonBarrel", "NSW", "MuonEndcap_sideA", "MuonEndcap_sideC","TGCSystem"};
        for (const GeoChildNodeWithTrf& node : childNodes){
            //These are taken care of in the chambers
            if(skipNodes.count(node.nodeName)) {
                ATH_MSG_VERBOSE("Skipping volume "<<node.nodeName);
                continue;
            }
            ATH_MSG_VERBOSE("Processing passive node "<<node.nodeName);
            // processPassiveNodes(*gctx, node, node.nodeName, detectorVolumePassiveVolumes, node.transform);
        }
        ATH_MSG_VERBOSE("Number of total passive volumes: "<< detectorVolumePassiveVolumes.size());

        if (m_dumpPassive){
            ATH_MSG_VERBOSE("Writing passiveVolumes.obj");
            Acts::ObjVisualization3D helper;
            for (const auto& vol : detectorVolumePassiveVolumes){
                Acts::GeometryView3D::drawDetectorVolume(helper, *vol, gctx->context());
            }     
            helper.write(objFileName("passiveVolumes"));
            helper.clear();
        }

        //Add the passive volumes to the end of detectorVolumeBoundingVolumes
        detectorVolumeBoundingVolumes.insert(detectorVolumeBoundingVolumes.end(), detectorVolumePassiveVolumes.begin(), detectorVolumePassiveVolumes.end());

        std::unique_ptr<Acts::CutoutCylinderVolumeBounds> msBounds = std::make_unique<Acts::CutoutCylinderVolumeBounds>(0, 4000, 14500, 22500, 3200);
        volumePtr msDetectorVolume = Acts::Experimental::DetectorVolumeFactory::construct(
                    portalGenerator, gctx->context(), "Muon Spectrometer Envelope", 
                    Acts::Transform3::Identity(), std::move(msBounds), surfaces, 
                    detectorVolumeBoundingVolumes, Acts::Experimental::tryAllSubVolumes(), 
                    Acts::Experimental::tryAllPortalsAndSurfaces());
        msDetectorVolume->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(15));

        if (m_dumpDetector) {
            ATH_MSG_VERBOSE("Writing detector.obj");
            Acts::ObjVisualization3D helper;
            Acts::GeometryView3D::drawDetectorVolume(helper, *msDetectorVolume, gctx->context());
            helper.write(objFileName("detector"));
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
        {{msDetectorVolume}, Acts::Experimental::tryAllSubVolumes()}};
    }

std::pair<std::vector<volumePtr>,std::vector<surfacePtr>> 
    MuonDetectorBuilderTool::constructElements(const ActsGeometryContext& gctx, 
                                               const MuonGMR4::Chamber& mChamber, 
                                               std::pair<unsigned int, unsigned int> chId) const{
    
    std::vector<volumePtr> readoutDetectorVolumes{};
    std::vector<surfacePtr> readoutSurfaces{}; 
    if(!m_buildSensitives) return std::make_pair(readoutDetectorVolumes, readoutSurfaces);
    Acts::GeometryIdentifier::Value surfId{1};
    Acts::GeometryIdentifier::Value mdtId{1};

    const MuonGMR4::Chamber::ReadoutSet& readoutElements = mChamber.readoutEles();

    for(const MuonGMR4::MuonReadoutElement* ele : readoutElements){
        switch (ele->detectorType()) {
            case DetectorType::Mdt: {    
                ATH_MSG_VERBOSE("Building MultiLayer for MDT Detector Volume");
                const auto* mdtReadoutEle = static_cast<const MuonGMR4::MdtReadoutElement*>(ele);
                const MuonGMR4::MdtReadoutElement::parameterBook& parameters{mdtReadoutEle->getParameters()};
                std::vector<surfacePtr> surfaces{};
                //loop over the tubes to get the surfaces  
                for(unsigned int lay=1; lay<=mdtReadoutEle->numLayers(); ++lay){
                    for(unsigned int tube=1; tube<=mdtReadoutEle->numTubesInLay(); ++tube){
                        const IdentifierHash measHash{mdtReadoutEle->measurementHash(lay,tube)};
                        if (!mdtReadoutEle->isValid(measHash)) continue;         
                        surfacePtr surface = mdtReadoutEle->surfacePtr(measHash);  
                        surface->assignGeometryId(Acts::GeometryIdentifier{}.withLayer(chId.first).withVolume(chId.second).withBoundary(mdtId).withSensitive(++surfId));
                        surfaces.push_back(surface);
                    }
                }
       
                //Get the transformation to the chamber's frame
                const Amg::Vector3D toChamber = mChamber.globalToLocalTrans(gctx)*mdtReadoutEle->center(gctx);
                const Acts::Transform3 mdtTransform = mChamber.localToGlobalTrans(gctx) * Amg::getTranslate3D(toChamber);       

                Acts::Experimental::MultiWireStructureBuilder::Config mlCfg{};  
                mlCfg.name = m_idHelperSvc->toStringDetEl(mdtReadoutEle->identify());
                mlCfg.mlSurfaces = std::move(surfaces);
                mlCfg.transform = mdtTransform;      
                auto mdtBounds = std::make_unique<Acts::TrapezoidVolumeBounds>(parameters.shortHalfX, parameters.longHalfX, parameters.halfY, parameters.halfHeight);
                using BoundsV = Acts::TrapezoidVolumeBounds::BoundValues;
                mlCfg.mlBounds= mdtBounds->values();              

                mlCfg.mlBinning = {{{Acts::AxisDirection::AxisY, Acts::AxisBoundaryType::Bound,                   
                                     -mdtBounds->get(BoundsV::eHalfLengthY), 
                                     mdtBounds->get(BoundsV::eHalfLengthY), 
                                     static_cast<std::size_t>(std::lround(2*mdtBounds->get(BoundsV::eHalfLengthY)/parameters.tubePitch))}, 2u}, 
                                   {{Acts::AxisDirection::AxisZ, Acts::AxisBoundaryType::Bound,                   
                                     -mdtBounds->get(BoundsV::eHalfLengthZ), 
                                     mdtBounds->get(BoundsV::eHalfLengthZ), 
                                     static_cast<std::size_t>(std::lround(2*mdtBounds->get(BoundsV::eHalfLengthZ)/parameters.tubePitch))}, 1u}};

                Acts::Experimental::MultiWireStructureBuilder mdtBuilder(mlCfg);
                volumePtr mdtVolume = mdtBuilder.construct(gctx.context()).volumes[0];
                mdtVolume->assignGeometryId(Acts::GeometryIdentifier{}.withLayer(chId.first).withVolume(chId.second).withBoundary(mdtId++));
                readoutDetectorVolumes.push_back(mdtVolume); 
                break;
            } case DetectorType::Rpc: 
              case DetectorType::Tgc:
              case DetectorType::sTgc:
              case DetectorType::Mm: {
                ATH_MSG_VERBOSE("Building plane surfaces "<<m_idHelperSvc->toStringDetEl(ele->identify()));
                std::vector<surfacePtr> detSurfaces = ele->getSurfaces();
                for (surfacePtr& surf : detSurfaces) {
                    surf->assignGeometryId(Acts::GeometryIdentifier{}.withLayer(chId.first).withVolume(chId.second).withSensitive(++surfId));
                    readoutSurfaces.push_back(std::move(surf));
                } 
                break;          
            } default:
                THROW_EXCEPTION("Unknown detector type "<<ActsTrk::to_string(ele->detectorType()));
        }
    }

    return std::make_pair(std::move(readoutDetectorVolumes), std::move(readoutSurfaces));

    }

    surfacePtr MuonDetectorBuilderTool::getChamberMaterial(const MuonGMR4::Chamber& chamber, 
                                                           const Amg::Transform3D& chamberTransform,
                                                           const int totalMaterials,
                                                           BlendedBoundSet& boundSet) const {
        std::shared_ptr<const Acts::PlanarBounds> bounds = boundSet.make_bounds(chamber.halfXShort(), chamber.halfXLong(), chamber.halfY());
        const float thickness = chamber.halfZ() * 2;
        PVConstLink parentVolume = chamber.readoutEles().front()->getMaterialGeom()->getParent();
        std::pair<MaterialPtr, double> geoMaterials = getMaterial(parentVolume);
        const Acts::Material aMat = Acts::GeoModel::geoMaterialConverter(*geoMaterials.first);
        //rotate about the z axis
        std::shared_ptr<Acts::PlaneSurface> surface = Acts::Surface::makeShared<Acts::PlaneSurface>(chamberTransform, bounds);
        Acts::MaterialSlab slab{aMat, thickness};
        std::shared_ptr<Acts::HomogeneousSurfaceMaterial> material = std::make_shared<Acts::HomogeneousSurfaceMaterial>(slab);
        surface->assignSurfaceMaterial(material);
        surface->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(29).withSensitive(totalMaterials));
        return surface;
    }

    std::pair<MuonDetectorBuilderTool::MaterialPtr, double> 
        MuonDetectorBuilderTool::getMaterial(const PVConstLink& vol) const {
        std::vector<std::pair<MaterialPtr, double>> materialContent{};
        getMaterialContent(vol, materialContent);

        //blend the material
        double totalVolume{0.};
        double totalMass{0.};
        for(const auto& [material, volume] : materialContent){
            totalVolume += volume;
            totalMass += volume * material->getDensity();
        }

        //create the new GeoMaterial object
        auto blendedMaterial = make_intrusive<GeoMaterial>("BlendedMaterial", totalMass/totalVolume);
        for(const auto& [material, volume] : materialContent){
            blendedMaterial->add(material, material->getDensity() * volume / totalMass);
        }
        blendedMaterial->lock();
        return {blendedMaterial, totalVolume};
    }

    void MuonDetectorBuilderTool::getMaterialContent(const PVConstLink& vol, std::vector<std::pair<MaterialPtr, double>>& materialContent) const {
        double volume{0.};
        if(!checkDummyMaterial(vol)) volume = vol->getLogVol()->getShape()->volume();
        for(std::size_t c=0; c < vol->getNChildVols(); ++c){
            const PVConstLink child = vol->getChildVol(c);
            double childVol = child->getLogVol()->getShape()->volume();
            //dummy material does not contribute
            if(!checkDummyMaterial(child)) volume -= childVol;
            getMaterialContent(child, materialContent);
        }
        //skip dummy materials
        if(!checkDummyMaterial(vol)) materialContent.emplace_back(vol->getLogVol()->getMaterial(), volume);
    }

    bool MuonDetectorBuilderTool::checkDummyMaterial(const PVConstLink& vol) const {
       static const std::unordered_set<std::string> dummyMaterials{
            "special::Ether", 
            "WorldLog::Air", 
            "std::Air", 
            "Air"
        };
       const std::string materialName = vol->getLogVol()->getMaterial()->getName();
       return dummyMaterials.find(materialName) != dummyMaterials.end();
    }

    void MuonDetectorBuilderTool::processPassiveNodes(const ActsGeometryContext& gctx, 
                                                      const GeoChildNodeWithTrf& node, 
                                                      const std::string& name, 
                                                      std::vector<volumePtr>& passiveVolumes, 
                                                      const GeoTrf::Transform3D& transform) const {
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
        const GeoMaterial* geoMaterial = node.volume->getLogVol()->getMaterial();
        const Acts::Material aMat = Acts::GeoModel::geoMaterialConverter(*geoMaterial);
        std::shared_ptr<Acts::HomogeneousVolumeMaterial> material = std::make_shared<Acts::HomogeneousVolumeMaterial>(aMat);
        ATH_MSG_DEBUG("FINAL TRANSFORM " << GeoTrf::toString(transform));
        volumePtr volume = Acts::GeoModel::convertDetectorVolume(gctx.context(), *shape, name + "_" + std::to_string(passiveVolumes.size()), transform, {});
        volume->assignGeometryId(Acts::GeometryIdentifier{}.withVolume(30).withSensitive(passiveVolumes.size()));
        volume->assignVolumeMaterial(material);
        passiveVolumes.push_back(volume);
    }

}
