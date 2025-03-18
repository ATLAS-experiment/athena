/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthParentDecoratorAlg.h"

#include "StoreGate/WriteDecorHandle.h"

#include "TruthUtils/HepMCHelpers.h"

#include <format>

// structure to hold info on a matched parent particle
struct MatchedParent
{
  const xAOD::TruthParticle* parent;
  const xAOD::TruthParticle* child;
  float deltaR;
  unsigned int parent_index;
  std::set<int> cascade_pids;
};

namespace {

  using Barcodex = TruthParentDecoratorAlg::Barcodex;
  using IPMap = TruthParentDecoratorAlg::IPMap;
  using JC = TruthParentDecoratorAlg::JC;
  using TPC = TruthParentDecoratorAlg::TPC;
  using parent_mask_t = unsigned long long;

  parent_mask_t matchMask(const std::vector<MatchedParent>& matches) {
    parent_mask_t mask = 0x0;
    for (const auto& match: matches) {
      constexpr size_t max_idx = std::numeric_limits<decltype(mask)>::digits;
      if (match.parent_index > max_idx) {
        throw std::runtime_error(
          "parent index overflowed the match mask "
          "[index: "  + std::to_string(match.parent_index) +
          " , max_mask: " + std::to_string(max_idx) + "]");
      }
      mask |= (0x1u << match.parent_index);
    }
    return mask;
  }

  // debugging functions
  std::string join(const std::vector<std::string>& v, const std::string& sep = ", ") {
    std::string out;
    for (unsigned int pos = 0; pos < v.size(); pos++) {
      out.append(v.at(pos));
      if (pos + 1 < v.size()) out.append(sep);
    }
    return out;
  }
  template <typename T>
  std::vector<std::string> stringify(const T& container) {
    std::vector<std::string> out;
    for (const auto& v: container) out.push_back(std::to_string(v));
    return out;
  }
  // debugging function, not used now
  [[maybe_unused]]
  std::string listAllChildren(const xAOD::TruthParticle* p) {
    std::vector<std::string> output;
    for (unsigned int child_n = 0; child_n < p->nChildren(); child_n++) {
      output.push_back(std::to_string(p->child(child_n)->pdgId()));
    }
    if (output.empty()) return "none";
    return "[" + join(output) + "]";
  }


  // debugging functions using MsgStream
  void logInputs(MsgStream& msg,SG::ReadHandle<JC>& targets,SG::ReadHandle<TPC>& truth,std::vector<SG::ReadHandle<TPC>>& cascades_raw,const MSG::Level level = MSG::DEBUG)
  {
    if (msg.level() > level) return;
    unsigned int n_cascade_candidates = 0;
    for (auto& cascade: cascades_raw) {
      n_cascade_candidates += cascade->size();
    }
    msg << level <<
      "n_targets: " << targets->size() << ", "
      "n_parents: " << truth->size() << ", "
      "n_cascade_candidates: " << n_cascade_candidates <<
      endmsg;
  }
  void logIPMap(MsgStream& msg,const IPMap& ipmap,const MSG::Level level = MSG::VERBOSE)
  {
    if (msg.level() > level) return;
    for (const auto& [barcode, children]: ipmap) {
      std::set<int> ids;
      for (const xAOD::TruthParticle* dup: children) ids.insert(dup->pdgId());
      msg << level << "barcode: " << barcode << ", n_dup: " << children.size()
          << " idg_ids: [" << join(stringify(ids)) << "]" << endmsg;
    }
  }


  // functions to traverse barcodex and disambiguate the selected
  // child
  std::map<int, std::set<int>> findAllDescendants(int parent, const Barcodex& barcodex, std::set<int> history = {})
  {
    using return_t = std::map<int,std::set<int>>;
    auto itr = barcodex.find(parent);
    if (itr == barcodex.end()) {
      auto hist = stringify(history);
      throw std::runtime_error(
        "can't find barcode " + std::to_string(parent) + " history:"
        " {" + join(hist) + "}");
    }
    const std::set<int>& children = itr->second;
    if (children.empty()) return return_t{{parent, history}};
    if (!history.insert(parent).second) {
      auto hist = stringify(history);
      throw std::runtime_error("found cycle, tried to add " + std::to_string(parent) + " to {" + join(hist) + "}");
    }
    return_t all_children;
    for (int child: children) {
      // We don't just merge the children here in case there are
      // multiple histories that lead to the same discendent. Instead
      // we merge the histories of all descendents.
      for (auto& [dec, dh]: findAllDescendants(child, barcodex, history)) {
        all_children[dec].merge(dh);
      }
    }
    return all_children;
  }


