/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOREC_CALOCLUSTERTIMINGFILTER_H
#define CALOREC_CALOCLUSTERTIMINGFILTER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"

/**
  @class CaloClusterTimingFilter

  @brief Creates a new calo cluster collection with timing cuts applied.
         Makes a deep copy of the original clusters.
  */
class CaloClusterTimingFilter : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~CaloClusterTimingFilter() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_inputKey{
    this, "InputClusters", "",
    "Input CaloCluster container"
  };

  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputKey{
    this, "OutputClusters", "",
    "Filtered CaloCluster container"
  };

  SG::WriteHandleKey<CaloClusterCellLinkContainer> m_cellLinkKey{
    this, "OutputCellLinkName", "",
    "Output cell-link container"
  };

  Gaudi::Property<double> m_minTime{
    this, "MinTime", -999,
    "Minimum accepted cluster time"
  };

  Gaudi::Property<double> m_maxTime{
    this, "MaxTime", +999,
    "Maximum accepted cluster time"
  };

  Gaudi::Property<double> m_cellSignificanceThreshold{
    this, "CellSignificance", -1, // by default ignored
    "Timing cut is ignored if maximum cell significance is above this value"
  };

  Gaudi::Property<double> m_badLArQFracThreshold{
    this, "BadLArQFracThreshold", -1, // by default ignored
    "Timing cut is ignored if LArQ fraction is above this value"
  };

};

#endif // CALOREC_CALOCLUSTERTIMINGFILTER_H
