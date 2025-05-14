/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////
// HGTD_TrackingGeometryBuilderCond.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// HGTD
#include "HGTD_TrackingGeometryBuilderCond.h"
// EnvelopeDefinitionService
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"
// Trk interfaces
#include "TrkDetDescrInterfaces/ILayerBuilderCond.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeCreator.h"
#include "TrkDetDescrInterfaces/ILayerArrayCreator.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/GlueVolumesDescriptor.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkVolumes/VolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkSurfaces/DiscSurface.h"
//Gaudi
#include "GaudiKernel/MsgStream.h"
#include <algorithm>

// constructor
HGTD_TrackingGeometryBuilderCond::HGTD_TrackingGeometryBuilderCond(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p),
  m_enclosingEnvelopeSvc("AtlasEnvelopeDefSvc", n),
  m_trackingVolumeCreator("Trk::CylinderVolumeCreator/CylinderVolumeCreator"),
  m_indexStaticLayers(true),
  m_buildBoundaryLayers(true),
  m_replaceJointBoundaries(true),
  m_layerBinningType(2),
  m_colorCodeConfig(3)
{
  // envelope definition service
  declareProperty("EnvelopeDefinitionSvc",            m_enclosingEnvelopeSvc );
  declareProperty("LayerBuilder",                     m_layerBuilder);
  declareProperty("TrackingVolumeCreator",            m_trackingVolumeCreator);

  declareProperty("IndexStaticLayers",                m_indexStaticLayers);
  declareProperty("BuildBoundaryLayers",              m_buildBoundaryLayers);
  declareProperty("ReplaceAllJointBoundaries",        m_replaceJointBoundaries);
  declareProperty("LayerBinningType",                 m_layerBinningType);
  declareProperty("ColorCode",                        m_colorCodeConfig);

}

// destructor
HGTD_TrackingGeometryBuilderCond::~HGTD_TrackingGeometryBuilderCond()
= default;

// Athena standard methods
// initialize
StatusCode HGTD_TrackingGeometryBuilderCond::initialize()
{
  // retrieve envelope definition service
  ATH_CHECK(m_enclosingEnvelopeSvc.retrieve());

  // retrieve the layer provider
  ATH_CHECK(m_layerBuilder.retrieve());

  // retrieve the volume creator
  ATH_CHECK(m_trackingVolumeCreator.retrieve());

  ATH_MSG_INFO( "initialize() succesful" );
  return StatusCode::SUCCESS;
}

