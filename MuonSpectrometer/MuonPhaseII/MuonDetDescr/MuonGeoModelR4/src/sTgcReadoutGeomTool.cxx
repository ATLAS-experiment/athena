/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcReadoutGeomTool.h"

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoSimplePolygonBrep.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>

#include <RDBAccessSvc/IRDBRecord.h>

#ifndef SIMULATIONBASE
#   include "Acts/Utilities/BoundFactory.hpp"
#endif

using namespace CxxUtils;
using namespace ActsTrk;

namespace MuonGMR4 {

using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;
using defineArgs = sTgcReadoutElement::defineArgs;


sTgcReadoutGeomTool::sTgcShape sTgcReadoutGeomTool::extractParameters(const GeoShape* shape) const {
    sTgcShape result{};
    if (shape->typeID() == GeoTrd::getClassTypeID()) {
        const GeoTrd* trd = static_cast<const GeoTrd*>(shape);
        result.longWidth = trd->getXHalfLength1();
        result.shortWidth =  trd->getXHalfLength2();      
        result.halfHeight =  trd->getYHalfLength1();
        result.thickness =  trd->getZHalfLength();
    } else if (shape->typeID() == GeoSimplePolygonBrep::getClassTypeID()) {
        const GeoSimplePolygonBrep* poly = static_cast<const GeoSimplePolygonBrep*>(shape);
        std::vector<Amg::Vector2D> polyEdges = m_geoUtilTool->polygonEdges(*poly);
        result.thickness = poly->getDZ() * Gaudi::Units::mm;
        if (polyEdges.size() == 4) {
            result.longWidth = 0.5 * (polyEdges[0].x() - polyEdges[1].x()) * Gaudi::Units::mm;
            result.shortWidth = 0.5 * (polyEdges[3].x() - polyEdges[2].x()) * Gaudi::Units::mm;            
            result.halfHeight = 0.5 * (polyEdges[0].y() - polyEdges[3].y()) * Gaudi::Units::mm;
        } else if (polyEdges.size() == 6) {
            result.longWidth = 0.5 * (polyEdges[0].x() - polyEdges[1].x()) * Gaudi::Units::mm;
            result.shortWidth = 0.5 * (polyEdges[4].x() - polyEdges[3].x()) * Gaudi::Units::mm;
            result.halfHeight = 0.5 * (polyEdges[0].y() - polyEdges[4].y()) * Gaudi::Units::mm;
            result.yCutOut = (polyEdges[1].y() - polyEdges[2].y()) * Gaudi::Units::mm;
        }
    } else {
        ATH_MSG_FATAL("Unknown shape type "<<shape->type());
        throw std::runtime_error("Invalid shape to extract sTGC parameters");
    }
    return result;
}
    
StatusCode sTgcReadoutGeomTool::loadDimensions(sTgcReadoutElement::defineArgs& define, FactoryCache& factoryCache) {
    ATH_MSG_VERBOSE("Load dimensions of "<<m_idHelperSvc->toString(define.detElId)
                     <<std::endl<<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
    const GeoShape* shape = m_geoUtilTool->extractShape(define.physVol);
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Extracted shape "<<m_geoUtilTool->dumpShape(shape));
    /// The half sizes of the 
    sTgcShape modPars = extractParameters(shape);

    define.sHalfChamberLength = modPars.shortWidth;
    define.lHalfChamberLength = modPars.longWidth;
    define.halfChamberHeight = modPars.halfHeight;
    define.halfChamberTck = modPars.thickness;
    define.yCutout = modPars.yCutOut;
    
    ATH_MSG_DEBUG("chamber length (L/S) is: " << 2*define.lHalfChamberLength << "/" 
                 << 2*define.sHalfChamberLength << " chamber height is: " 
                 << 2*define.halfChamberHeight << " chamber thickness is: " << 2*define.halfChamberTck << " yCutout value is: " << define.yCutout);

    /// Navigate through the GeoModel tree to find all gas volume leaves
    std::vector<physVolWithTrans> allGasGaps = m_geoUtilTool->findAllLeafNodesByName(define.physVol, "sTgcGas");
    if (allGasGaps.empty()) {
        ATH_MSG_FATAL("The volume "<<m_idHelperSvc->toStringDetEl(define.detElId)<<" does not have any children 'sTgcGas'"
                     <<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
        return StatusCode::FAILURE;
    }

    /// Filling in number of layers
    define.numLayers = allGasGaps.size();
    ATH_MSG_VERBOSE("The number of gasGaps are: " << define.numLayers);
    FactoryCache::ParamBookTable::const_iterator parBookItr = factoryCache.parameterBook.find(define.chambDesign);
    if (parBookItr == factoryCache.parameterBook.end()) {
        ATH_MSG_FATAL("The chamber "<<define.chambDesign<<" is not part of the WSTGC table");
        return StatusCode::FAILURE;
    }    

    const wSTGCTable& paramBook{parBookItr->second};

    define.gasTck = paramBook.gasTck;
    /// Gasgaps are trapezoid
    unsigned int gasGap{0};
    for (physVolWithTrans& gapVol : allGasGaps) {
        StripDesignPtr stripDesign = std::make_unique<StripDesign>();
        WireDesignPtr wireGroupDesign = std::make_unique<WireGroupDesign>();
        PadDesignPtr padDesign = std::make_unique<PadDesign>();

        sTgcShape gapPars = extractParameters(m_geoUtilTool->extractShape(gapVol.volume));

        /// Strip Parameters
        double firstStripPos = -gapPars.halfHeight + paramBook.firstStripPitch[gasGap] - 0.5 * paramBook.stripPitch;
        define.firstStripPitch = paramBook.firstStripPitch;
        ATH_MSG_DEBUG("FirstStripPos for stripPitch: " << paramBook.firstStripPitch[gasGap] << " is: " << Amg::toString(firstStripPos, 2) << " and the half height is: " << gapPars.halfHeight);
        
        /// wireGroup Parameters
        unsigned int numWireGroups = paramBook.numWireGroups[gasGap];
        /// firstWirePos locates the x-coordinate of the beginning of first WireGroup
        double firstWirePos = paramBook.firstWirePos[gasGap];

        if(gapPars.yCutOut) {
            /// Diamond Strip Design       
            stripDesign->defineDiamond(gapPars.shortWidth, gapPars.longWidth, gapPars.halfHeight + 0.001, paramBook.yCutoutCathode);
            ATH_MSG_VERBOSE("The yCutout of the active area is: " << gapPars.yCutOut);
            stripDesign->defineStripLayout(Amg::Vector2D{firstStripPos, 0.},
                                        paramBook.stripPitch, paramBook.stripWidth, paramBook.numStrips);
            ATH_MSG_VERBOSE("Created new diamond strip design "<<(*stripDesign));
            /// Diamond WireGroup Design
            wireGroupDesign->defineDiamond(0.5*paramBook.sPadLength, 0.5*paramBook.lPadLength, gapPars.halfHeight, paramBook.yCutoutCathode);
            wireGroupDesign->flipTrapezoid();
            wireGroupDesign->defineStripLayout(Amg::Vector2D{firstWirePos, 0.}, 
                                            paramBook.wirePitch, 
                                            paramBook.wireWidth, 
                                            numWireGroups);
            ATH_MSG_VERBOSE("Created new diamond wireGroup design "<<(*wireGroupDesign));

            /// Diamond Pad Design
            padDesign->defineDiamond(0.5*paramBook.sPadLength, 0.5*paramBook.lPadLength, gapPars.halfHeight, paramBook.yCutoutCathode);
            padDesign->flipTrapezoid();
            ATH_MSG_VERBOSE("Created new diamond pad design "<<(*padDesign));
        } else {
            /// Trapezoid Strip Design
            stripDesign->defineTrapezoid(gapPars.shortWidth, gapPars.longWidth, gapPars.halfHeight + 0.01);
            stripDesign->defineStripLayout(Amg::Vector2D{firstStripPos, 0.},
                                        paramBook.stripPitch, paramBook.stripWidth, paramBook.numStrips);
            ATH_MSG_VERBOSE("Created new strip design "<<(*stripDesign));

            /// Trapezoid WireGroup Design
            wireGroupDesign->defineTrapezoid(0.5*paramBook.sPadLength, 0.5*paramBook.lPadLength, gapPars.halfHeight);
            wireGroupDesign->flipTrapezoid();        
            wireGroupDesign->defineStripLayout(Amg::Vector2D{firstWirePos, 0.}, 
                                            paramBook.wirePitch, 
                                            paramBook.wireWidth, 
                                            numWireGroups);    
            ATH_MSG_VERBOSE("Created new wireGroup design "<<(*wireGroupDesign));

            /// Trapezoid Pad Design
            padDesign->defineTrapezoid(0.5*paramBook.sPadLength, 0.5*paramBook.lPadLength, gapPars.halfHeight);
            padDesign->flipTrapezoid();
            ATH_MSG_VERBOSE("Created new pad design "<<(*padDesign));
        }
       
        /// Placing wires in the designated wireGroups for easy retrieval later, first and last are placed separately.
        wireGroupDesign->declareGroup(paramBook.firstWireGroupWidth[gasGap]);
        for (uint wireGr=2; wireGr<numWireGroups; wireGr++){
            wireGroupDesign->declareGroup(paramBook.wireGroupWidth);
        }
        unsigned int lastWireGroup = (paramBook.numWires[gasGap] - wireGroupDesign->nAllWires());
        wireGroupDesign->declareGroup(lastWireGroup);
        wireGroupDesign->defineWireCutout(paramBook.wireCutout[gasGap]);
        
        /// Defining the beamline radius, pad columns and pad rows            
        double beamlineRadius = (define.physVol->getAbsoluteTransform() * gapVol.transform).translation().perp();
        padDesign->defineBeamlineRadius(beamlineRadius);
        ATH_MSG_DEBUG("The beamline radius is: " << beamlineRadius);
        padDesign->definePadRow(paramBook.firstPadPhiDivision[gasGap],
                                paramBook.numPadPhi[gasGap],
                                paramBook.anglePadPhi,
                                paramBook.PadPhiShift[gasGap]);
        padDesign->definePadColumn(paramBook.firstPadHeight[gasGap],
                                    paramBook.numPadEta[gasGap],
                                    paramBook.padHeight[gasGap]);

        /// Stacking strip, wireGroup and pad layers
        /// Redefining the gasgap center for L3 (diamond) sector: Unlike the run 3 (legacy) code,
        /// we are NOT offsetting the gasgap center in the local y coordinate to match the base 
        /// of the cutout. As a result, localToGlobal transformations and the local y coordinate 
        /// of the pads, strips and wireGroups will not match the run 3. Additionally, the 
        /// global y coordinate of the wireGroup is shifted by ~25 mm due to this center redefinition.
        /// The redefinition will not affect reconstruction and digitization, and will simplify the 
        /// sTgc geometry code.
        ++gasGap;
        /// StripLayer
        stripDesign = (*factoryCache.stripDesigns.emplace(stripDesign).first);
        wireGroupDesign = (*factoryCache.wireGroupDesigns.emplace(wireGroupDesign).first);
        StripLayer stripLayer{factoryCache.trfNodeMaker.makeTransform(gapVol.transform
                                                * Amg::getRotateZ3D(-90. * Gaudi::Units::deg) 
                                                * Amg::getRotateY3D(180* Gaudi::Units::deg)), 
                                                stripDesign, wireGroupDesign,
                                                sTgcReadoutElement::createHash(gasGap, sTgcIdHelper::Strip, 0)};
        stripLayer.flipPhiRotation();
        ATH_MSG_VERBOSE("Added new diamond strip layer at "<< stripLayer);
        define.stripLayers.push_back(std::move(stripLayer));

        /// Pad Layer
        padDesign = (*factoryCache.padDesigns.emplace(padDesign).first);
        StripLayer padLayer(factoryCache.trfNodeMaker.makeTransform(gapVol.transform 
                                              * Amg::getRotateZ3D(-90. * Gaudi::Units::deg) 
                                              * Amg::getRotateY3D(180* Gaudi::Units::deg)), 
                                                stripDesign, padDesign, sTgcReadoutElement::createHash(gasGap, sTgcIdHelper::Pad, 0));
        padLayer.flipPhiRotation();
        ATH_MSG_VERBOSE("Added new diamond pad layer at "<<padLayer);
        define.padLayers.push_back(std::move(padLayer));
    }
    return StatusCode::SUCCESS;
}

StatusCode sTgcReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
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

    const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
    // Get the list of full phys volumes from SQLite, and create detector elements
    
    physNodeMap mapFPV = sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("Muon");
#ifndef SIMULATIONBASE
    auto layerBounds = std::make_shared<Acts::SurfaceBoundFactory>();
#endif

    for (auto& [key, pv] : mapFPV) {
        /// Key formatted as follows for sTGC:
        /// <sTGC>_<L/S + MODULE TYPE>_<QUADRUPLET NUMBER>_<ETA INDEX>_<PHI INDEX>
        /// e.g. sTGC_STL1QL2_1_6_1 .
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() != 5 ||
            key_tokens[0].find("sTGC") == std::string::npos)
            continue;
        ATH_MSG_DEBUG("Key is: "<<key);
        bool isValid{false};
        const std::string stName = key_tokens[1][1] == 'L' ? "STL" : "STS";
        const int stEta = atoi(key_tokens[2]);
        const int stPhi = atoi(key_tokens[3]);
        const int ml = atoi(key_tokens[4]);

        defineArgs define{};        
#ifndef SIMULATIONBASE
        define.layerBounds = layerBounds;
#endif
        //Need identifier to get multilayer, the following constants don't matter.
        define.detElId = idHelper.channelID(stName, stEta, stPhi, ml, 1, sTgcIdHelper::sTgcChannelTypes::Strip, 1, isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to build a good identifier out of " << key);
            return StatusCode::FAILURE;
        }
        /// Skip the endcap chambers
        define.physVol = pv;
        define.chambDesign = "sTGC_" + key_tokens[1];
        define.alignTransform = m_geoUtilTool->findAlignableTransform(define.physVol);           
        ATH_MSG_VERBOSE("Key "<<key<<" lead to the identifier "<<m_idHelperSvc->toStringDetEl(define.detElId));  
        ATH_CHECK(loadDimensions(define, facCache));
        std::unique_ptr<sTgcReadoutElement> readoutEle = std::make_unique<sTgcReadoutElement>(std::move(define));
        ATH_CHECK(mgr.addsTgcReadoutElement(std::move(readoutEle)));
    }
    return StatusCode::SUCCESS;
}
StatusCode sTgcReadoutGeomTool::readParameterBook(FactoryCache& cache) {
    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(), name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("WSTGC", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    for (const IRDBRecord_ptr& record : *paramTable) {
        // parameterBook pars{};
        const std::string key = record-> getString("WSTGC_TYPE");
        wSTGCTable& parBook = cache.parameterBook[key];
        parBook.numStrips = record->getInt("nStrips");
        parBook.stripPitch = record->getDouble("stripPitch");
        parBook.stripWidth = record->getDouble("stripWidth");
        parBook.firstStripPitch = tokenizeDouble(record->getString("firstStripWidth"), ";");

        parBook.numWires = tokenizeInt(record->getString("nWires"), ";");
        parBook.firstWireGroupWidth = tokenizeInt(record->getString("firstWireGroup"), ";");
        parBook.numWireGroups = tokenizeInt(record->getString("nWireGroups"), ";");
        parBook.wireCutout = tokenizeDouble(record->getString("wireCutout"), ";");
        parBook.wirePitch = record->getDouble("wirePitch");
        parBook.wireWidth = record->getDouble("wireWidth");
        parBook.wireGroupWidth = record->getInt("wireGroupWidth");
        parBook.firstWirePos = tokenizeDouble(record->getString("firstWire"), ";");

        parBook.numPadEta = tokenizeInt(record->getString("nPadH"), ";");
        parBook.numPadPhi = tokenizeInt(record->getString("nPadPhi"), ";");
        parBook.firstPadHeight = tokenizeDouble(record->getString("firstPadH"), ";");
        parBook.padHeight = tokenizeDouble(record->getString("padH"), ";");
        parBook.PadPhiShift = tokenizeInt(record->getString("PadPhiShift"), ";");
        parBook.anglePadPhi = record->getDouble("anglePadPhi");
        parBook.firstPadPhiDivision = tokenizeDouble(record->getString("firstPadPhiDivision"), ";");
        parBook.lPadLength = record->getDouble("lPadWidth");
        parBook.sPadLength = record->getDouble("sPadWidth");

        parBook.yCutout = record->getDouble("yCutout");
        parBook.yCutoutCathode = record->getDouble("yCutoutCathode");
        parBook.gasTck = record->getDouble("gasTck");
        parBook.lFrameWidth = record->getDouble("ylFrame");
        parBook.sFrameWidth = record->getDouble("ysFrame");

        ATH_MSG_DEBUG("Parameters of the chamber " << key << " are: "
                        << " numStrips: " << parBook.numStrips
                        << " stripPitch: " << parBook.stripPitch
                        << " stripWidth: " << parBook.stripWidth
                        << " FirstStripPitch: "<< parBook.firstStripPitch
                        << " numWires: " << parBook.numWires
                        << " firstWireGroupWidth: " << parBook.firstWireGroupWidth
                        << " numWireGroups: " << parBook.numWireGroups
                        << " wireCutout: " << parBook.wireCutout
                        << " wirePitch: " << parBook.wirePitch
                        << " wireWidth: " << parBook.wireWidth
                        << " wireGroupWidth: " << parBook.wireGroupWidth 
                        << " firstWirePosition: " << parBook.firstWirePos
                        << " Pads in Eta: " << parBook.numPadEta
                        << " Pads in Phi: " << parBook.numPadPhi
                        << " firstPadHeight: " << parBook.firstPadHeight
                        << " padHeight: " << parBook.padHeight
                        << " PadPhiShift: " << parBook.PadPhiShift
                        << " anglePadPhi: " << parBook.anglePadPhi
                        << " firstPadPhiDivision: " << parBook.firstPadPhiDivision
                        << " lPadLength: " << parBook.lPadLength
                        << " sPadLength: " << parBook.sPadLength
                        << " yCutout: " << parBook.yCutout
                        << " yCutoutCathode: " << parBook.yCutoutCathode
                        << " gasGapTck: " << parBook.gasTck
                        << " lFrameWidth: " << parBook.lFrameWidth
                        << " sFrameWidth: " << parBook.sFrameWidth);
    }
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4
