/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSHGTDLAYERBUILDER_H
#define ACTSGEOMETRY_ACTSHGTDLAYERBUILDER_H

// PACKAGE
#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometry/ActsElementVector.h"

// ATHENA
#include "CxxUtils/checker_macros.h"
#include <mutex>

// ACTS
#include "Acts/Geometry/ILayerBuilder.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/BinningType.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Definitions/Units.hpp"

class ActsTrackingGeomtrySvc;

class HGTD_ID;
class HGTD_DetectorManager;
class ActsDetectorElement;

namespace Acts {
class Surface;
class LayerCreator;
}

/// @class ActsHGTDLayerBuilder
class ActsHGTDLayerBuilder : public Acts::ILayerBuilder
{
public:

  using ElementVector = ActsElementVector;

  /// @struct Config
  /// nested configuration struct for steering of the layer builder

  struct Config
  {
    /// string based identification
    std::string                          configurationName = "undefined";
    const HGTD_DetectorManager*   mng {nullptr};
    const HGTD_ID* idHelper = nullptr;
    std::shared_ptr<const Acts::LayerCreator> layerCreator = nullptr;
    Acts::BinningType bTypePhi = Acts::equidistant;
    /// the binning type of the contained surfaces in r
    /// (equidistant/arbitrary)
    Acts::BinningType bTypeR = Acts::equidistant;
    /// the binning type of the contained surfaces in z
    /// (equidistant/arbitrary)
    Acts::BinningType bTypeZ = Acts::equidistant;

    std::shared_ptr<ElementVector> elementStore {nullptr};

    std::array<double, 2> endcapEnvelopeR = {50 * Acts::UnitConstants::mm,
                                                 50 * Acts::UnitConstants::mm}; // m_discEnvelopeR in athena/HighGranularityTimingDetector/HGTD_Reconstruction/HGTD_TrackingGeometry/src/HGTD_LayerBuilderCond.cxx
    std::array<double, 2> endcapEnvelopeZ = {0.2 * Acts::UnitConstants::mm,
                                                 0.2 * Acts::UnitConstants::mm}; // m_discThickness

    std::pair<size_t, size_t> endcapMaterialBins = {20, 50}; //{Phi, R}

    bool objDebugOutput = false;

    /// controls how many bins are created for the sensitive surface grid.
    /// 1 results in the same number of bins as there are surfaces per layer in each dimension.
    /// using a higher number will reduce the number of surfaces per bin, thus speeding up navigation, but increasing memory consumption.
    double numberOfBinsFactor = 5.0;
  };

  /// Constructor
  /// @param cfg is the configuration struct
  /// @param logger the local logging instance

  ActsHGTDLayerBuilder(const Config&                cfg,
                    std::unique_ptr<const Acts::Logger> logger);

  /// Destructor
  virtual ~ActsHGTDLayerBuilder() = default;

  virtual const Acts::LayerVector
  negativeLayers(const Acts::GeometryContext& gctx) const override;

  virtual const Acts::LayerVector
  centralLayers(const Acts::GeometryContext& gctx) const override;

  virtual const Acts::LayerVector
  positiveLayers(const Acts::GeometryContext& gctx) const override;

  virtual
  const std::string&
  identification() const override
  {
    return m_cfg.configurationName;
  }

private:
  /// configuration object
  Config m_cfg;

  /// Private access to the logger
  const Acts::Logger&
  logger() const
  {
    return *m_logger;
  }

  std::vector<std::shared_ptr<const ActsDetectorElement>>
  getDetectorElements() const;

  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger;

  // Private helper method : build layers
  // @param layers is goint to be filled
  // @param type is the indication which ones to build -1 | 0 | 1
  void
  buildEndcap(const Acts::GeometryContext& gctx, Acts::LayerVector& layersOutput, int type = 0) const;

};

#endif
