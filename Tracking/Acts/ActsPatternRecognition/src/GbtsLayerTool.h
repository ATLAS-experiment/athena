/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_GBTSLAYERTOOL_H
#define ACTSTRK_GBTSLAYERTOOL_H

// Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IGbtsLayerTool.h"

#include <Gaudi/Property.h>

#include <map>
#include <tuple>
#include <vector>

class SCT_ID;
class PixelID;

namespace InDetDD {
class PixelDetectorManager;
class SCT_DetectorManager;
}  // namespace InDetDD

namespace ActsTrk {

/// Builds the GBTS layer geometry from the ITk readout geometry.
///
/// Ported from the trigger's `TrigL2LayerNumberToolITk` so that the ACTS GBTS
/// seeding does not have to reach into the trigger for its geometry. The
/// grouping is unchanged - modules are keyed by (side, technology, volume,
/// layer) and each group becomes one GBTS layer - but the result is handed out
/// as `Acts::Experimental::GbtsLayerDescription` directly, with the layer's
/// technology filled in rather than left to be decoded from its id.
class GbtsLayerTool final : public extends<AthAlgTool, IGbtsLayerTool> {
 public:
  GbtsLayerTool(const std::string& type, const std::string& name,
                const IInterface* parent);
  virtual ~GbtsLayerTool() = default;

  virtual StatusCode initialize() override;

  virtual const std::vector<Acts::Experimental::GbtsLayerDescription>&
  layerDescriptions() const override {
    return m_layerDescriptions;
  }

  virtual const std::vector<short>& pixelLayers() const override {
    return m_pixelLayers;
  }

  virtual const std::vector<short>& stripLayers() const override {
    return m_stripLayers;
  }

  virtual const std::vector<GbtsTechnology>& layerTechnologies() const override {
    return m_layerTechnologies;
  }

 private:
  /// One module, as grouped into a layer.
  struct ModuleEntry {
    short phiIndex{};
    short etaIndex{};
    int hash{};
  };

  /// Layer key: (side, technology, volume id, layer id). The side is the
  /// barrel/endcap code, with the barrel remapped so that it sorts first.
  using LayerKey = std::tuple<int, int, short, short>;

  /// Group the modules into layers and fill the descriptions and hash maps.
  StatusCode buildLayers();

  Gaudi::Property<bool> m_dumpGeometry{this, "dumpGbtsGeometry", false,
                                       "dump the layer geometry to a file"};
  Gaudi::Property<std::string> m_geometryDumpDir{
      this, "geometryDump", "", "file to dump the layer geometry to"};

  const PixelID* m_pixelId{nullptr};
  const SCT_ID* m_stripId{nullptr};
  const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
  const InDetDD::SCT_DetectorManager* m_stripManager{nullptr};

  std::vector<Acts::Experimental::GbtsLayerDescription> m_layerDescriptions;
  /// What each layer is made of, parallel to m_layerDescriptions.
  std::vector<GbtsTechnology> m_layerTechnologies;
  /// Wafer hash addressable dense GBTS layer indices.
  std::vector<short> m_pixelLayers, m_stripLayers;
};

}  // namespace ActsTrk

#endif
