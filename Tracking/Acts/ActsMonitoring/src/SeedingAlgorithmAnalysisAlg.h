/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKALGS_SEEDINGALGORITHMANALYSISALG_H
#define ACTSTRKALGS_SEEDINGALGORITHMANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "InDetRecToolInterfaces/ISiSpacePointsSeedMaker.h"

namespace ActsTrk {
  class SeedingAlgorithmAnalysisAlg final: public AthMonitorAlgorithm {

  public:
    SeedingAlgorithmAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~SeedingAlgorithmAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    ToolHandleArray<InDet::ISiSpacePointsSeedMaker> m_seedingTools{this,
      "SeedingTools", {}, "List of seeding tools to test"};

    StringArrayProperty m_monitoringGroupNames{this,
      "MonitorNames", {}, "List of names of the monitoring groups"};

    Gaudi::Property< bool > m_doPixel {this, "DoPixel", true};
    Gaudi::Property< bool > m_doStrip {this, "DoStrip", true};

    enum TimeMonitoringType : int {
      StripSeedInitialisation = 0,
      PixelSeedInitialisation,
      StripSeedProduction,
      PixelSeedProduction,
      AllTypes
    };

  };

}

#endif // ACTSTRKALGS_SEEDINGALGORITHMANALYSISALG_H
