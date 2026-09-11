/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGNN_GNNPIPELINETOOL_H
#define ACTSGNN_GNNPIPELINETOOL_H

#include <cstdint>
#include <memory>
#include <optional>
#include <semaphore>
#include <string>
#include <vector>

#include "ActsToolInterfaces/IGnnPipelineTool.h"
#include "ActsInterop/Logger.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ServiceHandle.h"

class PixelID;
class SCT_ID;

namespace ActsPlugins {
class GnnPipeline;
}  // namespace ActsPlugins


namespace ActsTrk {

class GnnPipelineTool : public extends<AthAlgTool, IGnnPipelineTool> {
 public:
  GnnPipelineTool(const std::string& type, const std::string& name,
                  const IInterface* parent);
  virtual ~GnnPipelineTool();

  StatusCode initialize() override;

  StatusCode buildSeed(
      const std::vector<const xAOD::SpacePointContainer*>&
          spacePointCollections,
      ActsTrk::SeedContainer& seeds) const override;

 private:
  Gaudi::Property<std::string> m_moduleMapPath{this, "moduleMapPath", "",
                                               "Path to the module map files"};
  Gaudi::Property<std::string> m_gnnPath{this, "gnnPath", "",
                                         "Path to the gnn file"};
  Gaudi::Property<bool> m_usePhiOverlapSps{
      this, "usePhiOverlapSps", false,
      "Whether to use phi overlap spacepoints"};
  Gaudi::Property<unsigned int> m_maxGpuInstances{
      this, "maxGpuInstances", 1,
      "Number of events that can be on GPU in parallel"};
  Gaudi::Property<unsigned int> m_numTrtContexts{
      this, "numTrtContexts", 1, "Number of TensorRT contexts to allocate"};
  Gaudi::Property<double> m_edgeCut{this, "edgeCut", 0.5,
                                    "Edge cut to apply after the GNN"};
  Gaudi::Property<unsigned int> m_minCandidateMeasurements{
      this, "minCandidateMeasurements", 7,
      "Minimum number of spacepoints to cut for in the GNN candidates"};
  Gaudi::Property<int> m_cudaDeviceIndex{this, "cudaDeviceIndex", 0,
                                         "CUDA device index for GNN inference"};

  const Acts::Logger& logger() const { return *m_logger; }
  std::unique_ptr<const Acts::Logger> m_logger;

  ServiceHandle<IChronoStatSvc> m_chronoSvc{"ChronoStatSvc", name()};

  const PixelID* m_pixelIdHelper{nullptr};
  const SCT_ID* m_stripIdHelper{nullptr};

  StatusCode buildFeatures(
      const std::vector<const xAOD::SpacePointContainer*>&
          spacePointCollections,
      std::vector<float>& features, std::vector<std::uint64_t>& moduleIds,
      std::vector<int>& ids,
      std::vector<const xAOD::SpacePoint*>& allSPPtrs,
      std::size_t nFeatures = 12) const;

  std::unique_ptr<ActsPlugins::GnnPipeline> m_gnnPipeline;
  mutable std::optional<std::counting_semaphore<>> m_gpuInstanceCount
      ATLAS_THREAD_SAFE{};
};

}  // namespace ActsTrk

#endif
