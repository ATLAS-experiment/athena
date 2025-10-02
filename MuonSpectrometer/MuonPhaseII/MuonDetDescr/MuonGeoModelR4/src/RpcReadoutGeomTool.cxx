/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcReadoutGeomTool.h"

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>

#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoBox.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <RDBAccessSvc/IRDBRecord.h>

#ifndef SIMULATIONBASE
#   include "Acts/Utilities/BoundFactory.hpp"
#endif

using namespace CxxUtils;
using namespace ActsTrk;

namespace MuonGMR4 {

using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;
using defineArgs = RpcReadoutElement::defineArgs;

/// Helper struct to attribute the Identifier fields with the
/// gas gap volumes
struct gapVolume: public GeoChildNodeWithTrf {
    gapVolume(GeoChildNodeWithTrf&& physVol,
              unsigned gap,
              unsigned phi):
              GeoChildNodeWithTrf{std::move(physVol)},
        gasGap{gap},
        doubPhi{phi} {}
    unsigned gasGap{0};
    unsigned doubPhi{0};
    
};


std::unique_ptr<StripDesign> RpcReadoutGeomTool::constructDesign(const GeoBox* planeBox,
                                                                 const wRPCTable& paramBook,
                                                                 bool phiPlane) const {
    const unsigned nStrips = phiPlane ? paramBook.numPhiStrips : paramBook.numEtaStrips;
    if (!nStrips) {
        ATH_MSG_VERBOSE("Parameter book does not for see a readout in "<<(phiPlane? "phi" : "eta")<<" direction");
        return nullptr;
    }

    const double halfX = phiPlane ? planeBox->getYHalfLength() : planeBox->getZHalfLength();
    const double halfY = phiPlane ? planeBox->getZHalfLength() : planeBox->getYHalfLength();

    const double pitch = phiPlane ? paramBook.stripPitchPhi : paramBook.stripPitchEta;
    const double width = phiPlane ? paramBook.stripWidthPhi : paramBook.stripWidthEta;
    const double firstPos = -halfX + (phiPlane ? paramBook.firstOffSetPhi : paramBook.firstOffSetEta);
    auto newDesign = std::make_unique<StripDesign>();
    newDesign->defineTrapezoid(halfY, halfY, halfX);
    newDesign->defineStripLayout(firstPos * Amg::Vector2D::UnitX(), pitch,width, nStrips); 
    return newDesign;
}

StatusCode RpcReadoutGeomTool::loadDimensions(RpcReadoutElement::defineArgs& define,
                                              FactoryCache& factoryCache) {    
    
    ATH_MSG_VERBOSE("Load dimensions of "<<m_idHelperSvc->toString(define.detElId)
                     <<std::endl<<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
    
    const GeoShape* shape = m_geoUtilTool->extractShape(define.physVol);
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Extracted shape "<<m_geoUtilTool->dumpShape(shape));
    /// The half sizes of the 
    if (shape->typeID() != GeoBox::getClassTypeID()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" expect shape to be a box but it's "<<m_geoUtilTool->dumpShape(shape));
        return StatusCode::FAILURE;
    }

    const GeoBox* box = static_cast<const GeoBox*>(shape);   
    define.halfThickness = box->getXHalfLength() * Gaudi::Units::mm;
    define.halfLength = box->getZHalfLength() * Gaudi::Units::mm;
    define.halfWidth = box->getYHalfLength() * Gaudi::Units::mm;
   
    /** Rpc are made up out of 2 or 3 gasGap singlet. A singlet module is a RPC gas gap
     *  sandwiched by two strip layers. In large sectors, the gas gap may be split
     *  into two gasGaps.  
     *
     *          | Strip layer  |  Strip layer |    |  Strip layer  |  Strip layer |
     *          |           gas gap           |    |    Gas gap    |    Gas gap   | 
     *          | Strip layer  |  Strip layer |    |  Strip layer  |  Strip layer |
     *   
    */
    std::vector<GeoChildNodeWithTrf> rpcLayers = getChildrenWithRef(define.physVol, false);
    /// Fetch all volumes with Identifiers from the tree
    rpcLayers.erase(std::remove_if(rpcLayers.begin(), rpcLayers.end(),
                     [](const GeoChildNodeWithTrf& subVol){ return !subVol.volumeId; }), rpcLayers.end());
    /// Next sort them by Identifier
    std::ranges::sort(rpcLayers, [](const GeoChildNodeWithTrf& a, const GeoChildNodeWithTrf& b){ 
                                  return a.volumeId.value_or(0) < b.volumeId.value_or(0);});
    
    if (rpcLayers.empty()) {
        ATH_MSG_FATAL("The volume "<<m_idHelperSvc->toStringDetEl(define.detElId)<<" does not have any childern with Identifiers "
            <<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
        return StatusCode::FAILURE;
    }
    /// Fetch for each rpc layer the gasGaps
    unsigned gasGap{0};
    const unsigned modulePhi = m_idHelperSvc->rpcIdHelper().doubletPhi(define.detElId);

    std::vector<gapVolume> allGapsWithIdx{};
    for (const GeoChildNodeWithTrf& rpcSinglet : rpcLayers) {
        auto fetchNodes = [this, &rpcSinglet] (const std::string& leafName) {
            std::vector<GeoChildNodeWithTrf> nodes = m_geoUtilTool->findAllLeafNodesByName(rpcSinglet.volume, leafName);
            std::ranges::for_each(nodes,[&rpcSinglet](GeoChildNodeWithTrf& node){ node.transform = rpcSinglet.transform * node.transform; });
            return nodes;
        };
        std::vector<GeoChildNodeWithTrf> gasGaps = fetchNodes("RpcGasGap");
        if (gasGaps.empty()) {
            ATH_MSG_FATAL("The child "<<m_geoUtilTool->dumpVolume(rpcSinglet.volume)<<" has "<<gasGaps.size()<<" gasGaps. ");
            return StatusCode::FAILURE;
        }
        std::vector<GeoChildNodeWithTrf> stripLayers = fetchNodes("bottomStripLayer");
        if (stripLayers.empty()) {
            ATH_MSG_FATAL("The child "<<m_geoUtilTool->dumpVolume(rpcSinglet.volume)<<" does not have a strip layer ");
            return StatusCode::FAILURE;
        }
        ++gasGap;
        for (GeoChildNodeWithTrf& stripPanel : stripLayers) {
            /// Adjust the height of the strip panel to be in the centre of the gasGap
            stripPanel.transform.translation().x() = gasGaps.front().transform.translation().x();
            const int doubPhi = std::max(modulePhi, 1u*stripPanel.volumeId.value_or(999));
            allGapsWithIdx.emplace_back(std::move(stripPanel), gasGap, doubPhi);
        }
    }
   
    const bool isAside{m_idHelperSvc->stationEta(define.detElId) > 0};
    /// We know now whether we had 2 or 3 gasgaps and also whether there 2 or 1 panels in phi
    define.nGasGaps = gasGap;
    /// Special case for the BML4 DBZ = 3 chambers. The doubletPhi is incorporated 
    /// into the detector element but there's only one strip panel
    define.nPanelsInPhi = modulePhi == 2 ? 1 : allGapsWithIdx.size () / gasGap;    
    FactoryCache::ParamBookTable::const_iterator parBookItr = factoryCache.parameterBook.find(define.chambDesign);
    if (parBookItr == factoryCache.parameterBook.end()) {
        ATH_MSG_FATAL("The chamber "<<define.chambDesign<<" is not part of the WRPC table");
        return StatusCode::FAILURE;
    }
    const wRPCTable& paramBook{parBookItr->second};

    auto insertStripLayer = [this, &define, &factoryCache](std::unique_ptr<StripLayer> stripLay) {
        const unsigned layIdx = static_cast<unsigned>(stripLay->hash());
        if (layIdx >= define.layers.size()) {
            define.layers.resize(layIdx + 1);
        }
        define.layers[layIdx] = (*factoryCache.stripLayers.emplace(std::move(stripLay)).first);
        ATH_MSG_VERBOSE("Added new eta gap at "<<(*define.layers[layIdx]));
    };
    for (gapVolume& gapVol : allGapsWithIdx) {
        const GeoShape* gapShape = m_geoUtilTool->extractShape(gapVol.volume);
        if (gapShape->typeID() != GeoBox::getClassTypeID()) {
            ATH_MSG_FATAL("Failed to extract a geo shape");
            return StatusCode::FAILURE;
        }
        const GeoBox* gapBox = static_cast<const GeoBox*>(gapShape);
        ATH_MSG_DEBUG("Gas gap dimensions "<<m_geoUtilTool->dumpShape(gapBox));
        StripDesignPtr etaDesign = constructDesign(gapBox, paramBook, false);
        StripDesignPtr phiDesign = constructDesign(gapBox, paramBook, true);
        if (etaDesign) {
            etaDesign = (*factoryCache.stripDesigns.emplace(etaDesign).first);
        }
        if (phiDesign) {
            phiDesign = (*factoryCache.stripDesigns.emplace(phiDesign).first);
        }
        if (!define.etaDesign) {
            define.etaDesign = etaDesign;
        }
        if (!define.phiDesign) {
            define.phiDesign = phiDesign;
        }
        gapVol.transform = gapVol.transform * Amg::getRotateY3D( (isAside ? -90. :  90.)* Gaudi::Units::degree);

        if (etaDesign) {
            insertStripLayer(std::make_unique<StripLayer>(factoryCache.trfNodeMaker.makeTransform(gapVol.transform), 
                                                          etaDesign,
                                                          RpcReadoutElement::createHash(0, gapVol.gasGap, gapVol.doubPhi, false)));
        }
        if (phiDesign) {
            insertStripLayer(std::make_unique<StripLayer>(factoryCache.trfNodeMaker.makeTransform(gapVol.transform*
                                                                                                  Amg::getRotateZ3D(90. * Gaudi::Units::deg)), 
                                                          phiDesign,
                                                          RpcReadoutElement::createHash(0, gapVol.gasGap, gapVol.doubPhi, true))); 
        }     
    }
    return StatusCode::SUCCESS;
}
StatusCode RpcReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
    GeoModelIO::ReadGeoModel* sqliteReader = m_geoDbTagSvc->getSqliteReader();
    if (!sqliteReader) {
        ATH_MSG_FATAL("Error, the tool works exclusively from sqlite geometry inputs");
        return StatusCode::FAILURE;
    }
    
    FactoryCache facCache{};
    ATH_CHECK(readParameterBook(facCache));

    const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
    // Get the list of full phys volumes from SQLite, and create detector elements

    /// Retrieve the list of full physical volumes & alignable nodes and connect them together afterwards
    physNodeMap mapFPV = sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("Muon");
#ifndef SIMULATIONBASE
    auto layerBounds = std::make_shared<Acts::SurfaceBoundFactory>();
#endif
    for (auto& [key, pv] : mapFPV) {
        /// The keys should be formatted like
        /// <STATION_NAME>_<MUON_CHAMBERTYPE>_etc. The <MUON_CHAMBERTYPE> also
        /// indicates whether we're dealing with a MDT / TGC / CSC / RPC chamber
        ///    If we are dealing with a MDT chamber, then there are 3 additional
        ///    properties encoded into the chamber
        ///       <STATIONETA>_<STATIONPHI>_<DOUBLETR>_<DOUBLETPHI>_<DOUBLETZ>
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() < 7 ||
            key_tokens[1].find("RPC") == std::string::npos) {
            continue;
        }
        bool isValid{false};
        /// Retrieve first the station Identifier
        const Identifier elementID = idHelper.padID(idHelper.stationNameIndex(key_tokens[0].substr(0, 3)),
                                                    atoi(key_tokens[2]),      ///stationEta
                                                    atoi(key_tokens[3]),      ///stationPhi
                                                    atoi(key_tokens[4]),      ///DoubletR
                                                    atoi(key_tokens[6]),      ///DoubletZ
                                                    atoi(key_tokens[5]),      ///DoubletPhi
                                                    isValid);
        if (!isValid) {
            ATH_MSG_FATAL("Failed to construct the station Identifier from "<<key);
            return StatusCode::FAILURE;
        }
        defineArgs define{};        
        define.physVol = pv;
        define.chambDesign = key_tokens[1];
        define.alignTransform = m_geoUtilTool->findAlignableTransform(define.physVol);
        define.detElId = elementID;
        ATH_MSG_VERBOSE("Key  "<<key<<" lead to Identifier "<<m_idHelperSvc->toStringDetEl(elementID));
        ATH_CHECK(loadDimensions(define, facCache));
#ifndef SIMULATIONBASE
        define.layerBounds = layerBounds;
#endif
        std::unique_ptr<RpcReadoutElement> readoutEle = std::make_unique<RpcReadoutElement>(std::move(define));
        ATH_CHECK(mgr.addRpcReadoutElement(std::move(readoutEle)));
    }    
    return StatusCode::SUCCESS;
}
StatusCode RpcReadoutGeomTool::readParameterBook(FactoryCache& cache) {
    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(), name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("WRPC", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    
    for (const IRDBRecord_ptr& record : *paramTable) {
        const std::string chambType = record->getString("WRPC_TYPE");
        wRPCTable& parBook = cache.parameterBook[record->getString("WRPC_TYPE")];
        parBook.stripPitchEta = record->getDouble("etaStripPitch") * Gaudi::Units::cm;
        parBook.stripPitchPhi = record->getDouble("phiStripPitch") * Gaudi::Units::cm;
        const double stripDeadWidth = record->getDouble("stripDeadWidth") * Gaudi::Units::cm;
        parBook.stripWidthEta = parBook.stripPitchEta - stripDeadWidth;
        parBook.stripWidthPhi = parBook.stripPitchPhi - stripDeadWidth;
        parBook.numEtaStrips = record->getInt("nEtaStrips");
        parBook.numPhiStrips = record->getInt("nPhiStrips");
        parBook.firstOffSetPhi = record->getDouble("phiStripOffSet") * Gaudi::Units::cm + 
                                 0.5 * parBook.stripPitchPhi;
        parBook.firstOffSetEta = record->getDouble("etaStripOffSet") * Gaudi::Units::cm +
                                 record->getDouble("TCKSSU") * Gaudi::Units::cm +
                                 0.5 * parBook.stripPitchEta;
        
        ATH_MSG_VERBOSE("Extracted parameters for chamber "<<chambType
                       <<", num strips (eta/phi): "<<parBook.numEtaStrips<<"/"<<parBook.numPhiStrips
                       <<", strip pitch (eta/phi) "<<parBook.stripPitchEta<<"/"<<parBook.stripPitchPhi
                       <<", strip width (eta/phi): "<<parBook.stripWidthEta<<"/"<<parBook.stripWidthPhi
                       <<", strip offset (eta/phi): "<<parBook.firstOffSetEta<<"/"<<parBook.firstOffSetPhi
                       <<", etaStripOffSet: "<<(record->getDouble("etaStripOffSet") * Gaudi::Units::cm)
                       <<", phiStripOffSet: "<<(record->getDouble("phiStripOffSet") * Gaudi::Units::cm)
                       <<", TCKSSU: "<<(record->getDouble("TCKSSU")* Gaudi::Units::cm));
    }
    return StatusCode::SUCCESS;
}
}  // namespace MuonGMR4