  const xAOD::TruthParticle* selectChild(const IPMap::mapped_type& barkids)
  {
    const xAOD::TruthParticle* child = *barkids.begin();
    if (barkids.size() > 1) {
      std::set<int> pdg_ids;
      TLorentzVector sum_p4;
      // look at duplicates, take the one with the most children
      for (const xAOD::TruthParticle* dupkid: barkids) {
        sum_p4 += dupkid->p4();
        pdg_ids.insert(dupkid->pdgId());
        if (dupkid->nChildren() > child->nChildren()) child = dupkid;
      }
      if (pdg_ids.size() != 1) {
        throw std::runtime_error("same barcode, different pdgid: [" + join(stringify(pdg_ids)) + "]");
      }
      if (float dr = child->p4().DeltaR(sum_p4); dr > 0.001) {
        throw std::runtime_error( "Same barcode, different vector: { deltaR: " + std::to_string(dr) + ", pdgid: " + std::to_string(child->pdgId()) + "}"
          );
      }
    }
    return child;
  }


  bool isOriginal(const xAOD::TruthParticle* p) {
    for (unsigned int parent_n = 0; parent_n < p->nParents(); parent_n++) {
      const xAOD::TruthParticle* parent = p->parent(parent_n);
      if (!parent) throw std::runtime_error("broken truth record");
      if (parent->pdgId() == p->pdgId()) return false;
    }
    return true;
  }


  // these functions are for dealing with specific vertices

  const xAOD::TruthParticle* getParent(const xAOD::TruthParticle* p) {
    if (int n_parents = p->nParents(); n_parents != 1) {
      throw std::logic_error("can't get parent [n_parents: " + std::to_string(n_parents) + "]");
    }
    return p->parent(0);
  }
  bool isSoftLepton(const xAOD::TruthParticle* p) {
    const xAOD::TruthParticle* parent = getParent(p);
    return (parent->hasCharm() || parent->hasBottom()) && p->isChLepton();
  }
  bool isSoftCharm(const xAOD::TruthParticle* p) {
    const xAOD::TruthParticle* parent = getParent(p);
    return parent->hasBottom() && p->hasCharm();
  }

}


// cascade count decorator
CascadeCountDecorator::CascadeCountDecorator( const std::string& name, const std::vector<int>& pids):
  m_pids(pids.begin(), pids.end()),
  m_dec(name),
  // also hang on to the auxid so we can lock it later
  m_auxid(SG::AuxTypeRegistry::instance().findAuxID(name))
{
}
void CascadeCountDecorator::decorate(const SG::AuxElement& target,const std::vector<MatchedParent>& parents) const
{
  unsigned char n_match = 0;
  for (const auto& parent: parents) {
    for (const auto& pid: m_pids) {
      if (parent.cascade_pids.contains(pid)) n_match++;
    }
  }
  m_dec(target) = n_match;
}
void CascadeCountDecorator::decorateDefault(const SG::AuxElement& target) const
{
  m_dec(target) = 0;
}
void CascadeCountDecorator::lock(const xAOD::IParticleContainer* target) const
{
  // We're locking a decoration on a const container, which would be a
  // problem if they were made by anyone else since someone else might
  // be messing with them. But here it shouldn't be a problem since we
  // made them.
  auto* cont ATLAS_THREAD_SAFE = const_cast<xAOD::IParticleContainer*>(target);
  cont->lockDecoration(m_auxid);
}


