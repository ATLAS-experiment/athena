/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// InDet
#include "InDetTrackingGeometry/StagedTrackingGeometryBuilderCond.h"
#include "InDetTrackingGeometry/DiscOverlapDescriptor.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
// Trk Geometry stuff
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/BinnedArray1D1D.h"
#include "TrkVolumes/VolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkSurfaces/DiscBounds.h"
//Athena
#include "CxxUtils/checker_macros.h"
#include "AthenaKernel/IOVInfiniteRange.h"
//Gaudi
#include "GaudiKernel/SystemOfUnits.h"
#include <iterator> //std::advance

// constructor
InDet::StagedTrackingGeometryBuilderCond::StagedTrackingGeometryBuilderCond(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
  // layer builders and their configurations
  declareProperty("LayerBuilders",                    m_layerProviders);
}

// Athena standard methods
// initialize
StatusCode InDet::StagedTrackingGeometryBuilderCond::initialize()
{

  // Retrieve the layer builders -----------------------------------------------------------
  if (m_layerProviders.retrieve().isFailure())
    {
      ATH_MSG_FATAL( "Failed to retrieve tool(s) " << m_layerProviders );
      return StatusCode::FAILURE;
    } else
    ATH_MSG_DEBUG( "Retrieved tool " << m_layerProviders );

  return StagedTrackingGeometryBuilderImpl::initialize();
}


//FIXME: ctx, tVolPair not used yet, range not created
std::unique_ptr<Trk::TrackingGeometry>
InDet::StagedTrackingGeometryBuilderCond::trackingGeometry
(const EventContext& ctx,
 Trk::TrackingVolume* /*tVol*/,
 SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const
{
  // only one assumption:
  // layer builders are ordered in increasing r
  ATH_MSG_DEBUG( "[ Start building the ID TrackingGeometry. ]");
  ATH_MSG_DEBUG( "[ STEP 0 ] : Getting overal dimensions from DetectorEnvelope service." );

  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  // The Overall Geometry

  // get the dimensions from the envelope service
  const RZPairVector& envelopeDefs = m_enclosingEnvelopeSvc->getInDetRZBoundary();
  ATH_MSG_VERBOSE("       -> retrieved Inner Detector envelope definitions at size " << envelopeDefs.size());
  double envelopeVolumeRadius = envelopeDefs[1].first;
  double envelopeVolumeHalfZ  = fabs(envelopeDefs[1].second);

  if (m_removeHGTD) {
    // If running with the HGTD, we don't want to include its volume
    // as it will be included in another tracking geometry.
    // re-evaluating the ID envelope dimension
    envelopeVolumeHalfZ = m_zMinHGTD;
  }

  ATH_MSG_VERBOSE("       -> envelope R/Z defined as : " << envelopeVolumeRadius << " / " << envelopeVolumeHalfZ );

  ATH_MSG_DEBUG( "[ STEP 1 ] : Getting overall dimensions from the different layer builders." );
  size_t ilS = 0;
  double maximumLayerExtendZ   = 0.;
  double maximumLayerRadius    = 0.;
  std::vector<InDet::LayerSetup> layerSetups;
  for ( const auto& lProvider : m_layerProviders){
    // screen output
    ATH_MSG_DEBUG( "[ LayerBuilder : '" << lProvider->identification() << "' ] being processed. " );
    // retrieve the layers
    std::vector<Trk::Layer*> centralLayers = lProvider->centralLayers(ctx, whandle);
    std::pair<const std::vector<Trk::Layer*>, const std::vector<Trk::Layer*> > endcapLayersPair = lProvider->endcapLayer(ctx, whandle);
    ATH_MSG_INFO("       -> retrieved "  << centralLayers.size()  << " central layers.");
    ATH_MSG_INFO("       -> retrieved "  << endcapLayersPair.second.size() << " layers on negative side.");
    ATH_MSG_INFO("       -> retrieved "  << endcapLayersPair.first.size() << " layers on positive side.");
    // getting the Layer setup from parsing the builder output
    InDet::LayerSetup lSetup =
      estimateLayerSetup(lProvider->identification(),
                         ilS,
                         endcapLayersPair.second,
                         centralLayers,
                         endcapLayersPair.first,
                         envelopeVolumeRadius,
                         envelopeVolumeHalfZ);
    // get the maxima - for R and Z
    takeBigger(maximumLayerRadius, lSetup.rMax);
    takeBigger(maximumLayerExtendZ, lSetup.zMax);
    //layer setups for the second run
    layerSetups.push_back(std::move(lSetup));
    // increase counter
    ++ilS;
  }
  ATH_MSG_VERBOSE("       -> layer max R/Z defined as : " << maximumLayerRadius << " / " << maximumLayerExtendZ );
   return trackingGeometryImpl(layerSetups, maximumLayerExtendZ,
                               maximumLayerRadius, envelopeVolumeHalfZ,
                               envelopeVolumeRadius);
}


