/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#include "PartonHistory/CalcPartonHistory.h"

#include <xAODEventInfo/EventInfo.h>

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "AthContainers/ConstDataVector.h"
#include "VectorHelpers/LorentzHelper.h"

#ifdef XAOD_STANDALONE
#define TDS() evtStore()->tds()
#else
#define TDS() evtStore()
#endif

namespace {
const std::vector<const xAOD::TruthParticle*>* findVector(
    const std::map<std::string, std::vector<const xAOD::TruthParticle*>>& map,
    const std::string& key) {
  auto it = map.find(key);
  return it == map.end() ? nullptr : &it->second;
}

constexpr const char* kBeforeFSR = "_beforeFSR";
constexpr const char* kAfterFSR = "_afterFSR";
}  // namespace

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

CalcPartonHistory::CalcPartonHistory(
    const std::string& name, const std::vector<std::string>& truthCollection)
    : asg::AsgTool(name), m_truthCollections(truthCollection) {}

bool CalcPartonHistory::ExistsInMap(const std::string& key) const {
  // Checks whether a given key exists in the particle map.
  return m_particleMap.find(key) != m_particleMap.end();
}

bool CalcPartonHistory::ExistsInKey(const std::string& key,
                                    const xAOD::TruthParticle* p) const {
  // Checks whether a given particle exists in the vector for the given key.
  if (const auto* v = findVector(m_particleMap, key)) {
    return std::find(v->begin(), v->end(), p) != v->end();
  }
  return false;
}

bool CalcPartonHistory::Retrievep4(const std::string& key,
                                   PtEtaPhiMVector& p4) {
  return Retrievep4(key, p4, 0);
}

bool CalcPartonHistory::Retrievep4(const std::string& key, PtEtaPhiMVector& p4,
                                   std::size_t idx) {
  const auto* v = findVector(m_particleMap, key);
  if (!v || idx >= v->size())
    return false;
  p4 = GetPtEtaPhiMfromTruth(v->at(idx));
  return true;
}

bool CalcPartonHistory::Retrievep4Gamma(PtEtaPhiMVector& p4, int& parentpdgId) {
  // Finds the highest-pT photon among all m_particleMap entries. Photons are
  // stored either under "..._gamma_<beforeFSR|afterFSR>" keys or, when they
  // come from a W/Z/H decay vertex, under "...Decay<N>_<beforeFSR|afterFSR>".
  const xAOD::TruthParticle* bestPhoton = nullptr;
  for (const auto& entry : m_particleMap) {
    for (const auto* p : entry.second) {
      if (p && p->pdgId() == 22 && (!bestPhoton || p->pt() > bestPhoton->pt()))
        bestPhoton = p;
    }
  }
  if (!bestPhoton) {
    parentpdgId = 0;
    return false;
  }
  p4 = GetPtEtaPhiMfromTruth(bestPhoton);
  // Skip identical-photon copies to find the particle that emitted it.
  const xAOD::TruthParticle* parent =
      bestPhoton->nParents() > 0 ? bestPhoton->parent(0) : nullptr;
  while (parent && parent->pdgId() == 22)
    parent = parent->nParents() > 0 ? parent->parent(0) : nullptr;
  parentpdgId = parent ? parent->pdgId() : 0;
  return true;
}

bool CalcPartonHistory::RetrievepdgId(const std::string& key, int& pdgId) {
  return RetrievepdgId(key, pdgId, 0);
}

bool CalcPartonHistory::RetrievepdgId(const std::string& key,
                                      std::vector<int>& pdgIds) {
  const auto* v = findVector(m_particleMap, key);
  if (!v)
    return false;
  pdgIds.reserve(pdgIds.size() + v->size());
  for (const auto* p : *v)
    if (p)
      pdgIds.push_back(p->pdgId());
  return true;
}

bool CalcPartonHistory::RetrievepdgId(const std::string& key, int& pdgId,
                                      std::size_t idx) {
  const auto* v = findVector(m_particleMap, key);
  if (!v || idx >= v->size())
    return false;
  pdgId = v->at(idx)->pdgId();
  return true;
}

bool CalcPartonHistory::RetrieveParticleInfo(
    const std::string& prefix, std::vector<const xAOD::TruthParticle*>& out) {
  const auto* v = findVector(m_particleMap, prefix);
  if (!v)
    return false;
  out.insert(out.end(), v->begin(), v->end());
  return true;
}