// main algorithm
TruthParentDecoratorAlg::TruthParentDecoratorAlg(const std::string& name, ISvcLocator* loc):
  AthReentrantAlgorithm(name, loc)
{
  // these aren't user configurable
  declare(m_target_pdgid_key);
  declare(m_target_dr_truth_key);
  declare(m_target_link_key);
  declare(m_target_index_key);
  declare(m_target_n_matched_key);
  declare(m_target_match_mask_key);
  declare(m_match_pdgid_key);
  declare(m_match_children_key);
  declare(m_match_link_key);
}

StatusCode TruthParentDecoratorAlg::initialize() {
  // initialize inputs
  ATH_CHECK(m_parents_key.initialize());
  ATH_CHECK(m_target_container_key.initialize());
  ATH_CHECK(m_cascades_key.initialize());
  // weird hack because Gaudi can't handle an infinite default
  if (m_match_delta_r.value() <= 0) m_match_delta_r.value() = INFINITY;
  // initialize outputs
  std::string jc = m_target_container_key.key();
  std::string pfx = jc + "." + m_prefix.value();
  m_target_pdgid_key = pfx + "PdgId";
  m_target_dr_truth_key = pfx + "DRTruthParticle";
  m_target_link_key = pfx + "Link";
  m_target_index_key = pfx + "Index";
  m_target_n_matched_key = pfx + "NMatchedChildren";
  m_target_match_mask_key = pfx + "ParentsMask";
  m_match_pdgid_key = pfx + "MatchingParticlePdgId";
  m_match_children_key = pfx + "MatchingParticleNChildren";
  m_match_link_key = pfx + "MatchingParticleLink";
  ATH_CHECK(m_target_pdgid_key.initialize());
  ATH_CHECK(m_target_dr_truth_key.initialize());
  ATH_CHECK(m_target_link_key.initialize());
  ATH_CHECK(m_target_index_key.initialize());
  ATH_CHECK(m_target_n_matched_key.initialize());
  ATH_CHECK(m_target_match_mask_key.initialize());
  ATH_CHECK(m_match_pdgid_key.initialize());
  ATH_CHECK(m_match_children_key.initialize());
  ATH_CHECK(m_match_link_key.initialize());

  for (auto& [key, pids]: m_counts_matching_cascade) {
    m_cascade_count_writer_keys.emplace_back(jc + "." + key);
    m_cascade_count_decorators.emplace_back(key, pids);
  }
  for (auto& key: m_cascade_count_writer_keys) declare(key);
  ATH_CHECK(m_cascade_count_writer_keys.initialize());

  return StatusCode::SUCCESS;
}