std::unique_ptr<Trk::TrackingGeometry>
HGTD_TrackingGeometryBuilderCond::trackingGeometry(
  const EventContext& ctx,
  Trk::TrackingVolume* innerVol,
  SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const

{

  ATH_MSG_VERBOSE( "Starting to build HGTD_TrackingGeometry ..." );

  // the enclosed input volume (ID)
  double enclosedInnerSectorHalflength = std::numeric_limits<float>::max();
  double enclosedOuterRadius = 0.;
  double enclosedInnerRadius = 0.;

  if (innerVol) {
    ATH_MSG_VERBOSE( "Got Inner Detector Volume: " << innerVol->volumeName() );
    innerVol->screenDump(msg(MSG::VERBOSE));

    // retrieve dimensions
    const Trk::CylinderVolumeBounds* innerDetectorBounds
      = dynamic_cast<const Trk::CylinderVolumeBounds*>(&(innerVol->volumeBounds()));
    if (!innerDetectorBounds) std::abort();

    enclosedInnerSectorHalflength = innerDetectorBounds->halflengthZ();
    enclosedOuterRadius = innerDetectorBounds->outerRadius();
    enclosedInnerRadius = innerDetectorBounds->innerRadius();
  }

  float enclosedOuterSectorHalflength = std::numeric_limits<float>::max();
  // for the HGTD we only need the first envelope definition
  for (const auto & bounds : m_enclosingEnvelopeSvc->getCaloRZBoundary()) {
    if (std::abs(bounds.second) < enclosedOuterSectorHalflength) {
      enclosedOuterSectorHalflength = std::abs(bounds.second);
    }
  }

  // in case you have no inner volume you need to find the
  // envelope extensions --> beampipe and HGTD
  if (not innerVol) {
    // from the beampipe envelope you get the inner z extension
    for (const auto & bounds : m_enclosingEnvelopeSvc->getBeamPipeRZBoundary()) {
      if (std::abs(bounds.second) < enclosedInnerSectorHalflength) {
        enclosedInnerSectorHalflength = std::abs(bounds.second);
      }
    }
    // from the calo envelope you get the outer radius
    for (const auto & bounds : m_enclosingEnvelopeSvc->getCaloRZBoundary()) {
      if (std::abs(bounds.second) == enclosedOuterSectorHalflength) {
        if (bounds.first>enclosedOuterRadius)
          enclosedOuterRadius=bounds.first;
      }
    }
  }

  ATH_MSG_VERBOSE("Got Dimensions Zmin/Rmin - Zmax/Rmax: "
                  << enclosedInnerSectorHalflength << "/" << enclosedInnerRadius
                  << " - " << enclosedOuterSectorHalflength << "/"
                  << enclosedOuterRadius);

  // prepare the layers
  std::vector<Trk::Layer*> negativeLayers;
  std::vector<Trk::Layer*> positiveLayers;

  std::unique_ptr<const std::vector<Trk::DiscLayer*> > discLayers = m_layerBuilder->discLayers(ctx, whandle);

  float maxZ = -9999.;
  float minZ =  9999.;
  float thickness = -9999;

  // loop and fill positive and negative Layers
  if (discLayers && !discLayers->empty()){
    // loop over and push into the return/cache vector
    for (const auto & discLayer : (*discLayers) ){
      // get the center posituion
      float zpos = discLayer->surfaceRepresentation().center().z();
      if (zpos > 0) {
        positiveLayers.push_back(discLayer);
        // only saving layer info for positive side
        // as the detector is simmetric
        maxZ = std::max(maxZ, zpos);
        minZ = std::min(minZ, zpos);
        thickness = std::max(thickness, float(discLayer->thickness()));
      }
      else {
        negativeLayers.push_back(discLayer);
      }
    }
  }

  float envelope = thickness*0.5;
  float minZ_HGTD = minZ-envelope;
  float maxZ_HGTD = maxZ+envelope;
  float maxZ_HGTDEnclosure = enclosedOuterSectorHalflength;

  // dummy material property
  auto materialProperties = std::make_unique<Trk::Material>();

  float zGapPos = 0.5*(minZ_HGTD+enclosedInnerSectorHalflength);
  float gapHalfLengthZ = 0.5*(minZ_HGTD-enclosedInnerSectorHalflength);

  // create the gap between the ID and the HGTD endcap volumes
  auto negativeInnerGapTrans = std::make_unique<Amg::Transform3D>(Amg::Translation3D(Amg::Vector3D(0.,0.,-zGapPos)));
  auto negativeInnerGapBounds = std::make_shared<Trk::CylinderVolumeBounds>(enclosedInnerRadius,enclosedOuterRadius,gapHalfLengthZ);

  Trk::TrackingVolume * negativeInnerGapVolume =
      new Trk::TrackingVolume(std::move(negativeInnerGapTrans),
                              negativeInnerGapBounds,
                              *materialProperties,
                              nullptr, nullptr,
                              m_layerBuilder->identification()+"::NegativeInnerGap");

  auto positiveInnerGapTrans = std::make_unique<Amg::Transform3D>(Amg::Translation3D(Amg::Vector3D(0.,0.,zGapPos)));
  auto positiveInnerGapBounds = std::make_shared<Trk::CylinderVolumeBounds>(enclosedInnerRadius,enclosedOuterRadius,gapHalfLengthZ);

  Trk::TrackingVolume * positiveInnerGapVolume =
       new Trk::TrackingVolume(std::move(positiveInnerGapTrans),
                               std::move(positiveInnerGapBounds),
                               *materialProperties,
                               nullptr, nullptr,
                               m_layerBuilder->identification()+"::PositiveInnerGap");

  // create dummy inner volume if not built already
  if (not innerVol) {
    auto idBounds = std::make_shared<Trk::CylinderVolumeBounds>(enclosedInnerRadius,
                                                                enclosedInnerSectorHalflength);
    auto idTr = std::make_unique<Amg::Transform3D>(Trk::s_idTransform);

    innerVol = new Trk::TrackingVolume(std::move(idTr), std::move(idBounds), *materialProperties,
                                       nullptr, nullptr,
                                       "HGTD::GapVolumes::DummyID");
  }

  std::vector<Trk::TrackingVolume*> inBufferVolumes;
  inBufferVolumes.push_back(negativeInnerGapVolume);
  inBufferVolumes.push_back(innerVol);
  inBufferVolumes.push_back(positiveInnerGapVolume);

  Trk::TrackingVolume* inDetEnclosed =
    m_trackingVolumeCreator->createContainerTrackingVolume(inBufferVolumes,
                                                           *materialProperties,
                                                           "HGTD::Container::EnclosedInnerDetector");

  // create the tracking volumes
  // create the three volumes
  Trk::TrackingVolume* negativeVolume =
    m_trackingVolumeCreator->createTrackingVolume(negativeLayers,
                                                  *materialProperties,
                                                  enclosedInnerRadius, enclosedOuterRadius,
                                                  -maxZ_HGTD, -minZ_HGTD,
                                                  m_layerBuilder->identification()+"::NegativeEndcap",
                                                  (Trk::BinningType)m_layerBinningType);


  Trk::TrackingVolume* positiveVolume =
    m_trackingVolumeCreator->createTrackingVolume(positiveLayers,
                                                  *materialProperties,
                                                  enclosedInnerRadius, enclosedOuterRadius,
                                                  minZ_HGTD, maxZ_HGTD,
                                                  m_layerBuilder->identification()+"::PositiveEndcap",
                                                  (Trk::BinningType)m_layerBinningType);

  // the base volumes have been created
  ATH_MSG_VERBOSE('\t' << '\t'<< "Volumes have been created, now pack them into a triple.");
  negativeVolume->registerColorCode(m_colorCodeConfig);
  inDetEnclosed->registerColorCode(m_colorCodeConfig);
  positiveVolume->registerColorCode(m_colorCodeConfig);

  // pack them together
  std::vector<Trk::TrackingVolume*> tripleVolumes;
  tripleVolumes.push_back(negativeVolume);
  tripleVolumes.push_back(inDetEnclosed);
  tripleVolumes.push_back(positiveVolume);

  // create the tiple container
  Trk::TrackingVolume* tripleContainer =
    m_trackingVolumeCreator->createContainerTrackingVolume(tripleVolumes,
                                                           *materialProperties,
                                                           "HGTD::Containers::" + m_layerBuilder->identification(),
                                                           m_buildBoundaryLayers,
                                                           m_replaceJointBoundaries);

  ATH_MSG_VERBOSE( '\t' << '\t'<< "Created container volume with bounds: " << tripleContainer->volumeBounds() );

  // finally create the two endplates: negative
  Trk::TrackingVolume* negativeEnclosure = m_trackingVolumeCreator->createGapTrackingVolume(
    *materialProperties,
    enclosedInnerRadius,
    enclosedOuterRadius,
    -maxZ_HGTDEnclosure,
    -maxZ_HGTD,
    1,
    false,
    "HGTD::Gaps::NegativeEnclosure" + m_layerBuilder->identification());

  // finally create the two endplates: positive
  Trk::TrackingVolume* positiveEnclosure = m_trackingVolumeCreator->createGapTrackingVolume(
    *materialProperties,
    enclosedInnerRadius,
    enclosedOuterRadius,
    maxZ_HGTD,
    maxZ_HGTDEnclosure,
    1,
    false,
    "HGTD::Gaps::PositiveEnclosure" + m_layerBuilder->identification());
  // and the final tracking volume
  std::vector<Trk::TrackingVolume*> enclosedVolumes;
  enclosedVolumes.push_back(negativeEnclosure);
  enclosedVolumes.push_back(tripleContainer);
  enclosedVolumes.push_back(positiveEnclosure);

   Trk::TrackingVolume* enclosedDetector =
      m_trackingVolumeCreator->createContainerTrackingVolume(enclosedVolumes,
                                                             *materialProperties,
                                                             "HGTD::Detectors::"+m_layerBuilder->identification(),
                                                             m_buildBoundaryLayers,
                                                             m_replaceJointBoundaries);

  ATH_MSG_VERBOSE( '\t' << '\t'<< "Created enclosed HGTD volume with bounds: " << enclosedDetector->volumeBounds() );

  //  create the TrackingGeometry ------------------------------------------------------
  auto hgtdTrackingGeometry = std::make_unique<Trk::TrackingGeometry>(enclosedDetector);

  if (m_indexStaticLayers and hgtdTrackingGeometry)
   hgtdTrackingGeometry->indexStaticLayers( geometrySignature() );
  if (msgLvl(MSG::VERBOSE) && hgtdTrackingGeometry)
    hgtdTrackingGeometry->printVolumeHierarchy(msg(MSG::VERBOSE));

  return hgtdTrackingGeometry;
}
