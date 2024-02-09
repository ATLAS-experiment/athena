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
StatusCode sTgcReadoutGeomTool::initialize() {
    ATH_CHECK(m_geoDbTagSvc.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoUtilTool.retrieve());
    return StatusCode::SUCCESS;
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
    if (shape->typeID() != GeoSimplePolygonBrep::getClassTypeID()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" expect shape to be a Trapezoid but it's "<<m_geoUtilTool->dumpShape(shape));
        return StatusCode::FAILURE;
    }
    /// Registering vertices of the Polygon
    const GeoSimplePolygonBrep* simplePoly = static_cast<const GeoSimplePolygonBrep*>(shape);
    std::vector<Amg::Vector2D> polyChamb = m_geoUtilTool->polygonEdges(*simplePoly);
    /// Building sTGC chamber
    if (polyChamb.size() == 4) {
        define.lHalfChamberLength = 0.5 * (polyChamb[0].x() - polyChamb[1].x()) * Gaudi::Units::mm;
        define.sHalfChamberLength = 0.5 * (polyChamb[3].x() - polyChamb[2].x()) * Gaudi::Units::mm;
        define.halfChamberTck = simplePoly->getDZ() * Gaudi::Units::mm;
        define.halfChamberHeight = 0.5 * (polyChamb[0].y() - polyChamb[3].y()) * Gaudi::Units::mm;
        define.yCutout = 0.;
    }
    else if (polyChamb.size() == 6) {
        define.lHalfChamberLength = 0.5 * (polyChamb[0].x() - polyChamb[1].x()) * Gaudi::Units::mm;
        define.sHalfChamberLength = 0.5 * (polyChamb[4].x() - polyChamb[3].x()) * Gaudi::Units::mm;
        define.halfChamberTck = simplePoly->getDZ() * Gaudi::Units::mm;
        define.halfChamberHeight = 0.5 * (polyChamb[0].y() - polyChamb[4].y()) * Gaudi::Units::mm;
        define.yCutout = (polyChamb[1].y() - polyChamb[2].y()) * Gaudi::Units::mm;
    } else {
        ATH_MSG_FATAL("Found unusual polygon with number of vertices:" << polyChamb.size());
        return StatusCode::FAILURE;
    }

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
        /// WireGroupDesign will join here
        StripDesignPtr stripDesign = std::make_unique<StripDesign>();
 
        const GeoShape* gapShape = m_geoUtilTool->extractShape(gapVol.physVol);
        if (gapShape->typeID() == GeoTrd::getClassTypeID()) {
            const GeoTrd* gapTrd = static_cast<const GeoTrd*>(gapShape);
            double halfHeight = gapTrd->getZHalfLength();
            double halfShortY = std::min(gapTrd->getYHalfLength1(), gapTrd->getYHalfLength2());
            double halfLongY = std::max(gapTrd->getYHalfLength1(), gapTrd->getYHalfLength2());
            double firstStripPos = -halfHeight + paramBook.firstStripPitch[gasGap];
            stripDesign->defineTrapezoid(halfShortY, halfLongY, halfHeight);
            stripDesign->defineStripLayout(Amg::Vector2D{firstStripPos, 0.},
                                           paramBook.stripPitch, paramBook.stripWidth, paramBook.numStrips);
            ATH_MSG_VERBOSE("Created new strip design "<<(*stripDesign));           
        } else if (gapShape->typeID() == GeoSimplePolygonBrep::getClassTypeID()) {
            /// Fetching edges of the diamond gasGap
            const GeoSimplePolygonBrep* gapPoly = static_cast<const GeoSimplePolygonBrep*>(gapShape);
            ATH_MSG_VERBOSE("Gas gap dimensions: "<<m_geoUtilTool->dumpShape(gapShape));
            std::vector<Amg::Vector2D> polyGap = m_geoUtilTool->polygonEdges(*gapPoly);
            double halfLongY = 0.5 * (polyGap[0].x() - polyGap[1].x()) * Gaudi::Units::mm;
            double halfShortY = 0.5 * (polyGap[4].x() - polyGap[3].x()) * Gaudi::Units::mm;
            double halfHeight = 0.5 * (polyGap[0].y() - polyGap[4].y()) * Gaudi::Units::mm;
            double yCut = (polyGap[1].y() - polyGap[2].y()) * Gaudi::Units::mm;
            double firstStripPos = -2*halfHeight + yCut + paramBook.firstStripPitch[gasGap];
            ///Need Diamond function
            stripDesign->defineStripLayout(Amg::Vector2D{firstStripPos, 0.},
                                           paramBook.stripPitch, 
                                           paramBook.stripWidth, 
                                           paramBook.numStrips);
            stripDesign->defineTrapezoid(halfShortY, halfLongY, halfHeight);
            ATH_MSG_VERBOSE("Added new strip design "<<(*stripDesign));           
        } else {
            ATH_MSG_FATAL("Failed to extract a geo shape");
            return StatusCode::FAILURE;
        }

        /// Strip rotation by 90 degrees (Not sure if required)
        gapVol.transform = gapVol.transform * Amg::getRotateY3D(-90. * Gaudi::Units::degree);
        ++gasGap;
        stripDesign = (*factoryCache.stripDesigns.emplace(stripDesign).first);
        StripLayer stripLayer(gapVol.transform, stripDesign, 
                              sTgcReadoutElement::createHash(gasGap, sTgcIdHelper::Strip, 0));
        ATH_MSG_VERBOSE("Added new strip layer at "<<stripLayer);
        define.stripLayers.push_back(std::move(stripLayer));
        if (!define.stripDesign) define.stripDesign = stripDesign;  
    }
    return StatusCode::SUCCESS;
}

StatusCode sTgcReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
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
        /// The keys should be formatted like UPDATE!!
        /// <STATION_NAME>_<MUON_CHAMBERTYPE>_etc. The <MUON_CHAMBERTYPE> also
        /// indicates whether we're dealing with a MDT / TGC / CSC / RPC chamber
        ///    If we are dealing with a MDT chamber, then there are 3 additional
        ///    properties encoded into the chamber
        ///       <STATIONETA>_(<STATIONPHI>-1)_ML
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
                        << " gasGapTck: " << parBook.gasTck);
    }
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4