StatusCode TruthParentDecoratorAlg::execute(const EventContext& cxt) const
{
  ATH_MSG_DEBUG("Executing");

  //////////////////////////////////
  // part 1: read in the particles
  //////////////////////////////////
  SG::ReadHandle<JC> targets(m_target_container_key, cxt);

  using uc_t = unsigned char;
  SG::WriteDecorHandle<JC,int> pdgid(m_target_pdgid_key, cxt);
  SG::WriteDecorHandle<JC,float> deltaR(m_target_dr_truth_key, cxt);
  SG::WriteDecorHandle<JC,JL> link(m_target_link_key, cxt);
  SG::WriteDecorHandle<JC,char> index(m_target_index_key, cxt);
  SG::WriteDecorHandle<JC,uc_t> nMatched(m_target_n_matched_key, cxt);
  SG::WriteDecorHandle<JC,parent_mask_t> mask(m_target_match_mask_key, cxt);
  SG::WriteDecorHandle<JC,int> matchPdgId(m_match_pdgid_key, cxt);
  SG::WriteDecorHandle<JC,int> matchChildCount(m_match_children_key, cxt);
  SG::WriteDecorHandle<JC,JL> matchLink(m_match_link_key, cxt);

  if (targets->empty()) return StatusCode::SUCCESS;

  // read in and sort the parent collection
  SG::ReadHandle<TPC> phandle(m_parents_key, cxt);
  std::set<int> parentids(m_parent_pdgids.begin(), m_parent_pdgids.end());
  std::vector<const xAOD::TruthParticle*> psort;
  for (const xAOD::TruthParticle* p: *phandle) {
    if (!parentids.contains(p->pdgId())) continue;
    if (!isOriginal(p)) continue;
    psort.push_back(p);
  }
  // for lack of a better idea, store truth parents sorted by mass
  std::sort(psort.begin(), psort.end(),[](const auto* p1, const auto* p2) {return p1->m() > p2->m();});
  // check to make sure we don't overflow the match mask
  constexpr size_t max_idx = std::numeric_limits<parent_mask_t>::digits;
  if (psort.size() > max_idx) {
    ATH_MSG_WARNING(
      "Found too many parent particles to store in parent match mask "
      "truncating the parent collection [max: " << max_idx << ", "
      "n: " << psort.size() << "]");
    psort.resize(max_idx);
  }


  std::vector<SG::ReadHandle<TPC>> cascades_raw;
  for (const auto& key: m_cascades_key) {
    cascades_raw.emplace_back(key, cxt);
  }
  logInputs(msgStream(), targets, phandle, cascades_raw);

  ///////////////////////////////
  // part 2: build the barcodex
  ///////////////////////////////
  Barcodex barcodex;
  IPMap ipmap;
  addTruthContainer(barcodex, ipmap, *phandle);
  for (auto& cascade: cascades_raw) {
    addTruthContainer(barcodex, ipmap, *cascade);
  }
  logIPMap(msg(), ipmap);

  ATH_MSG_DEBUG("merged cascade contains " << barcodex.size() << " particles");

  /////////////////////////////////////////////
  // Part 3: build map from targets to parents
  /////////////////////////////////////////////
  std::unordered_map<const J*, std::vector<MatchedParent>> labeled_targets;
  unsigned int n_parents = 0;
  for (const auto* p: psort) {
    unsigned int parent_index = n_parents++;
    ATH_MSG_VERBOSE("pdgid: " << p->pdgId() << ", barcode: " << HepMC::barcode(p));
    for (auto& [cbar, histbars]: findAllDescendants(HepMC::barcode(p), barcodex)) {
      IPMap::mapped_type& barkids = ipmap.at(cbar);
      const xAOD::TruthParticle* child = selectChild(barkids);
      std::vector<std::pair<float, const J*>> drs;
      float drsMinDR=9999;
      const J* drsMinMatch = 0;
      for (const auto* j: *targets) {
        if(j->p4().DeltaR(child->p4()) < drsMinDR) {
          drsMinDR=j->p4().DeltaR(child->p4());
          drsMinMatch=j;
        }
      }
      if(drsMinMatch){
        MatchedParent match;
        match.parent = p;
        match.child = child;
        match.deltaR = drsMinDR;
        match.parent_index = parent_index;
        match.cascade_pids.insert(child->pdgId());
        for (auto& histbar: histbars) {
          match.cascade_pids.insert(selectChild(ipmap.at(histbar))->pdgId());
        }
        if (match.deltaR < m_match_delta_r) {
          labeled_targets[drsMinMatch].push_back(match);
        }
      }
    }
  }

  ///////////////////////
  // Part 4: decorate!
  ///////////////////////

  for (const J* j: *targets) {
    if (labeled_targets.contains(j)) {
      const std::vector<MatchedParent>& matches = labeled_targets.at(j);
      auto min_dr = [](auto& p1, auto& p2) {
        return p1.deltaR < p2.deltaR;
      };
      const MatchedParent& nearest = *std::min_element(
        matches.begin(), matches.end(), min_dr);
      const xAOD::TruthParticle* p = nearest.parent;
      pdgid(*j) = p->pdgId();
      deltaR(*j) = nearest.deltaR;
      auto* container = dynamic_cast<const TPC*>(p->container());
      link(*j) = JL(*container, p->index());
      index(*j) = nearest.parent_index;
      nMatched(*j) = matches.size();
      mask(*j) = matchMask(matches);
      const xAOD::TruthParticle* child = nearest.child;
      matchPdgId(*j) = child->pdgId();
      matchChildCount(*j) = child->nChildren();
      auto* matchedContainer = dynamic_cast<const TPC*>(child->container());
      matchLink(*j) = JL(*matchedContainer, child->index());
      for (const auto& cascadeCount: m_cascade_count_decorators) {
        cascadeCount.decorate(*j, matches);
      }
    } else {
      pdgid(*j) = 0;
      deltaR(*j) = NAN;
      link(*j) = JL();
      index(*j) = -1;
      nMatched(*j) = -1;
      mask(*j) = 0x0;
      matchPdgId(*j) = 0;
      matchChildCount(*j) = 0;
      matchLink(*j) = JL();
      for (const auto& cascadeCount: m_cascade_count_decorators) {
        cascadeCount.decorateDefault(*j);
      }
    }
  }

  ///////////////////////
  // Part 5: lock decorations
  ///////////////////////

  for (const auto& dec: m_cascade_count_decorators) {
    dec.lock(targets.get());
  }

  return StatusCode::SUCCESS;
}

