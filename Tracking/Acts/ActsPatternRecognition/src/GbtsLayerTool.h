/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_GBTSLAYERTOOL_H
#define ACTSTRK_GBTSLAYERTOOL_H

// Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IGbtsLayerTool.h"

#include <Gaudi/Property.h>

// Acts Core
#include "Acts/Seeding/GbtsLayerConnection.hpp"

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
///
/// Also includes helper functions to read in layer connection table, using
/// the layer geometry, so that both GPU and CPU GBTS can use the same readers
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

  /// Reads the connection table and keeps the connections this pass is for:
  /// both layers have to be layers this detector has, of one technology,
  /// and that technology has to be one the pass asked for.
  /// @param connections filled with the layer pairs, in stage order
  /// @param etaBinWidth filled with the eta bin width the table was made for
  StatusCode readConnections(
    const std::vector<Acts::Experimental::GbtsLayerDescription>& layers,
    std::vector<Acts::Experimental::GbtsLayerConnection>& connections,
    float& etaBinWidth, std::string connectorInputFile, bool usePixel, bool useStrips) const override;

 private:
  /// One module, as grouped into a layer.
  struct ModuleEntry {
    short phiIndex{};
    short etaIndex{};
    int hash{};
  };

  /// One row of the table: the two layers it connects, and the stage it is in.
  struct Connection {
    std::uint32_t stage{};
    std::uint32_t src{};
    std::uint32_t dst{};
  };

  /// The table as it is on file.
  struct ReadResult {
    /// The eta bin width the table was made for.
    float etaBinWidth{};
    /// The connections, in file order.
    std::vector<Connection> connections;
  };

  /// Read the GBTS layer connection table.
  ///
  /// The whole table, as written: which of it applies to the detector and to
  /// the pass at hand is the caller's to decide. The bin table each connection
  /// carries is skipped, GBTS recomputes it from the layer geometry.
  ///
  /// @param inputStream the table
  /// @throws std::runtime_error if the table cannot be read
  /// @return the table
  ReadResult read(std::istream& inputStream) const;

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
  /// Wafer hash addressable dense GBTS layer indices.
  std::vector<short> m_pixelLayers, m_stripLayers;
};

}  // namespace ActsTrk

#endif
