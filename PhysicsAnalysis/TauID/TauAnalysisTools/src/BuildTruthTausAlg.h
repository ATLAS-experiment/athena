#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "TauAnalysisTools/BuildTruthTaus.h"

namespace TauAnalysisTools {

  class BuildTruthTausAlg : public AthReentrantAlgorithm {
  public:
    BuildTruthTausAlg(const std::string& name, ISvcLocator* svcLoc);

    StatusCode initialize() override;
    StatusCode execute(const EventContext&) const override;

  private:
    ToolHandle<TauAnalysisTools::BuildTruthTaus> m_buildTruthTaus{
      this, "BuildTruthTaus", "", "BuildTruthTaus tool"
        };
  };

}
