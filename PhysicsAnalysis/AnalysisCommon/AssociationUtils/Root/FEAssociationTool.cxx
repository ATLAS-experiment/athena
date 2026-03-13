/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AssociationUtils/FEAssociationTool.h"

#include "AsgMessaging/MessageCheck.h"
#include "AthContainers/AuxElement.h"

#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"
#ifndef XAOD_STANDALONE
#include "StoreGate/ReadDecorHandle.h"
#endif

// EDM
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/Jet.h"
#include "xAODMissingET/MissingETAssociationMap.h"

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>

using namespace asg::msgUserCode;

namespace
{
  constexpr float INV_GEV = 1.f / 1000.f;

  static const SG::AuxElement::Accessor<ElementLink<xAOD::IParticleContainer>> ACC_partA("FE_ParticleA");
  static const SG::AuxElement::Accessor<ElementLink<xAOD::IParticleContainer>> ACC_partB("FE_ParticleB");
  static const SG::AuxElement::Accessor<int>   ACC_typeA("FE_ParticleA_Type");
  static const SG::AuxElement::Accessor<int>   ACC_typeB("FE_ParticleB_Type");
  static const SG::AuxElement::Accessor<float> ACC_sharedEc("FE_SharedEc");
  static const SG::AuxElement::Accessor<float> ACC_sharedEn("FE_SharedEn");
  static const SG::AuxElement::Accessor<float> ACC_fracAc("FE_FracAc");
  static const SG::AuxElement::Accessor<float> ACC_fracAn("FE_FracAn");
  static const SG::AuxElement::Accessor<float> ACC_fracBc("FE_FracBc");
  static const SG::AuxElement::Accessor<float> ACC_fracBn("FE_FracBn");

#ifndef XAOD_STANDALONE
  using FELinks_t = std::vector<ElementLink<xAOD::FlowElementContainer>>;
  using OrigObjLink_t = ElementLink<xAOD::IParticleContainer>;
#else
  static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::FlowElementContainer>>>
    ACC_cFEs("chargedGlobalFELinks");
  static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::FlowElementContainer>>>
    ACC_nFEs("neutralGlobalFELinks");
  static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::IParticleContainer>>
    ACC_origObj("originalObjectLink");
#endif

  inline void warnLimited(std::atomic<unsigned>& ctr, const std::function<void()>& fn)
  {
    if (ctr.fetch_add(1, std::memory_order_relaxed) < 10) fn();
  }
}