bool CalcPartonHistory::RetrieveParticleInfo(
    const std::string& prefix, std::vector<PtEtaPhiMVector>& particles,
    std::vector<int>& pdgIds) {
  const auto* v = findVector(m_particleMap, prefix);
  if (!v)
    return false;
  particles.reserve(particles.size() + v->size());
  pdgIds.reserve(pdgIds.size() + v->size());
  for (const auto* p : *v) {
    particles.push_back(GetPtEtaPhiMfromTruth(p));
    pdgIds.push_back(p->pdgId());
  }
  return true;
}

bool CalcPartonHistory::RetrieveParticleInfo(const std::string& prefix,
                                             PtEtaPhiMVector& particle,
                                             int& pdgId) {
  return RetrieveParticleInfo(prefix, particle, pdgId, 0);
}

bool CalcPartonHistory::RetrieveParticleInfo(const std::string& prefix,
                                             PtEtaPhiMVector& particle,
                                             int& pdgId, std::size_t idx) {
  return Retrievep4(prefix, particle, idx) && RetrievepdgId(prefix, pdgId, idx);
}

bool CalcPartonHistory::RetrieveParticleInfo(const std::string& prefix,
                                             const std::string& alt_prefix,
                                             PtEtaPhiMVector& particle,
                                             int& pdgId) {
  if (Retrievep4(prefix, particle) && RetrievepdgId(prefix, pdgId))
    return true;
  return Retrievep4(alt_prefix, particle) && RetrievepdgId(alt_prefix, pdgId);
}

std::string CalcPartonHistory::GetParticleType(
    const xAOD::TruthParticle* particle) {
  static const std::unordered_map<int, std::string> pdgMap = {
      {1, "_q"},       {2, "_q"},       {3, "_q"},      {-1, "_qbar"},
      {-2, "_qbar"},   {-3, "_qbar"},   {6, "_t"},      {-6, "_tbar"},
      {5, "_b"},       {-5, "_bbar"},   {4, "_c"},      {-4, "_cbar"},
      {25, "_H"},      {24, "_W"},      {-24, "_W"},    {23, "_Z"},
      {22, "_gamma"},  {21, "_g"},      {11, "_l"},     {13, "_l"},
      {15, "_l"},      {-11, "_lbar"},  {-13, "_lbar"}, {-15, "_lbar"},
      {12, "_nu"},     {14, "_nu"},     {16, "_nu"},    {-12, "_nubar"},
      {-14, "_nubar"}, {-16, "_nubar"}, {2212, "_p"},   {1103, "_dd"},
      {2101, "_ud"},   {2103, "_ud"},   {2203, "_uu"},  {3101, "_sd"},
      {3103, "_sd"},   {3201, "_su"},   {3203, "_su"},  {3303, "_ss"},
      {4101, "_cd"},   {4103, "_cd"},   {4201, "_cu"},  {4203, "_cu"},
      {4301, "_cs"},   {4303, "_cs"},   {4403, "_cc"},  {5101, "_bd"},
      {5103, "_bd"},   {5201, "_bu"},   {5203, "_bu"},  {5301, "_bs"},
      {5303, "_bs"},   {5401, "_bc"},   {5403, "_bc"},  {5503, "_bb"}};
  int pdgId = particle->pdgId();
  auto it = pdgMap.find(pdgId);
  return it != pdgMap.end() ? it->second : "_" + std::to_string(pdgId);
}

