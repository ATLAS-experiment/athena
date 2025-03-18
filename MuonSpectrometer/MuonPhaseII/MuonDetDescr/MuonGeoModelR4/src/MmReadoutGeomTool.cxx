/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MmReadoutGeomTool.h"

#include <ActsGeoUtils/SurfaceBoundSet.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecordset.h>

#include <EventPrimitives/EventPrimitivesToStringConverter.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoModelKernel/GeoFullPhysVol.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelKernel/GeoTrd.h>

#include <GeoModelRead/ReadGeoModel.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <RDBAccessSvc/IRDBRecord.h>

#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#endif

using namespace ActsTrk;
using namespace CxxUtils;



namespace MuonGMR4 {


using physVolWithTrans = IMuonGeoUtilityTool::physVolWithTrans;

StatusCode MmReadoutGeomTool::loadDimensions(MmReadoutElement::defineArgs& define,
                                              FactoryCache& factoryCache) {    
    
    ATH_MSG_VERBOSE("Load dimensions of "<<m_idHelperSvc->toString(define.detElId)
                     <<std::endl<<std::endl<<m_geoUtilTool->dumpVolume(define.physVol->getParent()));
    const GeoShape* shape = m_geoUtilTool->extractShape(define.physVol);
    if (!shape) {
        ATH_MSG_FATAL("Failed to deduce a valid shape for "<<m_idHelperSvc->toString(define.detElId));
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Extracted shape "<<m_geoUtilTool->dumpShape(shape));
    /// The half sizes of the MicroMegas trapezoid
    if (shape->typeID() != GeoTrd::getClassTypeID()) {
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" expect shape to be a trapezoid but it's "<<m_geoUtilTool->dumpShape(shape));
        return StatusCode::FAILURE;
    }

    const GeoTrd* trapezoid = static_cast<const GeoTrd*>(shape);   
    define.halfThickness = trapezoid->getXHalfLength1() * Gaudi::Units::mm;
    define.halfShortWidth = trapezoid->getYHalfLength1() * Gaudi::Units::mm;
    define.halfLongWidth = trapezoid->getYHalfLength2() * Gaudi::Units::mm;
    define.halfHeight = trapezoid->getZHalfLength() * Gaudi::Units::mm;
   
    ATH_MSG_DEBUG("Extracted parameters "
                    <<", halfThickness: "<<define.halfThickness<<"/"
                    <<", halfShortWidth : "<<define.halfShortWidth<<"/"
                    <<", halfLongWidth : "<<define.halfLongWidth<<"/"
                    <<", halfHeight : "<<define.halfHeight);


    std::vector<physVolWithTrans> allGasGaps = m_geoUtilTool->findAllLeafNodesByName(define.physVol, "actMicroMegaGas");
    if (allGasGaps.empty()) {
        ATH_MSG_FATAL("The volume "<<m_idHelperSvc->toStringDetEl(define.detElId)<<" does not have any children actMicroMegaGas");
        return StatusCode::FAILURE;
    }

    /// Filling in number of layers
    define.nGasGaps = allGasGaps.size();
    ATH_MSG_VERBOSE("The number of gasGaps are: " << define.nGasGaps);

    FactoryCache::ParamBookTable::const_iterator parBookItr = factoryCache.parameterBook.find(define.chambDesign);
    if (parBookItr == factoryCache.parameterBook.end()) {
        ATH_MSG_FATAL("The chamber "<<define.chambDesign<<" is not part of the WMM table");
        return StatusCode::FAILURE;
    }     

    const wMMTable& paramBook{parBookItr->second};
    
    define.readoutSide = paramBook.readoutSide;


    for (std::size_t gap = 0; gap < allGasGaps.size(); ++gap) {

        auto& gapVol = allGasGaps[gap];
        const Amg::Vector3D posGapI = gapVol.transform.translation();

        //Check sorting of gasGaps. For Q1 --> Eta layers should be first. For Q2--> Stereo Layers should be first.
        //Add ATH_MSG_INFO("**************************************"); before the gasGap loop.
        ATH_MSG_DEBUG("quadruplet  " << define.chambDesign.substr(6,7) << "  stereoAngle : " << paramBook.stereoAngle.at(gap) << " totalStrips " << paramBook.totalActiveStrips.at(gap)  <<  "   GasGAP POS X : " << posGapI.x() );

        const GeoShape* gapShape = m_geoUtilTool->extractShape(gapVol.volume);
        if (gapShape->typeID() != GeoTrd::getClassTypeID()) {
            ATH_MSG_FATAL("Failed to extract a geo shape");
            return StatusCode::FAILURE;
        }

        bool isStereo = static_cast<bool>(paramBook.stereoAngle.at(gap));

        const GeoTrd* gapTrd = static_cast<const GeoTrd*>(gapShape);
        ATH_MSG_DEBUG("MicroMegas Gas gap dimensions "<<m_geoUtilTool->dumpShape(gapTrd));
        double gapHalfHeight = gapTrd->getZHalfLength();
        double gapHalfShortY = std::min(gapTrd->getYHalfLength1(), gapTrd->getYHalfLength2());
        double gapHalfLongY = std::max(gapTrd->getYHalfLength1(), gapTrd->getYHalfLength2());

        double firstStripPos{0.};
        int firstActiveStrip{0};
        if (isStereo) {
            firstActiveStrip = paramBook.nMissedBottomStereo + 1;
            firstStripPos = -gapHalfHeight + (1.*(firstActiveStrip - paramBook.nMissedBottomEta) -0.5)* paramBook.stripPitch;
        } else {
            firstActiveStrip = paramBook.nMissedBottomEta + 1;
            firstStripPos = -gapHalfHeight +  0.5*paramBook.stripPitch;
        }
        /*The origin of the chamber/gasGap axes system is located at the center of the chamber.
        We subtract the HalfLength across the Z axis to transform from the center to the origin of the trapezoid
        The we add the strip pitch to reach the position of the first strip.*/
        StripDesignPtr stripDesign = std::make_unique<StripDesign>();

        stripDesign->defineStripLayout(firstStripPos * Amg::Vector2D::UnitX(),
                                        paramBook.stripPitch,
                                        paramBook.stripWidth,
                                        paramBook.totalActiveStrips.at(gap),
                                        firstActiveStrip);       

        /// The stereo angle is defined clock-wise from the y-axis. So we need to input it with a minus when defining the trapezoid
        stripDesign->defineTrapezoid(gapHalfShortY, gapHalfLongY, gapHalfHeight, paramBook.stereoAngle.at(gap));

        //Necessary strip layer rotation to match the alignment coordinate system
        Amg::Transform3D stripLayerRotation{gapVol.transform
                                    * Amg::getRotateY3D(-90.*Gaudi::Units::deg)
                                    * Amg::getRotateZ3D(stripDesign->stereoAngle())};


        stripDesign = (*factoryCache.stripDesigns.emplace(stripDesign).first);
        auto stripLayer = std::make_unique<StripLayer>(stripLayerRotation, stripDesign, 
                                                       IdentifierHash{static_cast<unsigned int>(gap)});
        define.layers.push_back(*factoryCache.stripLayers.emplace(std::move(stripLayer)).first);
    } //end of gas gap loop
    return StatusCode::SUCCESS;
}

StatusCode MmReadoutGeomTool::buildReadOutElements(MuonDetectorManager& mgr) {
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


    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
    // Get the list of full phys volumes from SQLite, and create detector elements
    /// Retrieve the list of full physical volumes & alignable nodes and connect them together afterwards
    physNodeMap mapFPV = sqliteReader->getPublishedNodes<std::string, GeoFullPhysVol*>("Muon");
#ifndef SIMULATIONBASE
    SurfaceBoundSetPtr<Acts::TrapezoidBounds> layerBounds= std::make_shared<SurfaceBoundSet<Acts::TrapezoidBounds>>();
#endif 

    for (auto& [key, pv] : mapFPV) {
        /// For MicroMegas the Keys are formatted in the following way.
        ///   <MM>_<LARGE/SMALL SECTOR + MODULE TYPE>_<QUADRUPLET NUMBER>_<ETA INDEX>_<PHI INDEX>_<Quadruplet>
        // e.g. MM_SM1Q2_1_6_1 . It's the "type" Attribute in the new GeoModel XML files.
        std::vector<std::string> key_tokens = tokenize(key, "_");

        if (key_tokens[0].find("MM") == std::string::npos){
            continue;
        }
        ATH_MSG_DEBUG("Retrieving MicroMegas Quadruplet : " << key );

        MmReadoutElement::defineArgs define{};
        bool isValid{false};
        define.detElId = idHelper.channelID(key_tokens[1][0] == 'S' ? "MMS" : "MML", // Replace <MM> string part with <MMS> or <MML> to match the Identifier.
                                            atoi(key_tokens[2].c_str()), // Eta index
                                            atoi(key_tokens[3].c_str()), // Phi index (from 0 to 7 in GeoModel). needs a +1
                                            atoi(key_tokens[4].c_str()), 1, 1, isValid); //Copy Number which reflects the number of the multilayer.
                                            // THen the two 1s are reflecting gasGap and channel Number. They can be set to 1s as this is all we need
                                            // to get the Identifier for the multilayer.

    
        if (!isValid) {
            ATH_MSG_FATAL("Failed to build a good identifier out of " << key);
            return StatusCode::FAILURE;
        }      

        ATH_MSG_DEBUG("Key "<<key<<" brought us "<<m_idHelperSvc->toStringDetEl(define.detElId));
        define.physVol = pv;
        define.chambDesign = key_tokens[0]+"_"+key_tokens[1]; // Recover the string denoted in WMM tables. e.g. chambDesign = "MM_SM1Q2"
        define.alignTransform = m_geoUtilTool->findAlignableTransform(define.physVol);  
        ATH_CHECK(loadDimensions(define, facCache));
#ifndef SIMULATIONBASE
        define.layerBounds = layerBounds;
#endif
        std::unique_ptr<MmReadoutElement> readoutEle = std::make_unique<MmReadoutElement>(std::move(define));
        ATH_CHECK(mgr.addMmReadoutElement(std::move(readoutEle)));
    }    
    return StatusCode::SUCCESS;
}



StatusCode MmReadoutGeomTool::readParameterBook(FactoryCache& cache) {

    ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(), name());
    ATH_CHECK(accessSvc.retrieve());
    IRDBRecordset_ptr paramTable = accessSvc->getRecordsetPtr("WMM", "");
    if (paramTable->size() == 0) {
        ATH_MSG_FATAL("Empty parameter book table found");
        return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Found the " << paramTable->nodeName() << " ["
                                << paramTable->tagName() << "] table with "
                                << paramTable->size() << " records");
    

    
    for (const IRDBRecord_ptr& record : *paramTable) {
        const std::string chambType = record->getString("WMM_TYPE");
        wMMTable& parBook = cache.parameterBook[chambType];
        parBook.stripPitch = record->getDouble("stripPitch") ;
        parBook.stripWidth = record->getDouble("stripWidth") ; 
        parBook.stereoAngle = tokenizeDouble(record->getString("stereoAngle"), ";");
        parBook.totalActiveStrips = tokenizeInt(record->getString("totalActiveStrips"), ";");
        parBook.readoutSide = tokenizeInt(record->getString("readoutSide"),";");
        parBook.nMissedBottomEta = record->getInt("nMissedBottomEta"); 
        parBook.nMissedBottomStereo = record->getInt("nMissedBottomStereo"); 
        parBook.nMissedTopEta = record->getInt("nMissedTopEta");
        parBook.distBotFrameStrip = record->getDouble("dR_botFrame1stStrip");
        
        ATH_MSG_VERBOSE("Extracted parameters for chamber "<<chambType
                       <<", stripPitch: "<<parBook.stripPitch
                       <<", stripWidth: "<<parBook.stripWidth
                       <<", steroAngle: "<<parBook.stereoAngle
                       <<", totalActiveStrips: "<<parBook.totalActiveStrips
                       <<", readoutSites: "<<parBook.readoutSide);
    }
    
    return StatusCode::SUCCESS;
}





}  // namespace MuonGMR4
