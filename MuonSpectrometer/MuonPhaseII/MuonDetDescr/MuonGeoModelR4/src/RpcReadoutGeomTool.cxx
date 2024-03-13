/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonGeoModelR4/RpcReadoutGeomTool.h>

#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoTrd.h>
#include <GeoModelKernel/GeoBox.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <RDBAccessSvc/IRDBRecord.h>

namespace {
    constexpr double tolerance = 0.001 * Gaudi::Units::mm;
}

using namespace CxxUtils;
using namespace ActsTrk;

namespace MuonGMR4 {

using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;
using defineArgs = RpcReadoutElement::defineArgs;
using StripLayerPtr = RpcReadoutElement::StripLayerPtr;

/// Helper struct to attribute the Identifier fields with the
/// gas gap volumes
struct gapVolume: public physVolWithTrans {
    gapVolume(physVolWithTrans&& physVol,
              unsigned int gap,
              unsigned int phi):
        physVolWithTrans{std::move(physVol)},
        gasGap{gap},
        doubPhi{phi} {}
    unsigned int gasGap{0};
    unsigned int doubPhi{0};
    
};

inline bool layerSorter(const physVolWithTrans&a, const physVolWithTrans & b){
    const Amg::Vector3D cA = a.transform.translation();
    const Amg::Vector3D cB = b.transform.translation();
    if (std::abs(cA.x() - cB.x()) > tolerance) return (cA.x() < cB.x());
    return (cA.y() < cB.y());
}


RpcReadoutGeomTool::RpcReadoutGeomTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent)
    : AthAlgTool{type, name, parent} {
    declareInterface<IMuonReadoutGeomTool>(this);

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
    std::vector<physVolWithTrans> stripLayers = m_geoUtilTool->findAllLeafNodesByName(define.physVol, "bottomStripLayer");
    if (stripLayers.empty()) {
        ATH_MSG_FATAL("The volume "<<m_idHelperSvc->toStringDetEl(define.detElId)<<" does not have any childern 'bottomStripLayer'"
            <<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
        return StatusCode::FAILURE;
    }   
    std::vector<physVolWithTrans> allGasGaps = m_geoUtilTool->findAllLeafNodesByName(define.physVol, "RpcGasGap");
    if (allGasGaps.empty()) {
        ATH_MSG_FATAL("The volume "<<m_idHelperSvc->toStringDetEl(define.detElId)<<" does not have any childern 'RpcGasGap'"
            <<std::endl<<m_geoUtilTool->dumpVolume(define.physVol));
        return StatusCode::FAILURE;
    }
    /// In the GeoModel world, the x-axis points in radial direction & y axis along the phi direction
    std::stable_sort(allGasGaps.begin(), allGasGaps.end(), layerSorter);
    std::stable_sort(stripLayers.begin(),stripLayers.end(), layerSorter);
    /// The strip layers are used to express the dimensions of the strip layer. However, that's projected into the 
    /// Center of the gasgap which may or maybe not be split into two --> Find the closest gas gap in x for each
    /// strip layer and overwrite the x coordinate of the strip layerof that one.
    for (physVolWithTrans& stripLayer : stripLayers){
        /// Find the closest gas Gap
        const Amg::Vector3D stripTrans = stripLayer.transform.translation();
        std::vector<physVolWithTrans>::iterator closestGap =  std::min_element(allGasGaps.begin(), allGasGaps.end(), 
                             [&stripTrans](const physVolWithTrans& a, const physVolWithTrans& b){
                                return std::abs(stripTrans.x() - a.transform.translation().x()) <
                                       std::abs(stripTrans.x() - b.transform.translation().x());                                
                             });
        stripLayer.transform.translation().x() = closestGap->transform.translation().x();
    }

    /// Now we need to associate the gasGap volumes with the gas gap number &
    /// the doublet Phi
    Amg::Vector3D prevGap{stripLayers[0].transform.translation()};
    unsigned int gasGap{1}, doubletPhi{0};
    
    unsigned int modulePhi = m_idHelperSvc->rpcIdHelper().doubletPhi(define.detElId);
    std::vector<gapVolume> allGapsWithIdx{};
    const bool isAside{m_idHelperSvc->stationEta(define.detElId) > 0};
    for (physVolWithTrans& gapVol : stripLayers) {
        Amg::Vector3D gCen = gapVol.transform.translation();
        /// The volume points to a new gasgap
        if (std::abs(gCen.x() - prevGap.x()) > tolerance) {
            ++gasGap;
            doubletPhi = 1;
        } else ++doubletPhi;
        ATH_MSG_DEBUG("Gas gap at "<<Amg::toString(gCen, 2)<<" is associated with gasGap: "<<gasGap<<", doubletPhi: "<<doubletPhi);
        prevGap = std::move(gCen);
        /// Rpc volumes with doubletZ = 3 have two gas gaps along phi but they're split into two
        /// distnict modules with doublePhi = 1, 2. 
        doubletPhi = std::max (doubletPhi, modulePhi);
        allGapsWithIdx.emplace_back(std::move(gapVol), gasGap, doubletPhi);
    }
    /// We know now whether we had 2 or 3 gasgaps and also whether there 2 or 1 panels in phi
    define.nGasGaps = gasGap;
    define.nGapsInPhi = doubletPhi;    
    FactoryCache::ParamBookTable::const_iterator parBookItr = factoryCache.parameterBook.find(define.chambDesign);
    if (parBookItr == factoryCache.parameterBook.end()) {
        ATH_MSG_FATAL("The chamber "<<define.chambDesign<<" is not part of the WRPC table");
        return StatusCode::FAILURE;
    }
    const wRPCTable& paramBook{parBookItr->second};

    for (gapVolume& gapVol : allGapsWithIdx) {
        const GeoShape* gapShape = m_geoUtilTool->extractShape(gapVol.volume);
        if (gapShape->typeID() != GeoBox::getClassTypeID()) {
            ATH_MSG_FATAL("Failed to extract a geo shape");
            return StatusCode::FAILURE;
        }
        const GeoBox* gapBox = static_cast<const GeoBox*>(gapShape);
        ATH_MSG_DEBUG("Gas gap dimensions "<<m_geoUtilTool->dumpShape(gapBox));

        StripDesignPtr etaDesign = std::make_unique<StripDesign>();
        /// Define the strip layout
        etaDesign->defineStripLayout(Amg::Vector2D{-gapBox->getZHalfLength() + paramBook.firstOffSetEta, 0.},
                                     paramBook.stripPitchEta,
                                     paramBook.stripWidthEta,
                                     paramBook.numEtaStrips);
        /// Define the box layout
        etaDesign->defineTrapezoid(gapBox->getYHalfLength(), gapBox->getYHalfLength(), gapBox->getZHalfLength());
        gapVol.transform = gapVol.transform * Amg::getRotateY3D( (isAside ? -90. :  90.)* Gaudi::Units::degree);
        
        etaDesign = (*factoryCache.stripDesigns.emplace(etaDesign).first);
        const IdentifierHash etaHash {RpcReadoutElement::createHash(0, gapVol.gasGap, gapVol.doubPhi, false)};
        const unsigned int etaIdx = static_cast<unsigned int>(etaHash);
        if (etaIdx >= define.layers.size()) {
            define.layers.resize(etaIdx + 1);
        }
        define.layers[etaIdx] = std::make_unique<StripLayer>(gapVol.transform, etaDesign, etaHash);
        
        ATH_MSG_VERBOSE("Added new eta gap at "<<(*define.layers[etaIdx]));
        if (!define.etaDesign) define.etaDesign = etaDesign;
        StripDesignPtr phiDesign = std::make_unique<StripDesign>();
        phiDesign->defineStripLayout(Amg::Vector2D{-gapBox->getYHalfLength() + paramBook.firstOffSetPhi, 0.},
                                                   paramBook.stripPitchPhi,
                                                   paramBook.stripWidthPhi,
                                                   paramBook.numPhiStrips);
        phiDesign->defineTrapezoid(gapBox->getZHalfLength(), gapBox->getZHalfLength(), gapBox->getYHalfLength());
        /// Next build the phi layer
        phiDesign = (*factoryCache.stripDesigns.emplace(phiDesign).first);
        
        const IdentifierHash phiHash {RpcReadoutElement::createHash(0, gapVol.gasGap, gapVol.doubPhi, true)};
        const unsigned int phiIdx = static_cast<unsigned int>(phiHash);
        if (phiIdx >= define.layers.size()) {
            define.layers.resize(phiIdx + 1);
        }
        define.layers[phiIdx] = std::make_unique<StripLayer>(gapVol.transform  * Amg::getRotateZ3D(90. * Gaudi::Units::deg),
                                                             phiDesign, phiHash);
        ATH_MSG_VERBOSE("Added new phi gap at "<<(*define.layers[phiIdx]));
        if (!define.phiDesign) define.phiDesign = phiDesign;
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
    SurfaceBoundSetPtr<Acts::RectangleBounds> layerBounds = std::make_shared<SurfaceBoundSet<Acts::RectangleBounds>>();
#endif
    for (auto& [key, pv] : mapFPV) {
        /// The keys should be formatted like
        /// <STATION_NAME>_<MUON_CHAMBERTYPE>_etc. The <MUON_CHAMBERTYPE> also
        /// indicates whether we're dealing with a MDT / TGC / CSC / RPC chamber
        ///    If we are dealing with a MDT chamber, then there are 3 additional
        ///    properties encoded into the chamber
        ///       <STATIONETA>_(<STATIONPHI>-1)_<DOUBLETR>_<DOUBLETPHI>_<DOUBLETZ>
        std::vector<std::string> key_tokens = tokenize(key, "_");
        if (key_tokens.size() < 7 ||
            key_tokens[1].find("RPC") == std::string::npos)
            continue;
       
        bool isValid{false};
        /// Retrieve first the station Identifier
        const Identifier elementID = idHelper.padID(idHelper.stationNameIndex(key_tokens[0].substr(0, 3)),
                                                    atoi(key_tokens[2]),      ///stationEta
                                                    atoi(key_tokens[3]) + 1,  ///stationPhi
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
    
    for (const IRDBRecord* record : *paramTable) {
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
