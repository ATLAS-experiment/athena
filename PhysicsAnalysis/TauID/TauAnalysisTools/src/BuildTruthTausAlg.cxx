#include "BuildTruthTausAlg.h"
#include "GaudiKernel/EventContext.h"

namespace TauAnalysisTools {

  BuildTruthTausAlg::BuildTruthTausAlg(const std::string& name, ISvcLocator* svcLoc)
    : AthReentrantAlgorithm(name, svcLoc)
  {}

  StatusCode BuildTruthTausAlg::initialize()
  {
    ATH_CHECK(m_buildTruthTaus.retrieve());
    return StatusCode::SUCCESS;
  }

  StatusCode BuildTruthTausAlg::execute(const EventContext& ctx) const
  {
    using ITaus = TauAnalysisTools::IBuildTruthTaus::ITruthTausEvent;

    TauAnalysisTools::BuildTruthTaus::TruthTausEvent taus;

    ATH_CHECK(m_buildTruthTaus->retrieveTruthTaus(taus, ctx));

    return StatusCode::SUCCESS;
  }

}
