/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonGeoModelR4/sTgcReadoutGeomTool.h>

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoSimplePolygonBrep.h>
#include<CxxUtils/bitscan.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/sTgcReadoutElement.h>

#include <RDBAccessSvc/IRDBRecord.h>

using namespace CxxUtils;
using namespace ActsTrk;

namespace MuonGMR4 {

using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;
using defineArgs = sTgcReadoutElement::defineArgs;


sTgcReadoutGeomTool::sTgcReadoutGeomTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : AthAlgTool{type, name, parent} {
    declareInterface<IMuonReadoutGeomTool>(this);
}

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
    
    ATH_MSG_VERBOSE("chamber length (L/S) is: " << 2*define.lHalfChamberLength << "/" 
                 << 2*define.sHalfChamberLength << " chamber height is: " 
                 << 2*define.halfChamberHeight << " chamber thickness is: " << 2*define.halfChamberTck);

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

        sTgcShape gapPars = extractParameters(m_geoUtilTool->extractShape(gapVol.volume));

        if (true || !gapPars.yCutOut) {
            //StripDesign
            double firstStripPos = -gapPars.halfHeight + paramBook.firstStripPitch[gasGap] - 0.5 * paramBook.stripPitch;
            define.firstStripPitch = paramBook.firstStripPitch;
            ATH_MSG_DEBUG("FirstStripPos is: " << Amg::toString(firstStripPos, 2) << " and the half height is: " << gapPars.halfHeight);
            stripDesign->defineTrapezoid(gapPars.shortWidth, gapPars.longWidth, gapPars.halfHeight);
            stripDesign->defineStripLayout(Amg::Vector2D{firstStripPos, 0.},
                                           paramBook.stripPitch, paramBook.stripWidth, paramBook.numStrips);
            ATH_MSG_VERBOSE("Created new strip design "<<(*stripDesign));

            //WireGroupDesign
            wireGroupDesign->defineTrapezoid(gapPars.shortWidth, gapPars.longWidth, gapPars.halfHeight);
            wireGroupDesign->flipTrapezoid();
            unsigned int numWireGroups = paramBook.numWireGroups[gasGap];
            /// Placing wires in the designated wireGroups for easy retrieval later, first and last are placed separately.
            wireGroupDesign->declareGroup(paramBook.firstWireGroupWidth[gasGap]);
            for (uint wireGr=2; wireGr<numWireGroups; wireGr++){
                wireGroupDesign->declareGroup(paramBook.wireGroupWidth);
            }
            unsigned int lastWireGroup = (paramBook.numWires[gasGap] - wireGroupDesign->nAllWires());
            wireGroupDesign->declareGroup(lastWireGroup);
            /// Defining the wire group layout
            /// firstWirePos locates the y-coordinate of the beginning of first WireGroup
            double firstWirePos = paramBook.firstWirePos[gasGap];
            wireGroupDesign->defineStripLayout(Amg::Vector2D{firstWirePos, 0.}, 
                                                paramBook.wirePitch, 
                                                paramBook.wireWidth, 
                                                numWireGroups);

            wireGroupDesign->defineWireCutout(paramBook.wireCutout[gasGap]);
        }
        /// Stacking strip and wireGroup layers
        ++gasGap;        
        wireGroupDesign = (*factoryCache.wireGroupDesigns.emplace(wireGroupDesign).first);
        StripLayer wireGroupLayer(gapVol.transform * Amg::getRotateY3D(180* Gaudi::Units::deg), wireGroupDesign, sTgcReadoutElement::createHash(gasGap, sTgcIdHelper::Wire, 0));
        ATH_MSG_VERBOSE("Added new wireGroup layer at "<<wireGroupLayer);
        define.wireGroupLayers.push_back(std::move(wireGroupLayer));
        if(!define.wireGroupDesign) define.wireGroupDesign = wireGroupDesign;
        stripDesign = (*factoryCache.stripDesigns.emplace(stripDesign).first);
        StripLayer stripLayer(gapVol.transform * Amg::getRotateZ3D(-90. * Gaudi::Units::deg) * 
                                                 Amg::getRotateY3D(180* Gaudi::Units::deg), stripDesign, 
                                                 sTgcReadoutElement::createHash(gasGap, sTgcIdHelper::Strip, 0));
        ATH_MSG_VERBOSE("Added new strip layer at "<< stripLayer);
        define.stripLayers.push_back(std::move(stripLayer));
        if (!define.stripDesign) define.stripDesign = stripDesign;  
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
    SurfaceBoundSetPtr<Acts::TrapezoidBounds> layerBounds = std::make_shared<SurfaceBoundSet<Acts::TrapezoidBounds>>();
#endif

    for (auto& [key, pv] : mapFPV) {
        /// Key formatted as follows for sTGC:
        /// <sTGC>_<L/S + MODULE TYPE>_<QUADRUPLET NUMBER>_<ETA INDEX>_<PHI INDEX + 1>
        /// e.g. sTGC_STL1QL2_1_6_1 .
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() != 5 ||
            key_tokens[0].find("sTGC") == std::string::npos)
            continue;
        ATH_MSG_DEBUG("Key is: "<<key);
        bool isValid{false};
        const std::string stName = key_tokens[1][1] == 'L' ? "STL" : "STS";
        const int stEta = atoi(key_tokens[2]);
        const int stPhi = atoi(key_tokens[3]) + 1;
        const int ml = atoi(key_tokens[4]);
        /// Uncomment to avoid dumping diamonds
        //if (stName == "STL" && std::abs(stEta) == 3) continue; 
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
    for (const IRDBRecord* record : *paramTable) {
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
        ///Extra Pad Variables
        parBook.PadPhiShift_A = tokenizeInt(record->getString("PadPhiShift_A"), ";");
        parBook.PadPhiShift_C = tokenizeInt(record->getString("PadPhiShift_C"), ";");
        parBook.anglePadPhi = record->getDouble("anglePadPhi");
        parBook.firstPadPhiDivision_A = tokenizeDouble(record->getString("firstPadPhiDivision_A"), ";");
        parBook.firstPadPhiDivision_C = tokenizeDouble(record->getString("firstPadPhiDivision_C"), ";");
        parBook.firstPadRow = tokenizeInt(record->getString("firstPadRow"), ";");
        parBook.lPadWidth = record->getDouble("lPadWidth");
        parBook.rankPadEta = tokenizeInt(record->getString("rankPadH"), ";");
        parBook.rankPadPhi = tokenizeInt(record->getString("rankPadPhi"), ";");
        parBook.sPadWidth = record->getDouble("sPadWidth");

        parBook.gasTck = record->getDouble("gasTck");

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
                        ///ExtraPadVariables
                        << " PadPhiShift_A: " << parBook.PadPhiShift_A
                        << " PadPhiShift_C: " << parBook.PadPhiShift_C
                        << " anglePadPhi: " << parBook.anglePadPhi
                        << " firstPadPhiDivision_A: " << parBook.firstPadPhiDivision_A
                        << " firstPadPhiDivision_C: " << parBook.firstPadPhiDivision_C
                        << " firstPadRow: " << parBook.firstPadRow
                        << " lPadWidth: " << parBook.lPadWidth
                        << " rankPadEta: " << parBook.rankPadEta
                        << " rankPadPhi: " << parBook.rankPadPhi
                        << " sPadWidth: " << parBook.sPadWidth

                        << " gasGapTck: " << parBook.gasTck);
    }
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4
