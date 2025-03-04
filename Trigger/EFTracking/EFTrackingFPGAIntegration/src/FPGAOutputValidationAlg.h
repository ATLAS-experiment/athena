/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_ATHENA_VS_FPGA_CLUSTER_HIST_MAKER_H
#define EFTRACKING_ATHENA_VS_FPGA_CLUSTER_HIST_MAKER_H

#include "GaudiKernel/LockedHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

class FPGAOutputValidationAlg : public AthReentrantAlgorithm
{
  Gaudi::Property<std::string> m_outputRootFilePath{
    this,
    "outputRootFilePath", 
    "fpgaOutputValidation.root", 
    "Path to output root file containing histograms."
  };

  SG::ReadHandleKeyArray<xAOD::PixelClusterContainer> m_pixelKeys{this, "pixelKeys", {}};
  SG::ReadHandleKeyArray<xAOD::StripClusterContainer> m_stripKeys{this, "stripKeys", {}};
  
  // Hide histograms behind a shared lock for thread safe mutability
  mutable std::vector<LockedHandle<TH1>> m_pixelXSharedLocks ATLAS_THREAD_SAFE {};
  mutable std::vector<LockedHandle<TH1>> m_pixelYSharedLocks ATLAS_THREAD_SAFE {};
  mutable std::vector<LockedHandle<TH1>> m_pixelZSharedLocks ATLAS_THREAD_SAFE {};

  mutable std::vector<LockedHandle<TH1>> m_stripXSharedLocks ATLAS_THREAD_SAFE {};
  mutable std::vector<LockedHandle<TH1>> m_stripYSharedLocks ATLAS_THREAD_SAFE {};
  mutable std::vector<LockedHandle<TH1>> m_stripZSharedLocks ATLAS_THREAD_SAFE {};

  ServiceHandle<ITHistSvc> m_tHistSvc{this, "THistSvc", "THistSvc"};

 public:
  FPGAOutputValidationAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
};

#endif

