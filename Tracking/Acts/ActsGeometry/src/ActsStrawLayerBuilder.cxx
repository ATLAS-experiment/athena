/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// ATHENA
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "TRT_ReadoutGeometry/TRT_Numerology.h"
#include "InDetIdentifier/TRT_ID.h"
#include "Identifier/Identifier.h"

// PACKAGE
#include "ActsGeometry/ActsStrawLayerBuilder.h"
#include "ActsGeometry/ActsDetectorElement.h"

// ACTS
#include "Acts/Geometry/ILayerBuilder.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Geometry/LayerCreator.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Geometry/ProtoLayer.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Utilities/BinningType.hpp"

// STL
#include <Acts/Utilities/AxisDefinitions.hpp>
#include <iostream>
#include <fstream>

using namespace Acts::UnitLiterals;

const Acts::LayerVector
ActsStrawLayerBuilder::negativeLayers(const Acts::GeometryContext& gctx) const
{
  return endcapLayers(gctx, -1);
}

const Acts::LayerVector
ActsStrawLayerBuilder::positiveLayers(const Acts::GeometryContext& gctx) const
{
  return endcapLayers(gctx, 1);

}


const Acts::LayerVector
ActsStrawLayerBuilder::centralLayers(const Acts::GeometryContext& gctx) const
{
  ACTS_VERBOSE("Building central Straw layers");

  using LBBV = Acts::LineBounds::BoundValues;
  using enum Acts::AxisDirection;

  const InDetDD::TRT_Numerology* trtNums = m_cfg.mng->getNumerology();
  size_t    nBarrelRings  = trtNums->getNBarrelRings();
  size_t    nBarrelPhiSectors = trtNums->getNBarrelPhi();


  ACTS_VERBOSE("- Numerology reports: - " << nBarrelRings << " barrel rings");
  ACTS_VERBOSE("                      - " << nBarrelPhiSectors << " barrel phi sectors");

  Acts::LayerVector layers;

  std::vector<Acts::ProtoLayer> protoLayers;

  for(size_t iring=0; iring < nBarrelRings;iring++) {
    ACTS_VERBOSE("- Collecting elements for ring " << iring);

    // were calculating min/max radius while were at it.
    Acts::ProtoLayer pl;
    auto& ext = pl.extent;
    ext.range(AxisR).setMin( std::numeric_limits<double>::max() );
    ext.range(AxisR).setMax( std::numeric_limits<double>::lowest() );
    ext.range(AxisZ).setMin( std::numeric_limits<double>::max() );
    ext.range(AxisZ).setMax( std::numeric_limits<double>::lowest() );
    ext.range(AxisPhi).setMin( -M_PI );
    ext.range(AxisPhi).setMax( M_PI );

    pl.envelope[AxisZ] = {1_mm, 1_mm};
    pl.envelope[AxisR] = {0_mm, 0_mm};

    double fudge = 0_mm;
    // RING in TRT speak is translated to Layer in ACTS speak
    std::vector<std::shared_ptr<const Acts::Surface>> layerSurfaces;

    size_t nBarrelLayers = trtNums->getNBarrelLayers(iring);
    ACTS_VERBOSE("  - Numerology reports: " << nBarrelLayers << " layers in ring " << iring);
    for (size_t ilayer=0;ilayer < nBarrelLayers;ilayer++) {
      for (size_t iphisec=0;iphisec < nBarrelPhiSectors;iphisec++) {
        // what is this?
        for (size_t iposneg=0;iposneg<2;iposneg++) {

          const InDetDD::TRT_BarrelElement* brlElem
            = m_cfg.mng->getBarrelElement(iposneg, iring, iphisec, ilayer);

          unsigned int nStraws = brlElem->nStraws();

          for(unsigned int istraw=0;istraw<nStraws;istraw++) {

            Acts::Transform3 trf = brlElem->strawTransform(istraw);
            // need to convert translation to length unit
            trf.translation() *= 1_mm;
            auto code = brlElem->getCode();
            Identifier straw_id = m_cfg.idHelper->straw_id(code.isPosZ() == 1 ? 1 : -1,
                                                    code.getPhiIndex(),
                                                    code.getModuleIndex(),
                                                    code.getStrawLayerIndex(),
                                                    istraw);

            auto elem = std::make_shared<const ActsDetectorElement>(
                trf, *brlElem, straw_id);

            m_cfg.elementStore->push_back(elem);

            auto straw = dynamic_cast<const Acts::StrawSurface*>(&elem->surface());
            if (not straw) continue;
            auto strawBounds = dynamic_cast<const Acts::LineBounds*>(&straw->bounds());
            if (not strawBounds) continue;
            // units should be fine since they're already converted in det elem construction
            double radius = strawBounds->get(LBBV::eR);
            double length = strawBounds->get(LBBV::eHalfLengthZ);
            fudge = radius / 4.;

            // calculate min/max R and Z
            Acts::Vector3 ctr = straw->center(gctx);
            ext.range(AxisR).setMax( std::max(ext.max(AxisR), ctr.perp() + radius) );
            ext.range(AxisR).setMin( std::min(ext.min(AxisR), ctr.perp() - radius) );
            ext.range(AxisZ).setMax( std::max(ext.max(AxisZ), ctr.z() + length) );
            ext.range(AxisZ).setMin( std::min(ext.min(AxisZ), ctr.z() - length) );

            layerSurfaces.push_back(straw->getSharedPtr());
          }
        }
      }
    }

    ACTS_VERBOSE("  - Collected " << layerSurfaces.size() << " straws");

    if(iring > 0) {
      // match outer radius of previous ring
      const Acts::ProtoLayer &prev = protoLayers.at(iring-1);
      ext.range(AxisR).setMin( prev.extent.max(AxisR) + prev.envelope[AxisR][1] + pl.envelope[AxisR][0] + fudge );
    }

    std::shared_ptr<Acts::Layer> layer
      = m_cfg.layerCreator->cylinderLayer(gctx, std::move(layerSurfaces), 100, 1, pl);
    layers.push_back(layer);

    protoLayers.push_back(pl);

  }

  return layers;
}

