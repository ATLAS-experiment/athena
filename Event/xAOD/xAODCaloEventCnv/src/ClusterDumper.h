// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODCREATORALGS_CLUSTERDUMPER_H
#define XAODCREATORALGS_CLUSTERDUMPER_H

// System include(s):
#include <string>
#include <fstream>
#include <mutex>

// Athena/Gaudi include(s):
#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

class ClusterDumper : public AthAlgorithm {

public:
  /// Delegate algorithm constructor
  using AthAlgorithm::AthAlgorithm;

  /// Function initialising the algorithm
  virtual StatusCode initialize();
  /// Function executing the algorithm
  virtual StatusCode execute();

  virtual StatusCode finalize();

  

private:
  /// The key for the output xAOD::CaloClusterContainer
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_containerName{this,"ContainerName","CaloCalTopoClusters"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EvtInfo", "EventInfo", "EventInfo name"};

  Gaudi::Property<std::string> m_fileName{this,"FileName",{}};
  Gaudi::Property<bool> m_printCellLinks{this,"PrintCellLinks",true};
  Gaudi::Property<bool> m_reducedPrecision{this,"ReducedPrecision",false,
  "If true, use less precision in the output, to reduce exposure to FP rounding issues when comparing with a reference."};

  std::ostream* m_out=&std::cout;
  std::ofstream m_fileOut;
  
  std::mutex m_fileMutex;

}; // class ClusterDumper

#endif // XAODCREATORALGS_CLUSTERCREATOR_H
