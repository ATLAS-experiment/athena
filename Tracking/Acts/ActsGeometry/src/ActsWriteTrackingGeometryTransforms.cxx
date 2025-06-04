/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PACKAGE
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ActsWriteTrackingGeometryTransforms.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"


// ATHENA
#include "AthenaKernel/RNGWrapper.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ISvcLocator.h"
#include "ActsInterop/Logger.h"

// ACTS
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"

// STL
#include <string>

using gid = Acts::GeometryIdentifier;


StatusCode ActsWriteTrackingGeometryTransforms::initialize() {
  
  // Grab PixelID helper
  ATH_CHECK (detStore()->retrieve(m_pixelID, "PixelID") );
  ATH_CHECK (detStore()->retrieve(m_SCT_ID,"SCT_ID") );
  
  ATH_CHECK(m_trackingGeometryTool.retrieve());
  
  std::ofstream os(m_outputName); // truncate

  return StatusCode::SUCCESS;
}

StatusCode ActsWriteTrackingGeometryTransforms::execute() {
  
  ATH_MSG_DEBUG("In ActsWriteTrackingGeometryTransforms::execute");


  const EventContext& ctx = Gaudi::Hive::currentContext();
  
  auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();
  ATH_MSG_DEBUG("Retrieved tracking Geometry");
  const ActsGeometryContext& gctx = m_trackingGeometryTool->getGeometryContext(ctx);
  ATH_MSG_DEBUG("Retrieved geometry context");

  std::stringstream ss;


  std::ofstream os(m_outputName, std::ios_base::app);

  trackingGeometry->visitSurfaces([&] (const Acts::Surface* srf) {
    const Acts::DetectorElementBase *detElem = srf->associatedDetectorElement();
    const auto *gmde = static_cast<const ActsDetectorElement *>(detElem);

    Identifier ath_geoid = gmde->identify(); 

    if(dynamic_cast<const InDetDD::TRT_BaseElement*>(gmde->upstreamDetectorElement()) != nullptr) { 
      return;
    }
    
    if(dynamic_cast<const InDetDD::HGTD_DetectorElement*>(gmde->upstreamDetectorElement()) != nullptr) { 
      return;
    }
    
    const auto sil_de = dynamic_cast<const InDetDD::SiDetectorElement*>(gmde->upstreamDetectorElement());
    if(sil_de == nullptr) {
      throw std::runtime_error{"Not TRT, Not HGTD and not Si either"}; // this shouldn't happen
    }

    gid geoID = srf->geometryId();

    os << geoID.volume() << ";";
    os << geoID.boundary() << ";";
    os << geoID.layer() << ";";
    os << geoID.sensitive() << ";";

    os << ctx.eventID().event_number() << ";";

    int bec,ld,etam,phim, side;

    if(sil_de->isPixel()) {
      bec  =  m_pixelID->barrel_ec(ath_geoid);
      ld   =  m_pixelID->layer_disk(ath_geoid);
      etam =  m_pixelID->eta_module(ath_geoid);
      phim =  m_pixelID->phi_module(ath_geoid);
      side = 0;
      os << 0;
    }
    else if(sil_de->isSCT()) {
      
      bec =  m_SCT_ID->barrel_ec(ath_geoid);
      ld =   m_SCT_ID->layer_disk(ath_geoid);
      etam = m_SCT_ID->eta_module(ath_geoid);
      phim = m_SCT_ID->phi_module(ath_geoid);
      side = m_SCT_ID->side(ath_geoid);
      os << 1;
    }
    else {
      throw std::runtime_error{"The Detector Element is neither Pixel nor SCT"}; // this shouldn't happen
    }
    // Write the type of silicon first (0=PIX, 1=SCT)
    os << ";";
    
    // Then write the athena geoid and the unpacked version
    os<<ath_geoid<<","<<bec<<","<<ld<<","<<etam<<","<<phim<<","<<side<<";";
    
    ATH_MSG_DEBUG(geoID<<" "<<ath_geoid<<" "<<bec<<" "<<ld<<" "<<etam<<" "<<phim<<" "<<side);
    
    const ActsGeometryContext void_gctx;
    if (m_writeFullTransform) {
      // iterate over components of transform
      const auto* p = srf->transform(gctx.context()).data();
      for(size_t i=0;i<16;i++) {
        if(i>0) {
          os << ",";
        }
        os << *(p+i);
      }
    } else { // only write center of the detector element
      double cx = srf->center(void_gctx.context()).x();
      double cy = srf->center(void_gctx.context()).y();
      double cz = srf->center(void_gctx.context()).z();
      os<<cx<<","<<cy<<","<<cz;
    }
    
    os << "\n";
  });
  
  
  return StatusCode::SUCCESS;
}
