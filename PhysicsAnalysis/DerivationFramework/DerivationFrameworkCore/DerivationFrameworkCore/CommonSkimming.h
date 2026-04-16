// CommonSkimming.h
#ifndef COMMON_SKIMMING_H
#define COMMON_SKIMMING_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

namespace DerivationFramework {

class CommonSkimming : public AthReentrantAlgorithm {
public:
  CommonSkimming(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~CommonSkimming() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  ToolHandleArray<ISkimmingTool> m_skimTools{this, "SkimmingTools", {}, 
    "Array of ISkimmingTools; the event is kept only if ALL of them return true from eventPassesFilter()"};
};

}

#endif

