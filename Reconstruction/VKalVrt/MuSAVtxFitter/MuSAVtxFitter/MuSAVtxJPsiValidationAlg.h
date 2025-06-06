#ifndef MuSAVtxJPsiValidationAlg_H
#define MuSAVtxJPsiValidationAlg_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "JpsiUpsilonTools/JpsiFinder.h"

namespace Analysis { class JpsiFinder; }

namespace Rec{
class MuSAVtxJPsiValidationAlg : public AthAlgorithm {
public:
  MuSAVtxJPsiValidationAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

  const xAOD::Muon* findMuonFromTrack(const xAOD::TrackParticle* tp, const xAOD::MuonContainer* muons);

protected:
  SG::ReadHandleKey<xAOD::MuonContainer> m_muonContainer{ this, "MuonContainer", "Muons", "input muon collection" };
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{ this, "EventInfo", "EventInfo", "event info" };

  SG::WriteHandleKey<xAOD::MuonContainer> m_JPsiMuonContainer{ this, "JPsiMuonContainer", "JPsiMuons", "output J/Psi muon collection" };

  ToolHandle<Analysis::JpsiFinder> m_JPsiFinderTool{ this, "JpsiFinderTool", "Analysis::JpsiFinder/JpsiFinder", "find J/Psi -> mumu" };
};
}
#endif // MuSAVtxJPsiValidationAlg_H