const Acts::LayerVector
ActsStrawLayerBuilder::endcapLayers(const Acts::GeometryContext& gctx, int side) const
{
  ACTS_VERBOSE("Building endcap Straw layers");
  using LBBV = Acts::LineBounds::BoundValues;
  using enum Acts::AxisDirection;

  const InDetDD::TRT_Numerology* trtNums = m_cfg.mng->getNumerology();
  size_t    nEndcapWheels  = trtNums->getNEndcapWheels();
  size_t    nEndcapPhiSectors = trtNums->getNEndcapPhi();

  ACTS_VERBOSE("- Numerology reports: - " << nEndcapWheels<< " endcap wheels");
  ACTS_VERBOSE("                      - " << nEndcapPhiSectors << " endcap phi sectors");

  Acts::LayerVector layers;

  for(size_t iwheel=0;iwheel<nEndcapWheels;++iwheel) {
    ACTS_VERBOSE("- Collecting elements for wheel " << iwheel);


    size_t nEndcapLayers = trtNums->getNEndcapLayers(iwheel);
    ACTS_VERBOSE("  - Numerology reports: " << nEndcapLayers << " layers in wheel " << iwheel);
    for(size_t ilayer=0;ilayer<nEndcapLayers;++ilayer) {
      std::vector<std::shared_ptr<const Acts::Surface>> wheelSurfaces;


      Acts::ProtoLayer pl;
      auto& ext = pl.extent;;
      ext.range(AxisR).setMin( std::numeric_limits<double>::max() );
      ext.range(AxisR).setMax( std::numeric_limits<double>::lowest() );
      ext.range(AxisZ).setMin( std::numeric_limits<double>::max() );
      ext.range(AxisZ).setMax( std::numeric_limits<double>::lowest() );
      ext.range(AxisPhi).setMin( -M_PI );
      ext.range(AxisPhi).setMax( M_PI );
      pl.envelope[AxisR] = {0_mm, 0_mm};

      for (unsigned int iphisec=0; iphisec<nEndcapPhiSectors; ++iphisec) {

        size_t iposneg = side < 0 ? 0 : 1;
        const InDetDD::TRT_EndcapElement* ecElem = m_cfg.mng->getEndcapElement(iposneg, iwheel, ilayer, iphisec);
        unsigned int nStraws = ecElem->nStraws();

        for(unsigned int istraw=0;istraw<nStraws;istraw++) {

          Acts::Transform3 trf = (ecElem->strawTransform(istraw));
          // need to convert translation to length unit
          trf.translation() *= 1_mm;

          auto code = ecElem->getCode();
          Identifier straw_id = m_cfg.idHelper->straw_id(code.isPosZ() == 1 ? 2 : -2,
                                                  code.getPhiIndex(),
                                                  code.getWheelIndex(),
                                                  code.getStrawLayerIndex(),
                                                  istraw);


          auto elem = std::make_shared<const ActsDetectorElement>(
              trf, *ecElem, straw_id);

          m_cfg.elementStore->push_back(elem);

          auto straw = dynamic_cast<const Acts::StrawSurface*>(&elem->surface());
          if (straw){
            auto strawBounds = dynamic_cast<const Acts::LineBounds*>(&straw->bounds());
            if (strawBounds){
              double radius = strawBounds->get(LBBV::eR);
              double length = strawBounds->get(LBBV::eHalfLengthZ);

              Acts::Vector3 ctr = straw->center(gctx);
              ext.range(AxisZ).setMax( std::max(ext.max(AxisZ), ctr.z() + radius) );
              ext.range(AxisZ).setMin( std::min(ext.min(AxisZ), ctr.z() - radius) );
	      ext.range(AxisR).setMax( std::max(ext.max(AxisR), ctr.perp() + length) );
              ext.range(AxisR).setMin( std::min(ext.min(AxisR), ctr.perp() - length) );
	      pl.envelope[AxisZ] = {radius/2., radius/2.};

              wheelSurfaces.push_back(straw->getSharedPtr());
            }
          }
        }
      }

      std::shared_ptr<Acts::Layer> layer
        = m_cfg.layerCreator->discLayer(gctx, std::move(wheelSurfaces), 1, 100, pl);
      layers.push_back(layer);
      ACTS_VERBOSE("  - Collected " << wheelSurfaces.size() << " straws");
    }

  }

  ACTS_VERBOSE(" - Built " << layers.size() << " straw endcap layers");
  return layers;
}
