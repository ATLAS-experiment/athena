/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oliver Majersky
/// @author Baptiste Ravina

#ifndef KLFITTERNANALYSISALGORITHMS_KLFITTERFINALIZEOUTPUTALG_H_
#define KLFITTERNANALYSISALGORITHMS_KLFITTERFINALIZEOUTPUTALG_H_

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>

#include "KLFitterAnalysisAlgorithms/KLFitterResultAuxContainer.h"
#include "KLFitterAnalysisAlgorithms/KLFitterResultContainer.h"

namespace EventReco {
class KLFitterFinalizeOutputAlg final : public EL::AnaAlgorithm {
 public:
  KLFitterFinalizeOutputAlg(const std::string &name, ISvcLocator *pSvcLocator);
  virtual StatusCode initialize() final;
  virtual StatusCode execute() final;

 private:
  CP::SysListHandle m_systematicsList{this};

  CP::SysReadHandle<xAOD::KLFitterResultContainer> m_inHandle{
      this, "resultContainerToCheck", "KLFitterResult_%SYS%",
      "the input KLFitterResultContainer to check for existence"};
  CP::SysWriteHandle<xAOD::KLFitterResultContainer,
                     xAOD::KLFitterResultAuxContainer>
      m_outHandle{this, "resultContainerToWrite", "KLFitterResult_%SYS%",
                  "the output KLFitterResultContainer"};
};
}  // namespace EventReco

#endif
