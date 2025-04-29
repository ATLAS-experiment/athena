/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackingGeometry/BeamPipeBuilderImpl.h"

// Amg
#include "GeoPrimitives/GeoPrimitives.h"
// Trk inlcude
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkDetDescrUtils/BinUtility.h"
//#include "TrkDetDescrGeoModelCnv/GeoShapeConverter.h"
#include "TrkGeometry/MaterialProperties.h"
#include "TrkGeometry/LayerMaterialProperties.h"
#include "TrkGeometry/BinnedLayerMaterial.h"
#include "TrkGeometry/HomogeneousLayerMaterial.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkSurfaces/CylinderBounds.h"
// GeoModel include
#include "BeamPipeGeoModel/BeamPipeDetectorManager.h"
#include "GeoModelKernel/GeoTube.h"

// Athena standard methods
// initialize
StatusCode InDet::BeamPipeBuilderImpl::initialize()
{
  if (m_beamPipeFromDb) {
    ATH_CHECK(detStore()->retrieve(m_beamPipeMgr,m_beamPipeMgrName));
    ATH_MSG_DEBUG("Successfully retrieved BeamPipe detector manager '" << m_beamPipeMgrName << "'." );
  }
  ATH_MSG_DEBUG( "initialize()" );
  return StatusCode::SUCCESS;
}

std::unique_ptr<const std::vector<Trk::CylinderLayer*> > InDet::BeamPipeBuilderImpl::cylindricalLayersImpl() const
{
  auto beamPipe = std::make_unique<std::vector<Trk::CylinderLayer*> >();

  // the geometry
  Amg::Transform3D beamPipeTransform;
  beamPipeTransform.setIdentity();

  double beamPipeRadius = m_beamPipeRadius;

  if (m_beamPipeMgr) {
    // get the central top volume
    PVConstLink beamPipeTopVolume =  m_beamPipeMgr->getTreeTop(0);
    if (m_beamPipeMgr->getNumTreeTops()==1) {
      // Beampipe implementation using assembly volume has only one tree top instead of 3 in the default case(union of a central and two forward beampipes)
      beamPipeTopVolume =  m_beamPipeMgr->getTreeTop(0)->getChildVol(0)->getChildVol(0);
      //the BeamPipeCentral volume is the child of the child volume of the top volume in this case
    }
    beamPipeTransform = Amg::Translation3D(beamPipeTopVolume->getX().translation().x(),
                                           beamPipeTopVolume->getX().translation().y(),
                                           beamPipeTopVolume->getX().translation().z());
    const GeoLogVol* beamPipeLogVolume = beamPipeTopVolume->getLogVol();
    const GeoTube* beamPipeTube = nullptr;
    if (beamPipeLogVolume) {
      // get the geoShape and translate
      //Trk::GeoShapeConverter geoShaper;
      beamPipeTube = dynamic_cast<const GeoTube*>(beamPipeLogVolume->getShape());
      if (beamPipeTube) {
        for(unsigned int i=0;i<beamPipeTopVolume->getNChildVols();i++) {
          if(beamPipeTopVolume->getNameOfChildVol(i)=="SectionC03"){
            PVConstLink childTopVolume =  beamPipeTopVolume->getChildVol(i);
            const GeoLogVol* childLogVolume = childTopVolume->getLogVol();
            const GeoTube* childTube = nullptr;
            if (childLogVolume) {
              childTube = dynamic_cast<const GeoTube*>(childLogVolume->getShape());
              if (childTube) {
                beamPipeRadius = 0.5 * (childTube->getRMax()+childTube->getRMin());
              }
            }
            break; //Exit loop after SectionC03 is found
          }
        } // Loop over child volumes
      }
    }
    ATH_MSG_VERBOSE("BeamPipe constructed from Database: translation (yes) - radius "<< ( beamPipeTube ? "(yes)" : "(no)") << " - r = " << beamPipeRadius );
  } else {
    beamPipeTransform = Amg::Translation3D(m_beamPipeOffsetX, m_beamPipeOffsetY, 0.);
  }

  ATH_MSG_VERBOSE("BeamPipe shift estimated as    : "
                  <<  beamPipeTransform.translation().x() << ", "
                  <<  beamPipeTransform.translation().y() << ","
                  <<  beamPipeTransform.translation().y());

  auto beamPipeBounds    = std::make_shared<Trk::CylinderBounds>(beamPipeRadius, m_beamPipeHalflength);
  ATH_MSG_VERBOSE("BeamPipe bounds constructed as : " << (*beamPipeBounds) );

  // the material
  Trk::MaterialProperties beamPipeMaterial(m_beamPipeThickness,
                                           m_beamPipeX0,
                                           m_beamPipeL0,
                                           m_beamPipeA,
                                           m_beamPipeZ,
                                           m_beamPipeRho);

  // binned layer material for the beam pipe possible
  Trk::CylinderLayer * pThisCylinderLayer{};
  if (m_beamPipeBinsZ == 1u) {
    const auto &beamPipeLayerMaterial = Trk::HomogeneousLayerMaterial(beamPipeMaterial, 1.0);
    pThisCylinderLayer = new Trk::CylinderLayer(beamPipeTransform,
                                                beamPipeBounds,
                                                beamPipeLayerMaterial,
                                                m_beamPipeThickness);

  } else {
    Trk::BinUtility layerBinUtility(m_beamPipeBinsZ, -m_beamPipeHalflength, m_beamPipeHalflength, Trk::open, Trk::binZ );
    const auto &beamPipeLayerMaterial = Trk::BinnedLayerMaterial(layerBinUtility);
    pThisCylinderLayer = new Trk::CylinderLayer(beamPipeTransform,
                                                beamPipeBounds,
                                                beamPipeLayerMaterial,
                                                m_beamPipeThickness);
  }
  beamPipe->push_back(pThisCylinderLayer);
  return beamPipe;
}