void CalcPartonHistory::TraceParticle(
    const xAOD::TruthParticle* p, std::vector<const xAOD::TruthParticle*>& path,
    std::vector<std::vector<const xAOD::TruthParticle*>>& allPaths) {
  // Recursively builds decay paths from p down to stable particles.
  // Each completed root-to-leaf path is appended to allPaths and later
  // processed by FillParticleMap to assign m_particleMap keys.
  //
  // FSR handling: a particle that radiates before decaying appears as a chain
  // of identical PDG-ID nodes (e.g. t → t → t → W b). findAfterFSR() follows
  // that chain to the last node before the actual decay vertex, so the path
  // records the pre- and post-FSR instances rather than every intermediate.
  //
  // W linking (DAOD_PHYS only): TruthTop links to Ws in TruthBoson, which
  // carry no decay products. When we encounter such a W (isAfterFSR &&
  // |pdg|==24) we swap it for the corresponding entry in
  // TruthBosonsWithDecayParticles via the
  // "CustomLinkedTruthBosonWithDecayParticles" decoration so that the subsequent
  // child traversal finds the W decay products.
  if (!p || PartonHistoryUtils::isBrokenTop(*p))
    return;

  // If this W node has no identical child (i.e. it is the after-FSR instance)
  // but lacks decay products, swap to the linked decay-product-bearing copy.
  if (PartonHistoryUtils::isAfterFSR(*p) && std::abs(p->pdgId()) == 24) {
    p = getTruthParticleLinkedFromDecoration(
        p, "CustomLinkedTruthBosonWithDecayParticles");
  }

  path.push_back(p);

  // Leaf node: record the completed path.
  if (p->nChildren() == 0) {
    allPaths.push_back(path);
    path.pop_back();
    return;
  }

  const xAOD::TruthParticle* afterFSR = &PartonHistoryUtils::findAfterFSR(*p);
  // Same W-linking fix for the after-FSR node reached by findAfterFSR().
  if (std::abs(afterFSR->pdgId()) == 24) {
    afterFSR = getTruthParticleLinkedFromDecoration(
        afterFSR, "CustomLinkedTruthBosonWithDecayParticles");
  }

  if (afterFSR != p) {
    // There was FSR: continue the path from the post-FSR node.
    TraceParticle(afterFSR, path, allPaths);
  } else {
    // No FSR: branch into each child (e.g. W→lν gives two sub-paths).
    for (std::size_t i = 0; i < afterFSR->nChildren(); ++i) {
      if (const auto* c = afterFSR->child(i))
        TraceParticle(c, path, allPaths);
    }
  }
  path.pop_back();
}

void CalcPartonHistory::AddToParticleMap(const xAOD::TruthParticle* p,
                                         const std::string& key) {
  if (!ExistsInKey(key, p))
    m_particleMap[key].push_back(p);
}

void CalcPartonHistory::handleFSR(const xAOD::TruthParticle* p,
                                  const std::string& newKey, std::string& key) {
  // Last copy of an FSR chain: stored under the key of its first copy.
  if (PartonHistoryUtils::hasParentPdgId(*p)) {
    AddToParticleMap(p, key + kAfterFSR);
    return;
  }
  key += newKey;
  AddToParticleMap(p, key + kBeforeFSR);
  if (!PartonHistoryUtils::hasIdenticalChild(*p))
    AddToParticleMap(p, key + kAfterFSR);
}

bool CalcPartonHistory::handleDecay(const xAOD::TruthParticle* p,
                                    std::string& key, int decayID) {
  // Only daughters of a W/Z/H (not FSR copies of the boson itself).
  if (PartonHistoryUtils::hasParentPdgId(*p) ||
      !(PartonHistoryUtils::hasParentAbsPdgId(*p, 23) ||
        PartonHistoryUtils::hasParentAbsPdgId(*p, 24) ||
        PartonHistoryUtils::hasParentAbsPdgId(*p, 25)))
    return false;

  const std::string decayStr = "Decay" + std::to_string(decayID);
  key += decayStr;
  AddToParticleMap(p, key + kBeforeFSR);
  if (!PartonHistoryUtils::hasIdenticalChild(*p))
    AddToParticleMap(p, key + kAfterFSR);
  return true;
}

