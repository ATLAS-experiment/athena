// CommonSkimming.cxx
#include "DerivationFrameworkCore/CommonSkimming.h"

namespace DerivationFramework {

CommonSkimming::CommonSkimming(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode CommonSkimming::initialize()
{
  ATH_CHECK(m_skimTools.retrieve());
  if (m_skimTools.empty()) {
    ATH_MSG_WARNING("No SkimmingTools configured – every event will be accepted");
  }
  return StatusCode::SUCCESS;
}

StatusCode CommonSkimming::execute(const EventContext& ctx) const
{
  bool passes = true;

  for (const auto& tool : m_skimTools) {
    if (!tool->eventPassesFilterCtx(ctx)) {
      passes = false;
      ATH_MSG_DEBUG("Event rejected by skimming tool \"" << tool->name() 
                    << "\" → sequence will stop");
      break;   // early exit – no need to check the remaining tools
    } else {
      ATH_MSG_DEBUG("Event accepted by skimming tool \"" << tool->name() << "\"");
    }
  }

  setFilterPassed(passes, ctx);   // ← this tells AthSequencer to stop the rest of the sequence

  return StatusCode::SUCCESS;   // always return SUCCESS; the filter flag does the veto
}

}

