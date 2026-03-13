/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_FEASSOCIATIONTOOL_H
#define ASSOCIATIONUTILS_FEASSOCIATIONTOOL_H

#include "AsgTools/AsgTool.h"
#include "AssociationUtils/IFEAssociationTool.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"
#ifndef XAOD_STANDALONE
#include "StoreGate/ReadDecorHandleKey.h"
#endif

#include "xAODBase/IParticleContainer.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODCore/AuxContainerBase.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/JetContainer.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class EventContext;

namespace ORUtils
{

class FEAssociationTool : public asg::AsgTool,
                          virtual public IFEAssociationTool
{
  ASG_TOOL_CLASS(FEAssociationTool, ORUtils::IFEAssociationTool)

public:
  FEAssociationTool(const std::string& name);
  virtual ~FEAssociationTool() override = default;

  virtual StatusCode initialize() override;

#ifndef XAOD_STANDALONE
  virtual StatusCode buildAssociations(const EventContext& ctx) const override;
#else
  virtual StatusCode buildAssociations() override;
#endif

private:
  struct ObjView
  {
    enum class Type {
      Electron,
      Muon,
      Photon,
      Tau,
      SmallRJet,
      LargeRJet
    };

    ObjView(Type t, const xAOD::IParticleContainer* c, std::size_t i)
      : type(t), cont(c), idx(i) {}

    Type type;
    const xAOD::IParticleContainer* cont = nullptr;
    std::size_t idx = 0;

    std::unordered_set<const xAOD::FlowElement*> cSet;
    std::unordered_set<const xAOD::FlowElement*> nSet;

    float EcTot = 0.f;
    float EnTot = 0.f;
  };

  struct PairKey
  {
    const xAOD::IParticleContainer* c1 = nullptr;
    std::size_t i1 = 0;
    const xAOD::IParticleContainer* c2 = nullptr;
    std::size_t i2 = 0;

    static PairKey make(const xAOD::IParticleContainer* a, std::size_t ia,
                        const xAOD::IParticleContainer* b, std::size_t ib);

    bool operator==(const PairKey& o) const noexcept;
  };

  struct PairKeyHash
  {
    std::size_t operator()(const PairKey& k) const noexcept;
  };

  struct SharedAcc
  {
    float Ec = 0.f;
    float En = 0.f;

    float AcTot = 0.f;
    float AnTot = 0.f;
    float BcTot = 0.f;
    float BnTot = 0.f;

    int typeA = 0;
    int typeB = 0;
  };

#ifndef XAOD_STANDALONE
  virtual StatusCode collectObjects(const EventContext& ctx,
                                    std::vector<ObjView>& objects) const;
#else
  virtual StatusCode collectObjects(std::vector<ObjView>& objects) const;
#endif

#ifndef XAOD_STANDALONE
  void collectFEsFromIndex(const EventContext& ctx,
                           const ObjView::Type type,
                           const xAOD::IParticleContainer* cont,
                           std::size_t idx,
                           ObjView& view) const;
#else
  void collectFEsFromIndex(const ObjView::Type type,
                           const xAOD::IParticleContainer* cont,
                           std::size_t idx,
                           ObjView& view) const;
#endif

#ifndef XAOD_STANDALONE
  virtual StatusCode buildMapFromPairs(const EventContext& ctx,
                                       const std::vector<ObjView>& objects) const;
#else
  virtual StatusCode buildMapFromPairs(const std::vector<ObjView>& objects);
#endif

  SG::ReadHandleKey<xAOD::ElectronContainer> m_elKey{
    this, "ElectronContainer", "Electrons", "Input electron container"
  };
  SG::ReadHandleKey<xAOD::MuonContainer> m_muKey{
    this, "MuonContainer", "Muons", "Input muon container"
  };
  SG::ReadHandleKey<xAOD::PhotonContainer> m_phKey{
    this, "PhotonContainer", "Photons", "Input photon container"
  };
  SG::ReadHandleKey<xAOD::TauJetContainer> m_tauKey{
    this, "TauContainer", "TauJets", "Input tau container"
  };
  SG::ReadHandleKey<xAOD::JetContainer> m_srjKey{
    this, "SmallRJetContainer", "AntiKt4EMPFlowJets", "Input small-R jet container"
  };
  SG::ReadHandleKey<xAOD::JetContainer> m_lrjKey{
    this, "LargeRJetContainer", "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets", "Input large-R jet container"
  };

#ifndef XAOD_STANDALONE
  SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_elChargedFELinksKey{
    this, "ElectronChargedFELinksDecorKey",
    "Electrons.chargedGlobalFELinks",
    "Electron charged global FE links decoration"
  };
  SG::ReadDecorHandleKey<xAOD::ElectronContainer> m_elNeutralFELinksKey{
    this, "ElectronNeutralFELinksDecorKey",
    "Electrons.neutralGlobalFELinks",
    "Electron neutral global FE links decoration"
  };

  SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muChargedFELinksKey{
    this, "MuonChargedFELinksDecorKey",
    "Muons.chargedGlobalFELinks",
    "Muon charged global FE links decoration"
  };
  SG::ReadDecorHandleKey<xAOD::MuonContainer> m_muNeutralFELinksKey{
    this, "MuonNeutralFELinksDecorKey",
    "Muons.neutralGlobalFELinks",
    "Muon neutral global FE links decoration"
  };

  SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_phChargedFELinksKey{
    this, "PhotonChargedFELinksDecorKey",
    "Photons.chargedGlobalFELinks",
    "Photon charged global FE links decoration"
  };
  SG::ReadDecorHandleKey<xAOD::PhotonContainer> m_phNeutralFELinksKey{
    this, "PhotonNeutralFELinksDecorKey",
    "Photons.neutralGlobalFELinks",
    "Photon neutral global FE links decoration"
  };

  SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_tauChargedFELinksKey{
    this, "TauChargedFELinksDecorKey",
    "TauJets.chargedGlobalFELinks",
    "Tau charged global FE links decoration"
  };
  SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_tauNeutralFELinksKey{
    this, "TauNeutralFELinksDecorKey",
    "TauJets.neutralGlobalFELinks",
    "Tau neutral global FE links decoration"
  };

  SG::ReadDecorHandleKey<xAOD::JetContainer> m_srjChargedFELinksKey{
    this, "SmallRJetChargedFELinksDecorKey",
    "AntiKt4EMPFlowJets.chargedGlobalFELinks",
    "Small-R jet charged global FE links decoration"
  };
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_srjNeutralFELinksKey{
    this, "SmallRJetNeutralFELinksDecorKey",
    "AntiKt4EMPFlowJets.neutralGlobalFELinks",
    "Small-R jet neutral global FE links decoration"
  };

  SG::ReadDecorHandleKey<xAOD::JetContainer> m_lrjChargedFELinksKey{
    this, "LargeRJetChargedFELinksDecorKey",
    "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.chargedGlobalFELinks",
    "Large-R jet charged global FE links decoration"
  };
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_lrjNeutralFELinksKey{
    this, "LargeRJetNeutralFELinksDecorKey",
    "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.neutralGlobalFELinks",
    "Large-R jet neutral global FE links decoration"
  };

  SG::ReadDecorHandleKey<xAOD::FlowElementContainer> m_originalObjectLinkKey{
    this, "OriginalObjectLinkDecorKey",
    "JetETMissChargedParticleFlowObjects.originalObjectLink",
    "FlowElement originalObjectLink decoration"
  };
#endif

  SG::WriteHandleKey<xAOD::MissingETAssociationMap> m_outputMapKey{
    this, "OutputMap", "FEAssociationMap", "Output MissingETAssociationMap"
  };
};

} // namespace ORUtils

#endif