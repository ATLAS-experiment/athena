/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Trk include
#include "TrkDetDescrTools/LayerProviderCond.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkGeometry/Layer.h"



// initialize
StatusCode
Trk::LayerProviderCond::initialize()
{
  if (m_layerBuilder.retrieve().isFailure()) {
    ATH_MSG_WARNING("Could not retrieve layer builder");
  }
  return StatusCode::SUCCESS;
}

/** LayerBuilderCond interface method - returning the central layers */
const std::vector<Trk::Layer*>
Trk::LayerProviderCond::centralLayers(const EventContext& ctx,
                                      SG::WriteCondHandle<TrackingGeometry>& whandle) const
{
  // retrieving the cylinder layers from the layer builder
  std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
    cylinderLayers = m_layerBuilder->cylindricalLayers(ctx, whandle);
  return cylindricalLayersToCentralLayers(std::move(cylinderLayers));
}

/** LayerBuilderCond interface method - returning the layers at negative side */
std::pair<const std::vector<Trk::Layer*>, const std::vector<Trk::Layer*> >
Trk::LayerProviderCond::endcapLayer(const EventContext& ctx,
                                    SG::WriteCondHandle<TrackingGeometry>& whandle) const
{
  // retrieving the cylinder layers from the layer builder
  std::unique_ptr<const std::vector<Trk::DiscLayer*> > discLayers =
    m_layerBuilder->discLayers(ctx, whandle);
  return discLayersToEndcapLayers(std::move(discLayers));
}

const std::string&
Trk::LayerProviderCond::identification() const
{
  return m_layerBuilder->identification();
}
