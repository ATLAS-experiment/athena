/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtReadoutGeomTool.h"

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoTube.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <RDBAccessSvc/IRDBRecord.h>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#   include "Acts/Surfaces/LineBounds.hpp"
#endif

using namespace CxxUtils;
using namespace ActsTrk;

namespace MuonGMR4 {
using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;


StatusCode MdtReadoutGeomTool::loadDimensions(FactoryCache& facCache, 
                                              MdtReadoutElement::defineArgs& define) const {
    
    ATH_MSG_VERBOSE("Load dimensions of "<<m_idHelperSvc->toString(define.detElId)
                     <<std::endl<<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
    const GeoShape* shape = m_geoUtilTool->extractShape(define.physVol);
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }
    /// The trapezoid defines the length of the chamber including the extra material
    /// stemming from the faraday cache etc.
    if (shape->typeID() == GeoTrd::getClassTypeID()) {
        ATH_MSG_VERBOSE("Extracted shape "<<m_geoUtilTool->dumpShape(shape));
        const GeoTrd* trd = static_cast<const GeoTrd*>(shape);
        define.longHalfX = std::max(trd->getYHalfLength1(), trd->getYHalfLength2()) * Gaudi::Units::mm;
        define.shortHalfX = std::min(trd->getYHalfLength1(), trd->getYHalfLength2())* Gaudi::Units::mm;
        define.halfY = trd->getZHalfLength()* Gaudi::Units::mm;
        define.halfHeight = std::max(trd->getXHalfLength1(), trd->getXHalfLength2()) * Gaudi::Units::mm;
    } else {
        ATH_MSG_FATAL("Unknown shape type "<<shape->type());
        return StatusCode::FAILURE;
    }
    /// Loop over the child nodes of the full mdt tube layer volume to pick the ones representing the 
    /// tubeLayer -- their logical volume is callded TubeLayerLog. 
    /// The node right before the child volume is the associated transform node
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    for (unsigned int ch = 1; ch < define.physVol->getNChildNodes(); ++ch) {
        const GeoGraphNode* childNode = (*define.physVol->getChildNode(ch));
        const GeoVPhysVol* childVol = dynamic_cast<const GeoVPhysVol*>(childNode);
        if (!childVol  || childVol->getLogVol()->getName() != "TubeLayerLog") {
            continue;
        }
        const GeoTransform* trfNode = dynamic_cast<const GeoTransform*>(*define.physVol->getChildNode(ch-1));
        if (!trfNode) {
            ATH_MSG_FATAL("Expect a GeoTransform node right before the tubelayer node");
            return StatusCode::FAILURE;
        }
        ATH_MSG_VERBOSE("Add new tube layer "<<m_idHelperSvc->toStringDetEl(define.detElId)<<
                       std::endl<<std::endl<<m_geoUtilTool->dumpVolume(childVol));
        const Identifier tubeLayId = idHelper.channelID(define.detElId,
                                                        idHelper.multilayer(define.detElId),
                                                        define.tubeLayers.size() +1, 1);
        MdtTubeLayerPtr newLay = std::make_unique<MdtTubeLayer>(childVol, trfNode, facCache.cutTubes[tubeLayId]);
        define.tubeLayers.emplace_back(*facCache.tubeLayers.insert(newLay).first);

        const MdtTubeLayer& lay{*define.tubeLayers.back()};
        /// Next check all tubes whether they're made up out of air or not. If yes, then there's no tube at this place 
        /// and add the corresponding has hto the list.
        bool chEndPlug{false};
        for (unsigned int tube = 0 ; tube < lay.nTubes(); ++tube) {
            constexpr std::string_view airTubeName{"airTube"};
            PVConstLink tubeVol{lay.getTubeNode(tube)};
            if (tubeVol->getLogVol()->getName() == airTubeName) {
                define.removedTubes.insert(MdtReadoutElement::measurementHash(define.tubeLayers.size(), tube+1));
            } else if (!chEndPlug) {
                /// Check for the endplug volumes
                chEndPlug = true;
                std::vector<physVolWithTrans> endPlugs = m_geoUtilTool->findAllLeafNodesByName(tubeVol, "Endplug");
                if (endPlugs.empty()) {
                    /// Either all tubes have an endplug or none
                    continue;
                }
                const GeoShape* plugShape = m_geoUtilTool->extractShape(endPlugs[0].volume);
                if (plugShape->typeID() != GeoTube::getClassTypeID()){
                    ATH_MSG_FATAL("The shape "<<m_geoUtilTool->dumpShape(plugShape)<<" is not a tube");
                    return StatusCode::FAILURE;
                }
                const GeoTube* plugTube = static_cast<const GeoTube*>(plugShape);
                define.endPlugLength = plugTube->getZHalfLength();
            }
        }
    }
    define.readoutSide = facCache.readoutOnLeftSide.count(m_idHelperSvc->chamberId(define.detElId)) ? -1. : 1.;
    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoUtilTool.retrieve());
    GeoModelIO::ReadGeoModel* sqliteReader = m_geoDbTagSvc->getSqliteReader();
    if (!sqliteReader) {
        ATH_MSG_FATAL("Error, the tool works exclusively from sqlite geometry inputs");
        return StatusCode::FAILURE;
    }
    FactoryCache facCache{};
    ATH_CHECK(readParameterBook(facCache));
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    // Get the list of full phys volumes from SQLite, and create detector elements
    physNodeMap mapFPV = sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("Muon");
#ifndef SIMULATIONBASE
    auto boundFactory = std::make_shared<Acts::SurfaceBoundFactory>();   
#endif
    for (auto& [key, pv] : mapFPV) {
        /// The keys should be formatted like
        /// <STATION_NAME>_<MUON_CHAMBERTYPE>_etc. The <MUON_CHAMBERTYPE> also
        /// indicates whether we're dealing with a MDT / TGC / CSC / RPC chamber
        ///    If we are dealing with a MDT chamber, then there are 3 additional
        ///    properties encoded into the chamber
        ///       <STATIONETA>_<STATIONPHI>_ML
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() != 5 ||
            key_tokens[1].find("MDT") == std::string::npos)
            continue;

