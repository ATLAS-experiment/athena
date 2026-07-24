/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkDetDescrTools/LayerProviderImpl.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkGeometry/Layer.h"


const std::vector<Trk::Layer*>
Trk::LayerProviderImpl::cylindricalLayersToCentralLayers(std::unique_ptr<const std::vector<Trk::CylinderLayer*> > cylinderLayers) 
{
  // central layers
  std::vector<Trk::Layer*> cLayers;
  // loop over cylinderLayers and push into the return vector;
  if (cylinderLayers) {
    for (Trk::CylinderLayer* cL : (*cylinderLayers))
      cLayers.push_back(cL);
  }
  // and return
  return cLayers;
}


std::pair<const std::vector<Trk::Layer*>, const std::vector<Trk::Layer*> >
  Trk::LayerProviderImpl::discLayersToEndcapLayers(std::unique_ptr<const std::vector<Trk::DiscLayer*> > discLayers) 
{

  // get the disc layers
  std::vector<Trk::Layer*> dLayers_pos;
  std::vector<Trk::Layer*> dLayers_neg;
  // loop and fill either dLayers
  if (discLayers) {
    // loop over and push into the return/cache vector
    for (Trk::DiscLayer* dL : (*discLayers)) {
      // get the center posituion
      double zpos = dL->surfaceRepresentation().center().z();
      if (zpos > 0.)
          dLayers_pos.push_back(dL);
      else
          dLayers_neg.push_back(dL);
    }
  }
  // and return
  return std::make_pair(dLayers_pos, dLayers_neg);
}