void CalcPartonHistory::FillParticleMap(
    std::vector<std::vector<const xAOD::TruthParticle*>>& allPaths) {
  // Converts the raw decay paths produced by TraceParticles into the
  // m_particleMap used by all Retrieve* and Fill* methods.
  //
  // Key construction: each path is walked particle by particle, accumulating
  // a string key of the form
  // "<prefix>_MC_<type1>_<type2>_..._<beforeFSR|afterFSR>". For example, a b
  // quark from a top gives "MySch_MC_t_b_beforeFSR". Decay products of W/Z/H
  // get an additional "Decay<N>" segment to distinguish the two daughters, e.g.
  // "MySch_MC_t_WDecay1_beforeFSR".
  //
  // Handlers (first match wins for each particle in the path):
  //   handleDecay — daughters of W/Z/H: appends "Decay<N>" and records
  //                 beforeFSR/afterFSR
  //   handleFSR   — all other particles: the first copy of an FSR chain
  //                 appends the type suffix and records beforeFSR (and
  //                 afterFSR if it does not radiate); the last copy (same PDG
  //                 as parent) records afterFSR under the current key
  m_particleMap.clear();

  for (const auto& path : allPaths) {
    // m_particleMap keys always include the prefix, built once here.
    std::string key = m_prefix + "_" + "MC";

    for (const auto* p : path) {
      // beforeFSR: this node has an identical child (it will radiate).
      // afterFSR:  this node's parent has the same PDG ID (it was radiated
      // from).
      const bool beforeFSR = PartonHistoryUtils::hasIdenticalChild(*p);
      const bool afterFSR = PartonHistoryUtils::hasParentPdgId(*p);

      // Determine which child index this particle is under its parent; used to
      // label W/Z/H decay daughters as Decay1, Decay2. Falls back to sign of
      // pdgId (negative → 2) if parent navigation is unavailable.
      int decayID = (p->pdgId() < 0) ? 2 : 1;
      if (p->nParents() != 0 && p->parent(0)) {
        const auto* par = p->parent(0);
        for (std::size_t i = 0; i < par->nChildren(); ++i) {
          if (par->child(i) == p) {
            decayID = static_cast<int>(i) + 1;
            break;
          }
        }
      }

      const std::string new_key = GetParticleType(p);

      // Skip pure intermediate FSR nodes that are neither the first nor last
      // in the FSR chain — they carry no additional physics information.
      if (beforeFSR && afterFSR)
        continue;
      if (handleDecay(p, key, decayID))
        continue;
      handleFSR(p, new_key, key);
    }
  }
}

void CalcPartonHistory::TraceParticles(
    const xAOD::TruthParticleContainer* truthParticles) {
  std::vector<std::vector<const xAOD::TruthParticle*>> allPaths;
  allPaths.reserve(truthParticles->size());
  for (const xAOD::TruthParticle* p : *truthParticles) {
    if (PartonHistoryUtils::hasParticleIdenticalParent(*p))
      continue;
    std::vector<const xAOD::TruthParticle*> path;
    path.reserve(16);
    TraceParticle(p, path, allPaths);
  }
  FillParticleMap(allPaths);
}

StatusCode CalcPartonHistory::initialize() {
  m_dec.setPrefix(m_prefix);
  initializeDecorators();
  return StatusCode::SUCCESS;
}

StatusCode CalcPartonHistory::execute() {
  const xAOD::TruthParticleContainer* truthParticles{nullptr};
  ANA_CHECK(linkTruthContainers(truthParticles));

  const xAOD::EventInfo* partonHistory = nullptr;
  ANA_CHECK(evtStore()->retrieve(partonHistory, "EventInfo"));

  ANA_CHECK(runHistorySaver(truthParticles, partonHistory));
  return StatusCode::SUCCESS;
}