        MdtReadoutElement::defineArgs define{};
        bool isValid{false};
        define.detElId = idHelper.channelID(key_tokens[0].substr(0, 3), 
                                            atoi(key_tokens[2]),
                                            atoi(key_tokens[3]), 
                                            atoi(key_tokens[4]), 1, 1, isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to build a good identifier out of " << key);
            return StatusCode::FAILURE;
        }       
        ATH_MSG_DEBUG("Key "<<key<<" brought us "<<m_idHelperSvc->toStringDetEl(define.detElId));
        /// Skip the endcap chambers
        define.physVol = pv;
        define.chambDesign = key_tokens[1];
        define.alignTransform = m_geoUtilTool->findAlignableTransform(define.physVol);   
       
        /// Load first tube etc. from the parameter book table
        ParamBookTable::const_iterator book_itr = facCache.parBook.find(define.chambDesign);
        if (book_itr == facCache.parBook.end()) {
            ATH_MSG_FATAL("There is no chamber called "<<define.chambDesign);
            return StatusCode::FAILURE;
        }
        static_cast<parameterBook&>(define) = book_itr->second;
#ifndef SIMULATIONBASE        
        define.boundFactory = boundFactory;
 #endif       
        /// Chamber dimensions are given from the GeoShape
        ATH_CHECK(loadDimensions(facCache, define));
        std::unique_ptr<MdtReadoutElement> mdtDetectorElement = std::make_unique<MdtReadoutElement>(std::move(define));      
        ATH_CHECK(mgr.addMdtReadoutElement(std::move(mdtDetectorElement)));       
    }
    return StatusCode::SUCCESS;
}
StatusCode MdtReadoutGeomTool::readParameterBook(FactoryCache& cache) const {
    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(),
                                           name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("WMDT", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    for (const IRDBRecord_ptr& record : *paramTable) {
        parameterBook pars{};
        pars.tubeWall = record->getDouble("TUBWAL") * Gaudi::Units::cm;
        pars.tubePitch = record->getDouble("TUBPIT") * Gaudi::Units::cm;
        pars.tubeInnerRad = record->getDouble("TUBRAD") * Gaudi::Units::cm;
        pars.endPlugLength = record->getDouble("TUBDEA") * Gaudi::Units::cm;
        pars.radLengthX0 = record->getDouble("X0");
        unsigned int nLay = record->getInt("LAYMDT");
        const std::string key {record->getString("WMDT_TYPE")};
        ATH_MSG_DEBUG("Extracted parameters " <<pars<<" number of layers: "<<nLay<<" will be safed under key "<<key);
        cache.parBook[key] = std::move(pars);
    }
    /// Load the chamber that have their readout on the negative z-side
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    paramTable = accessSvc->getRecordsetPtr("MdtTubeROSides" ,"");
     if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    for (const IRDBRecord_ptr& record : *paramTable) {
        const std::string stName = record->getString("stationName");
        const int stEta = record->getInt("stationEta");
        const int stPhi = record->getInt("stationPhi");
        const int side = record->getInt("side");
        if (side == -1) {
            bool isValid{false};            
            cache.readoutOnLeftSide.insert(idHelper.elementID(stName,stEta,stPhi, isValid));
            if (!isValid) {
                ATH_MSG_FATAL("station "<<stName<<" eta: "<<stEta<<" phi: "<<stPhi<<" is unknown.");
                return StatusCode::FAILURE;
            }
        }
    }
    /** List of cut tubes */
     paramTable = accessSvc->getRecordsetPtr("MdtCutTubes" ,"");
     if (paramTable->size() == 0) {
        ATH_MSG_INFO("No information about cut mdt tubes has been found. Skipping.");
        return StatusCode::SUCCESS;
    }
     for (const IRDBRecord_ptr& record : *paramTable) {
        const std::string stName = record->getString("stationName");
        const int stEta = record->getInt("stationEta");
        const int stPhi = record->getInt("stationPhi");
        const int multiLay = record->getInt("multiLayer");
        const int tubeLay = record->getInt("tubeLayer");
        bool isValid{false};
        const Identifier tubeId = idHelper.channelID(stName, stEta, stPhi, multiLay, tubeLay, 1 , isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to deduce valid Idnetifier "<<stName<<", "<<stEta<<", "<<stPhi<<","<<multiLay<<", "<<tubeLay);
            return StatusCode::FAILURE;
        }
        FactoryCache::CutTubes cutTubes{};

        cutTubes.firstTube = record->getInt("firstTube");
        cutTubes.lastTube = record->getInt("lastTube");
        cutTubes.unCutHalfLength = record->getDouble("uncutHalfLength");
        ATH_MSG_VERBOSE("Found new uncut tube set in "<<m_idHelperSvc->toString(tubeId)<<" tubes: ["
                     <<cutTubes.firstTube<<"-"<<cutTubes.lastTube<<"], length: "<< 2.*cutTubes.unCutHalfLength );
        cache.cutTubes[tubeId].insert(std::move(cutTubes));

     }
    return StatusCode::SUCCESS;
}

}  // namespace MuonGMR4
