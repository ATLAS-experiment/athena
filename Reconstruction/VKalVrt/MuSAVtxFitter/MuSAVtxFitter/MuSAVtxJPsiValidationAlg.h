#ifndef MuSAVtxJPsiValidationAlg_H
#define MuSAVtxJPsiValidationAlg_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "JpsiUpsilonTools/JpsiFinder.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TriggerMatchingTool/IMatchingTool.h"


namespace Analysis { class JpsiFinder; }

namespace Rec {
class MuSAVtxJPsiValidationAlg : public AthAlgorithm {
public:
  MuSAVtxJPsiValidationAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

  const xAOD::Muon* findMuonFromTrack(const xAOD::TrackParticle* tp, const xAOD::MuonContainer* muons);

protected:
  SG::ReadHandleKey<xAOD::MuonContainer> m_muonContainer{ this, "MuonContainer", "Muons", "input muon collection" };
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{ this, "EventInfo", "EventInfo", "event info" };

  SG::WriteHandleKey<xAOD::MuonContainer> m_JPsiMuonContainer{ this, "JPsiMuonContainer", "JPsiMuons", "output J/Psi muon collection" };
  SG::WriteHandleKey<xAOD::VertexContainer> m_JPsiVertexContainer{ this, "JPsiVertexContainer", "JPsiVertices", "output J/Psi vertex collection" };
  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_JPsiTrackParticleContainer{ this, "JPsiTrackParticleContainer", "JPsiVerticesTracks", "output J/Psi track collection" };

  ToolHandle<Analysis::JpsiFinder> m_JPsiFinderTool{ this, "JpsiFinderTool", "Analysis::JpsiFinder/JpsiFinder", "find J/Psi -> mumu" };
  PublicToolHandle<Trig::TrigDecisionTool> m_tdt{this, "TrigDecisionTool", ""};
  ToolHandle<Trig::IMatchingTool> m_matchTool{this, "TrigMatchingTool", ""};
  Gaudi::Property<std::vector<std::string>> m_triggerChains{this, "Chain", {"HLT_2mu10_bJpsimumu_L12MU8F", "HLT_2mu4_bJpsimumu_L12MU3VF"}};
  Gaudi::Property<double> m_dR{this, "TriggerMatchDR", 0.01};

  SG::WriteDecorHandleKeyArray<xAOD::VertexContainer> m_trigDecorKeys{this, "TriggerDecorKeys", {}};

};
}
#endif // MuSAVtxJPsiValidationAlg_H