namespace ORUtils
{

FEAssociationTool::PairKey
FEAssociationTool::PairKey::make(const xAOD::IParticleContainer* a, std::size_t ia,
                                 const xAOD::IParticleContainer* b, std::size_t ib)
{
  const bool aFirst =
    std::less<const xAOD::IParticleContainer*>{}(a, b) || (a == b && ia <= ib);

  return aFirst ? PairKey{a, ia, b, ib} : PairKey{b, ib, a, ia};
}

bool FEAssociationTool::PairKey::operator==(const PairKey& o) const noexcept
{
  return c1 == o.c1 && i1 == o.i1 && c2 == o.c2 && i2 == o.i2;
}

std::size_t FEAssociationTool::PairKeyHash::operator()(const PairKey& k) const noexcept
{
  std::size_t h1 = std::hash<const void*>{}(k.c1) ^ (std::hash<std::size_t>{}(k.i1) << 1);
  std::size_t h2 = std::hash<const void*>{}(k.c2) ^ (std::hash<std::size_t>{}(k.i2) << 1);
  return h1 ^ (h2 << 1);
}

FEAssociationTool::FEAssociationTool(const std::string& name)
  : asg::AsgTool(name)
{}

StatusCode FEAssociationTool::initialize()
{
  ANA_CHECK_SET_TYPE(StatusCode);

#ifndef XAOD_STANDALONE
  ATH_CHECK(m_elKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_muKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_phKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_tauKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_srjKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_lrjKey.initialize(SG::AllowEmpty));

  ATH_CHECK(m_elChargedFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_elNeutralFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_muChargedFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_muNeutralFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_phChargedFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_phNeutralFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_tauChargedFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_tauNeutralFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_srjChargedFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_srjNeutralFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_lrjChargedFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_lrjNeutralFELinksKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_originalObjectLinkKey.initialize(SG::AllowEmpty));
#else
  ATH_CHECK(m_elKey.initialize());
  ATH_CHECK(m_muKey.initialize());
  ATH_CHECK(m_phKey.initialize());
  ATH_CHECK(m_tauKey.initialize());
  ATH_CHECK(m_srjKey.initialize());
  ATH_CHECK(m_lrjKey.initialize());
#endif

  ATH_CHECK(m_outputMapKey.initialize());

  ANA_MSG_INFO("Initializing FEAssociationTool; OutputMap=" << m_outputMapKey.key());
  return StatusCode::SUCCESS;
}

#ifndef XAOD_STANDALONE
StatusCode FEAssociationTool::buildAssociations(const EventContext& ctx) const
#else
StatusCode FEAssociationTool::buildAssociations()
#endif
{
  ANA_CHECK_SET_TYPE(StatusCode);

  std::vector<ObjView> objects;
#ifndef XAOD_STANDALONE
  ANA_CHECK(collectObjects(ctx, objects));
#else
  ANA_CHECK(collectObjects(objects));
#endif

  ANA_MSG_DEBUG("Collected " << objects.size() << " objects");

#ifndef XAOD_STANDALONE
  ANA_CHECK(buildMapFromPairs(ctx, objects));
#else
  ANA_CHECK(buildMapFromPairs(objects));
#endif

  return StatusCode::SUCCESS;
}

#ifndef XAOD_STANDALONE
StatusCode FEAssociationTool::collectObjects(const EventContext& ctx,
                                             std::vector<ObjView>& objects) const
#else
StatusCode FEAssociationTool::collectObjects(std::vector<ObjView>& objects) const
#endif
{
  ANA_CHECK_SET_TYPE(StatusCode);

  auto collect_one = [&](ObjView::Type type, const xAOD::IParticleContainer* base)
  {
    if (!base) return;
    for (std::size_t idx = 0; idx < base->size(); ++idx)
    {
      ObjView view(type, base, idx);
#ifndef XAOD_STANDALONE
      collectFEsFromIndex(ctx, type, base, idx, view);
#else
      collectFEsFromIndex(type, base, idx, view);
#endif
      objects.emplace_back(std::move(view));
    }
  };

#ifndef XAOD_STANDALONE
  {
    SG::ReadHandle<xAOD::ElectronContainer> h(m_elKey, ctx);
    if (h.isValid()) {
      collect_one(ObjView::Type::Electron,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::MuonContainer> h(m_muKey, ctx);
    if (h.isValid()) {
      collect_one(ObjView::Type::Muon,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::PhotonContainer> h(m_phKey, ctx);
    if (h.isValid()) {
      collect_one(ObjView::Type::Photon,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::TauJetContainer> h(m_tauKey, ctx);
    if (h.isValid()) {
      collect_one(ObjView::Type::Tau,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::JetContainer> h(m_srjKey, ctx);
    if (h.isValid()) {
      collect_one(ObjView::Type::SmallRJet,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::JetContainer> h(m_lrjKey, ctx);
    if (h.isValid()) {
      collect_one(ObjView::Type::LargeRJet,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
#else
  {
    SG::ReadHandle<xAOD::ElectronContainer> h(m_elKey);
    if (h.isValid()) {
      collect_one(ObjView::Type::Electron,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::MuonContainer> h(m_muKey);
    if (h.isValid()) {
      collect_one(ObjView::Type::Muon,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::PhotonContainer> h(m_phKey);
    if (h.isValid()) {
      collect_one(ObjView::Type::Photon,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::TauJetContainer> h(m_tauKey);
    if (h.isValid()) {
      collect_one(ObjView::Type::Tau,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::JetContainer> h(m_srjKey);
    if (h.isValid()) {
      collect_one(ObjView::Type::SmallRJet,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
  {
    SG::ReadHandle<xAOD::JetContainer> h(m_lrjKey);
    if (h.isValid()) {
      collect_one(ObjView::Type::LargeRJet,
                  static_cast<const xAOD::IParticleContainer*>(h.cptr()));
    }
  }
#endif

  return StatusCode::SUCCESS;
}

#ifndef XAOD_STANDALONE
void FEAssociationTool::collectFEsFromIndex(const EventContext& ctx,
                                            const ObjView::Type type,
                                            const xAOD::IParticleContainer* cont,
                                            std::size_t idx,
                                            ObjView& view) const
#else
void FEAssociationTool::collectFEsFromIndex(const ObjView::Type /*type*/,
                                            const xAOD::IParticleContainer* cont,
                                            std::size_t idx,
                                            ObjView& view) const
#endif
{
  view.cSet.clear();
  view.nSet.clear();
  view.EcTot = 0.f;
  view.EnTot = 0.f;

  static std::atomic<unsigned> s_warn{0};

  if (!cont)
  {
    warnLimited(s_warn, [&]{ ANA_MSG_WARNING("collectFEsFromIndex called with null container"); });
    return;
  }
  if (idx >= cont->size())
  {
    warnLimited(s_warn, [&]{
      ANA_MSG_WARNING("collectFEsFromIndex index out of range: idx=" << idx
                        << " size=" << cont->size());
    });
    return;
  }

  const xAOD::IParticle* p = (*cont)[idx];
  if (!p)
  {
    warnLimited(s_warn, [&]{ ANA_MSG_WARNING("Null IParticle at idx=" << idx); });
    return;
  }

  auto loop_links =
    [&](const std::vector<ElementLink<xAOD::FlowElementContainer>>& links, bool charged)
  {
    for (const auto& el : links)
    {
      if (!el.isValid()) continue;

      const xAOD::FlowElement* fe = *el;
      if (!fe) continue;

      if (charged) {
        view.cSet.insert(fe);
        view.EcTot += fe->e() * INV_GEV;
      } else {
        view.nSet.insert(fe);
        view.EnTot += fe->e() * INV_GEV;
      }
    }
  };

  bool usedGlobal = false;

#ifndef XAOD_STANDALONE
  switch (type)
  {
  case ObjView::Type::Electron:
  {
    SG::ReadDecorHandle<xAOD::ElectronContainer, FELinks_t> chargedFEs(m_elChargedFELinksKey, ctx);
    SG::ReadDecorHandle<xAOD::ElectronContainer, FELinks_t> neutralFEs(m_elNeutralFELinksKey, ctx);
    if (chargedFEs.isAvailable()) { usedGlobal = true; loop_links(chargedFEs(*p), true); }
    if (neutralFEs.isAvailable()) { usedGlobal = true; loop_links(neutralFEs(*p), false); }
    break;
  }
  case ObjView::Type::Muon:
  {
    SG::ReadDecorHandle<xAOD::MuonContainer, FELinks_t> chargedFEs(m_muChargedFELinksKey, ctx);
    SG::ReadDecorHandle<xAOD::MuonContainer, FELinks_t> neutralFEs(m_muNeutralFELinksKey, ctx);
    if (chargedFEs.isAvailable()) { usedGlobal = true; loop_links(chargedFEs(*p), true); }
    if (neutralFEs.isAvailable()) { usedGlobal = true; loop_links(neutralFEs(*p), false); }
    break;
  }
  case ObjView::Type::Photon:
  {
    SG::ReadDecorHandle<xAOD::PhotonContainer, FELinks_t> chargedFEs(m_phChargedFELinksKey, ctx);
    SG::ReadDecorHandle<xAOD::PhotonContainer, FELinks_t> neutralFEs(m_phNeutralFELinksKey, ctx);
    if (chargedFEs.isAvailable()) { usedGlobal = true; loop_links(chargedFEs(*p), true); }
    if (neutralFEs.isAvailable()) { usedGlobal = true; loop_links(neutralFEs(*p), false); }
    break;
  }
  case ObjView::Type::Tau:
  {
    SG::ReadDecorHandle<xAOD::TauJetContainer, FELinks_t> chargedFEs(m_tauChargedFELinksKey, ctx);
    SG::ReadDecorHandle<xAOD::TauJetContainer, FELinks_t> neutralFEs(m_tauNeutralFELinksKey, ctx);
    if (chargedFEs.isAvailable()) { usedGlobal = true; loop_links(chargedFEs(*p), true); }
    if (neutralFEs.isAvailable()) { usedGlobal = true; loop_links(neutralFEs(*p), false); }
    break;
  }
  case ObjView::Type::SmallRJet:
  {
    SG::ReadDecorHandle<xAOD::JetContainer, FELinks_t> chargedFEs(m_srjChargedFELinksKey, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, FELinks_t> neutralFEs(m_srjNeutralFELinksKey, ctx);
    if (chargedFEs.isAvailable()) { usedGlobal = true; loop_links(chargedFEs(*p), true); }
    if (neutralFEs.isAvailable()) { usedGlobal = true; loop_links(neutralFEs(*p), false); }
    break;
  }
  case ObjView::Type::LargeRJet:
  {
    SG::ReadDecorHandle<xAOD::JetContainer, FELinks_t> chargedFEs(m_lrjChargedFELinksKey, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, FELinks_t> neutralFEs(m_lrjNeutralFELinksKey, ctx);
    if (chargedFEs.isAvailable()) { usedGlobal = true; loop_links(chargedFEs(*p), true); }
    if (neutralFEs.isAvailable()) { usedGlobal = true; loop_links(neutralFEs(*p), false); }
    break;
  }
  }
#else
  if (ACC_cFEs.isAvailable(*p)) {
    usedGlobal = true;
    loop_links(ACC_cFEs(*p), true);
  }
  if (ACC_nFEs.isAvailable(*p)) {
    usedGlobal = true;
    loop_links(ACC_nFEs(*p), false);
  }
#endif

  if (!usedGlobal && p->type() == xAOD::Type::Jet)
  {
    const auto* jet = static_cast<const xAOD::Jet*>(p);

    for (const auto& cl : jet->constituentLinks())
    {
      if (!cl.isValid()) continue;

      const xAOD::IParticle* cpart = *cl;
      if (!cpart) continue;
      if (cpart->type() != xAOD::Type::FlowElement) continue;

      const auto* constit = static_cast<const xAOD::FlowElement*>(cpart);

#ifndef XAOD_STANDALONE
      SG::ReadDecorHandle<xAOD::FlowElementContainer, OrigObjLink_t> origObj(m_originalObjectLinkKey, ctx);

      if (origObj.isAvailable())
      {
        const auto& feLink = origObj(*constit);
#else
      if (ACC_origObj.isAvailable(*constit))
      {
        const auto& feLink = ACC_origObj(*constit);
#endif
        if (!feLink.isValid()) continue;

        const xAOD::IParticle* opart = *feLink;
        if (!opart) continue;
        if (opart->type() != xAOD::Type::FlowElement) continue;

        const auto* fe = static_cast<const xAOD::FlowElement*>(opart);

        if (fe->isCharged()) {
          view.cSet.insert(fe);
          view.EcTot += fe->e() * INV_GEV;
        } else {
          view.nSet.insert(fe);
          view.EnTot += fe->e() * INV_GEV;
        }
      }
      else
      {
        const auto* fe = constit;

        if (fe->isCharged()) {
          view.cSet.insert(fe);
          view.EcTot += fe->e() * INV_GEV;
        } else {
          view.nSet.insert(fe);
          view.EnTot += fe->e() * INV_GEV;
        }
      }
    }
  }
}

#ifndef XAOD_STANDALONE
StatusCode FEAssociationTool::buildMapFromPairs(const EventContext& ctx,
                                                const std::vector<ObjView>& objects) const
#else
StatusCode FEAssociationTool::buildMapFromPairs(const std::vector<ObjView>& objects)
#endif
{
  ANA_CHECK_SET_TYPE(StatusCode);

  std::unordered_map<PairKey, SharedAcc, PairKeyHash> shared;
  shared.reserve(objects.size() * 2);

  for (std::size_t i = 0; i < objects.size(); ++i)
  {
    const auto& A = objects[i];

    for (std::size_t j = i + 1; j < objects.size(); ++j)
    {
      const auto& B = objects[j];

      float Ec = 0.f;
      float En = 0.f;

      if (!A.cSet.empty() && !B.cSet.empty())
      {
        const auto& small = (A.cSet.size() < B.cSet.size()) ? A.cSet : B.cSet;
        const auto& large = (A.cSet.size() < B.cSet.size()) ? B.cSet : A.cSet;
        for (const auto* fe : small) {
          if (large.count(fe)) Ec += fe->e() * INV_GEV;
        }
      }

      if (!A.nSet.empty() && !B.nSet.empty())
      {
        const auto& small = (A.nSet.size() < B.nSet.size()) ? A.nSet : B.nSet;
        const auto& large = (A.nSet.size() < B.nSet.size()) ? B.nSet : A.nSet;
        for (const auto* fe : small) {
          if (large.count(fe)) En += fe->e() * INV_GEV;
        }
      }

      if (Ec <= 0.f && En <= 0.f) continue;

      auto key = PairKey::make(A.cont, A.idx, B.cont, B.idx);
      auto& s = shared[key];

      s.Ec += Ec;
      s.En += En;

      s.AcTot = A.EcTot;
      s.AnTot = A.EnTot;
      s.BcTot = B.EcTot;
      s.BnTot = B.EnTot;

      s.typeA = static_cast<int>(A.type);
      s.typeB = static_cast<int>(B.type);
    }
  }

  auto assocMap = std::make_unique<xAOD::MissingETAssociationMap>();
  auto assocAux = std::make_unique<xAOD::AuxContainerBase>();
  assocMap->setStore(assocAux.get());

  for (const auto& [key, sh] : shared)
  {
    auto* assoc = new xAOD::MissingETAssociation();
    assocMap->push_back(assoc);

    ElementLink<xAOD::IParticleContainer> linkA(*key.c1, key.i1);
    ElementLink<xAOD::IParticleContainer> linkB(*key.c2, key.i2);

    if (!linkA.isValid() || !linkB.isValid())
    {
      assocMap->pop_back();
      delete assoc;
      continue;
    }

    ACC_partA(*assoc) = linkA;
    ACC_partB(*assoc) = linkB;
    ACC_typeA(*assoc) = sh.typeA;
    ACC_typeB(*assoc) = sh.typeB;

    ACC_sharedEc(*assoc) = sh.Ec;
    ACC_sharedEn(*assoc) = sh.En;

    ACC_fracAc(*assoc) = (sh.AcTot > 0.f ? sh.Ec / sh.AcTot : 0.f);
    ACC_fracAn(*assoc) = (sh.AnTot > 0.f ? sh.En / sh.AnTot : 0.f);
    ACC_fracBc(*assoc) = (sh.BcTot > 0.f ? sh.Ec / sh.BcTot : 0.f);
    ACC_fracBn(*assoc) = (sh.BnTot > 0.f ? sh.En / sh.BnTot : 0.f);
  }

#ifndef XAOD_STANDALONE
  SG::WriteHandle<xAOD::MissingETAssociationMap> mapH(m_outputMapKey, ctx);
  ATH_CHECK(mapH.record(std::move(assocMap), std::move(assocAux)));
#else
  ATH_CHECK(evtStore()->record(assocMap.release(), m_outputMapKey.key()));
  ATH_CHECK(evtStore()->record(assocAux.release(), m_outputMapKey.key() + "Aux."));
#endif

  return StatusCode::SUCCESS;
}

} // namespace ORUtils