StatusCode TruthParentDecoratorAlg::finalize() {
  if (!m_allow_missing_children_pdgids.empty()) {
    float missing_fraction = double(m_missing_n_ignored) / m_total_children;
    auto msg = std::format("ignored {} missing children out of {} ({:.2}%)",m_missing_n_ignored.load(),m_total_children.load(),missing_fraction * 100);
    if (missing_fraction > m_missing_children_fraction_warning_threshold) {
      ATH_MSG_WARNING(msg);
    } else {
      ATH_MSG_INFO(msg);
    }
  }
  if (!m_warn_missing_children_pdgids.empty()) {
    auto msg = std::format("warned of {} missing children out of {}", m_missing_n_warned.load(), m_total_children.load());
    if (m_missing_n_warned > 0) {
      ATH_MSG_WARNING(msg);
    } else {
      ATH_MSG_INFO(msg);
    }
  }
  return StatusCode::SUCCESS;
}

void TruthParentDecoratorAlg::addTruthContainer(Barcodex& barcodex,IPMap& ipmap,const xAOD::TruthParticleContainer& container) const {

  std::set<int> targid(m_cascade_pdgids.begin(), m_cascade_pdgids.end());
  // we allow decays through any of the parent pdgids
  targid.insert(m_parent_pdgids.begin(), m_parent_pdgids.end());

  // this determines if a cascade vertex should be saved or not
  auto cascadeWants = [
    &targid,
    b=m_add_b,
    c=m_add_c,
    vsl=m_veto_soft_lepton,
    vsc=m_veto_soft_charm
    ] (const xAOD::TruthParticle* p) {
    if (int n_parents = p->nParents(); n_parents == 1) {
      if (vsl && isSoftLepton(p)) return false;
      if (vsc && isSoftCharm(p)) return false;
    }
    if (targid.contains(p->pdgId())) return true;
    if (b && p->hasBottom()) return true;
    if (c && p->hasCharm()) return true;
    return false;
  };

  // insert a particle into the record, return the child set
  auto insert = [&barcodex, &ipmap](const xAOD::TruthParticle* p) -> auto& {
    ipmap[HepMC::barcode(p)].insert(p);
    return barcodex[HepMC::barcode(p)];
  };

  for (const xAOD::TruthParticle* p: container) {
    if (cascadeWants(p)) {
      auto& child_set = insert(p);
      for (unsigned int child_n = 0; child_n < p->nChildren(); child_n++) {
        const xAOD::TruthParticle* c = p->child(child_n);

        // Unfortunately the truth record is often broken in various
        // ways. There are a few configurable ways to deal with
        // this. We keep track of the total number and how often we
        // use these workarounds to make sure things don't go too
        // crazy.
        m_total_children++;
        const auto& ok_missing = m_allow_missing_children_pdgids.value();
        if (!c) {
          if(ok_missing.contains(p->pdgId())) {
            m_missing_n_ignored++;
          } else {
            auto problem = std::format(
              "null truth child [barcode={},pdg_id={},child={}of{}]",
              HepMC::barcode(p), p->pdgId(), child_n, p->nChildren());
            const auto& warn_missing = m_warn_missing_children_pdgids.value();
            if (warn_missing.contains(p->pdgId())) {
              m_missing_n_warned++;
              ATH_MSG_WARNING(problem);
            } else {
              throw std::runtime_error(problem);
            }
          }
        } else if (cascadeWants(c)) {
          insert(c);
          child_set.insert(HepMC::barcode(c));
        }
      };
    }
  }
}