StatusCode CalcPartonHistory::buildContainerFromMultipleCollections(
    const std::vector<std::string>& collections,
    const std::string& out_contName) {
  // In DAOD_PHYS there is no single TruthParticles collection, so we merge
  // several dedicated collections (e.g. TruthTop,
  // TruthBosonsWithDecayParticles) into one working container stored in the
  // event store.
  //
  // The merge must avoid double-counting: the same particle can appear in
  // multiple collections (e.g. the b from a top appears in both TruthTop and
  // TruthBottom). We therefore keep only "root" particles — those that are not
  // a descendant of any other candidate in the merged pool. TraceParticles then
  // walks down from these roots, naturally visiting all descendants regardless
  // of which original collection they came from.
  auto out_cont =
      std::make_unique<ConstDataVector<DataVector<xAOD::TruthParticle_v1>>>(
          SG::VIEW_ELEMENTS);
  std::vector<const xAOD::TruthParticle*> p_candidates;
  std::vector<const xAOD::TruthParticle*> p_parents;
  std::unordered_set<int> candidateUids;

  for (const std::string& collection : collections) {
    const xAOD::TruthParticleContainer* cont = nullptr;
    ANA_CHECK(evtStore()->retrieve(cont, collection));
    for (const xAOD::TruthParticle* p : *cont) {
      if (p) {
        p_candidates.push_back(p);
        candidateUids.insert(p->uid());
      }
    }
  }
  // Collect the uids of all candidates that descend from another (non-PDF)
  // candidate, walking the decay tree of each candidate once. All copies of
  // duplicated particles are walked, as their navigation links may differ.
  std::unordered_set<int> descendantUids;
  std::vector<const xAOD::TruthParticle*> stack;
  for (const xAOD::TruthParticle* ancestor : p_candidates) {
    if (PartonHistoryUtils::isQuarkFromPDF(*ancestor))
      continue;
    stack.assign(1, ancestor);
    while (!stack.empty()) {
      const xAOD::TruthParticle* p = stack.back();
      stack.pop_back();
      for (std::size_t i = 0; i < p->nChildren(); ++i) {
        const xAOD::TruthParticle* child = p->child(i);
        if (!child)
          continue;
        if (candidateUids.count(child->uid()))
          descendantUids.insert(child->uid());
        stack.push_back(child);
      }
    }
  }
  // Retain only particles that have no ancestor among the other candidates.
  // Particles present in several collections (same uid) are kept only once,
  // using the copy from the first collection that contains them.
  std::unordered_set<int> rootUids;
  for (const xAOD::TruthParticle* potential_parent : p_candidates) {
    if (!PartonHistoryUtils::isQuarkFromPDF(*potential_parent) &&
        !descendantUids.count(potential_parent->uid()) &&
        rootUids.insert(potential_parent->uid()).second)
      p_parents.push_back(potential_parent);
  }
  out_cont->insert(out_cont->end(), p_parents.begin(), p_parents.end());
  StatusCode save = TDS()->record(std::move(out_cont), out_contName);
  if (!save)
    return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

StatusCode CalcPartonHistory::linkBosonCollections() {
  return decorateCollectionWithLinksToAnotherCollection(
      "TruthBoson", "TruthBosonsWithDecayParticles",
      "CustomLinkedTruthBosonWithDecayParticles");
}

StatusCode CalcPartonHistory::decorateCollectionWithLinksToAnotherCollection(
    const std::string& collectionToDecorate,
    const std::string& collectionToLink, const std::string& nameOfDecoration) {
  const SG::Decorator<const xAOD::TruthParticle*> dec(nameOfDecoration);
  const xAOD::TruthParticleContainer* cont1 = nullptr;
  const xAOD::TruthParticleContainer* cont2 = nullptr;
  ANA_CHECK(evtStore()->retrieve(cont1, collectionToDecorate));
  ANA_CHECK(evtStore()->retrieve(cont2, collectionToLink));
  std::unordered_map<int, const xAOD::TruthParticle*> byUid;
  byUid.reserve(cont2->size());
  for (const auto* q : *cont2)
    byUid.emplace(q->uid(), q);
  for (const auto* p : *cont1) {
    const xAOD::TruthParticle* link = nullptr;
    auto it = byUid.find(p->uid());
    if (it != byUid.end() && it->second->pdgId() == p->pdgId())
      link = it->second;
    dec(*p) = link;
  }
  return StatusCode::SUCCESS;
}

const xAOD::TruthParticle*
CalcPartonHistory::getTruthParticleLinkedFromDecoration(
    const xAOD::TruthParticle* part, const std::string& decorationName) {
  const SG::ConstAccessor<const xAOD::TruthParticle*> acc(decorationName);
  if (!acc.isAvailable(*part))
    return part;
  const xAOD::TruthParticle* link = acc(*part);
  return link ? link : part;
}

StatusCode CalcPartonHistory::linkTruthContainers(
    const xAOD::TruthParticleContainer*& tp) {
  const std::string key = m_prefix + "_TruthParticles";
  if (!evtStore()->contains<xAOD::TruthParticleContainer>(key)) {
    const auto& collections =
        m_configured ? m_config.truthCollections : m_truthCollections;
    ANA_CHECK(buildContainerFromMultipleCollections(collections, key));
    ANA_CHECK(evtStore()->retrieve(tp, key));
    ANA_CHECK(linkBosonCollections());
    return StatusCode::SUCCESS;
  }
  ANA_CHECK(evtStore()->retrieve(tp, key));
  return StatusCode::SUCCESS;
}

void CalcPartonHistory::configure(const PartonSchemeConfig& config) {
  m_config = config;
  m_configured = true;
}

void CalcPartonHistory::initializeDecorators() {
  if (!m_configured)
    return;

  for (const auto& group : m_config.decoratorGroups) {
    switch (group) {
      case DecoratorGroup::Top:
        InitializeTopDecorators();
        break;
      case DecoratorGroup::TopFCNC:
        InitializeTopDecorators(true);
        break;
      case DecoratorGroup::AntiTop:
        InitializeAntiTopDecorators();
        break;
      case DecoratorGroup::AntiTopFCNC:
        InitializeAntiTopDecorators(true);
        break;
      case DecoratorGroup::FourTop:
        Initialize4TopDecorators();
        break;
      case DecoratorGroup::Ttbar:
        InitializeTtbarDecorators();
        break;
      case DecoratorGroup::Bottom:
        InitializeBottomDecorators();
        break;
      case DecoratorGroup::AntiBottom:
        InitializeAntiBottomDecorators();
        break;
      case DecoratorGroup::VectorBottom:
        InitializeVectorBottomDecorators();
        break;
      case DecoratorGroup::VectorAntiBottom:
        InitializeVectorAntiBottomDecorators();
        break;
      case DecoratorGroup::Charm:
        InitializeCharmDecorators();
        break;
      case DecoratorGroup::AntiCharm:
        InitializeAntiCharmDecorators();
        break;
      case DecoratorGroup::VectorCharm:
        InitializeVectorCharmDecorators();
        break;
      case DecoratorGroup::VectorAntiCharm:
        InitializeVectorAntiCharmDecorators();
        break;
      case DecoratorGroup::Photon:
        InitializePhotonDecorators();
        break;
      case DecoratorGroup::Higgs:
        InitializeHiggsDecorators();
        break;
    }
  }

  for (const auto& zw : m_config.decoratorZWs) {
    if (zw.type == DecoratorZW::Z)
      InitializeZDecorators(zw.count, zw.extended);
    else
      InitializeWDecorators(zw.count);
  }

  for (const auto& fill : m_config.genericFills) {
    if (fill.isVector) {
      m_dec.initializeVectorPtEtaPhiMDecorator(fill.decorationKey);
      m_dec.initializeVectorIntDecorator(fill.decorationKey + "_pdgId");
    } else {
      m_dec.initializePtEtaPhiMDecorator(fill.decorationKey);
      m_dec.initializeIntDecorator(fill.decorationKey + "_pdgId");
    }
  }

  for (const auto& fill : m_config.isOnShellFills)
    m_dec.initializeIntDecorator(fill.decorationKey);
}

StatusCode CalcPartonHistory::runHistorySaver(
    const xAOD::TruthParticleContainer* truthParticles,
    const xAOD::EventInfo* partonHistory) {

  // Register the EventInfo for this event; all m_dec.decorate*() calls
  // will write to it automatically.
  m_dec.setEventInfo(partonHistory);

  TraceParticles(truthParticles);

  for (const auto& op : m_config.specialFills) {
    switch (op.type) {
      case SpecialFillType::Top:
        FillTopPartonHistory();
        break;
      case SpecialFillType::TopFCNC:
        FillTopPartonHistory(true);
        break;
      case SpecialFillType::AntiTop:
        FillAntiTopPartonHistory();
        break;
      case SpecialFillType::AntiTopFCNC:
        FillAntiTopPartonHistory(true);
        break;
      case SpecialFillType::Ttbar:
        FillTtbarPartonHistory();
        break;
      case SpecialFillType::TtbarFCNC:
        FillTtbarPartonHistory(true);
        break;
      case SpecialFillType::Z:
        FillZPartonHistory(op.parent, op.count, op.mode);
        break;
      case SpecialFillType::Ztautau:
        FillZtautauPartonHistory(op.parent, op.count, op.mode);
        break;
      case SpecialFillType::W:
        FillWPartonHistory(op.parent, op.count, op.mode);
        break;
      case SpecialFillType::Higgs:
        FillHiggsPartonHistory(op.mode);
        break;
      case SpecialFillType::Gamma:
        FillGammaPartonHistory();
        break;
    }
  }

  for (const auto& fill : m_config.genericFills) {
    if (fill.isVector) {
      FillGenericVectorPartonHistory(fill.retrievalKeys.at(0),
                                     fill.decorationKey);
    } else if (fill.retrievalKeys.size() == 1) {
      FillGenericPartonHistory(fill.retrievalKeys.at(0), fill.decorationKey,
                               fill.idx);
    } else {
      FillGenericPartonHistory(fill.retrievalKeys, fill.decorationKey,
                               fill.idx);
    }
  }

  for (const auto& fill : m_config.isOnShellFills) {
    bool onShell = false;
    for (const auto& key : fill.retrievalKeys) {
      if (ExistsInMap(m_prefix + "_" + key)) {
        onShell = true;
        break;
      }
    }
    m_dec.decorateCustom(fill.decorationKey, onShell ? 1 : 0);
  }

  return StatusCode::SUCCESS;
}

}  // namespace CP
