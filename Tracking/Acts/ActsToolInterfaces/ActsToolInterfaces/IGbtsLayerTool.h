/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IGBTSLAYERTOOL_H
#define ACTSTOOLINTERFACES_IGBTSLAYERTOOL_H

// Athena
#include "GaudiKernel/IAlgTool.h"

// ACTS
#include "Acts/Seeding/GbtsLayerDescription.hpp"

#include <vector>

namespace ActsTrk {

/// Builds the GBTS layer geometry out of the ITk readout geometry.
///
/// GBTS groups the silicon modules into layers of its own and addresses them
/// by a dense index. This tool owns that grouping: it hands out the layer
/// descriptions in dense-index order, and the wafer hash to dense index maps
/// that turn a space point's module into the layer it belongs to.
class IGbtsLayerTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IGbtsLayerTool, 1, 0);

  /// Value stored in the hash maps for a module GBTS does not use.
  static constexpr short kNoLayer = -100;

  /// The GBTS layers, indexed by the dense GBTS layer index.
  virtual const std::vector<Acts::Experimental::GbtsLayerDescription>&
  layerDescriptions() const = 0;

  /// Pixel wafer hash to dense GBTS layer index, `kNoLayer` where unused.
  virtual const std::vector<short>& pixelLayers() const = 0;

  /// Strip wafer hash to dense GBTS layer index, `kNoLayer` where unused.
  virtual const std::vector<short>& stripLayers() const = 0;
};

}  // namespace ActsTrk

#endif
