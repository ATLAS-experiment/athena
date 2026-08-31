/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKDETDESCRTOOLS_LAYERPROVIDERCOND_H
#define TRKDETDESCRTOOLS_LAYERPROVIDERCOND_H

// Trk
#include "TrkDetDescrTools/LayerProviderImpl.h"
#include "TrkDetDescrInterfaces/ILayerProviderCond.h"
#include "TrkDetDescrInterfaces/ILayerBuilderCond.h"
// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk {

  class Layer;

/** @class LayerProviderCond

  Wrapper around an ILayerBuilderCond to feed into the StagedGeometryBuilderCond

  @author Andreas.Salzburger@cern.ch
 */
class LayerProviderCond final
  : public extends<LayerProviderImpl, ILayerProviderCond>
{

public:
  /** Constructor */
  using base_class::base_class;

  /** initialize */
  virtual StatusCode initialize() override final;

  /** LayerBuilder interface method - returning the layers in the endcaps */
  virtual std::pair<const std::vector<Layer*>, const std::vector<Layer*> >
  endcapLayer(const EventContext& ctx,
              SG::WriteCondHandle<TrackingGeometry>& whandle) const override final;

  /** LayerBuilder interface method - returning the central layers */
  virtual const std::vector<Layer*>
  centralLayers(const EventContext& ctx,
                SG::WriteCondHandle<TrackingGeometry>& whandle) const override final;

  /** Name identification */
  virtual const std::string& identification() const override final;

private:
  PublicToolHandle<ILayerBuilderCond> m_layerBuilder{this, "LayerBuilder", ""};  // Name specification from outside
};

} // end of namespace

#endif // TRKDETDESCRTOOLS_LAYERPROVIDERCOND_H

