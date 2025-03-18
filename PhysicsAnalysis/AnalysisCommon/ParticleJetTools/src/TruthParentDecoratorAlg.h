/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTH_PARENT_DECORATOR_ALG
#define TRUTH_PARENT_DECORATOR_ALG

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// no need for forward declaration here, no one reads this header
#include "xAODBase/IParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

struct MatchedParent;

class CascadeCountDecorator
{
public:
  CascadeCountDecorator(const std::string& name,
                        const std::vector<int>& pids);
  void decorate(const SG::AuxElement& target,
                const std::vector<MatchedParent>& parents) const;
  void decorateDefault(const SG::AuxElement& target) const;
  void lock(const xAOD::IParticleContainer* target) const;
private:
  std::vector<int> m_pids;
  SG::AuxElement::Decorator<unsigned char> m_dec;
  SG::auxid_t m_auxid;
};

class TruthParentDecoratorAlg: public AthReentrantAlgorithm
{
public:
  using Barcodex = std::map<int, std::set<int>>;
  using IPMap = std::map<int, std::set<const xAOD::TruthParticle*>>;
  using JC = xAOD::IParticleContainer;
  using J = xAOD::IParticle;
  using TPC = xAOD::TruthParticleContainer;
  using JL = ElementLink<JC>;
  TruthParentDecoratorAlg(const std::string& name, ISvcLocator* loc);
  virtual StatusCode initialize () override;
  virtual StatusCode execute (const EventContext&) const override;
  virtual StatusCode finalize () override;
private:
  using cascade_counter_property_t = std::map<std::string,std::vector<int>>;
  void addTruthContainer(Barcodex&, IPMap&, const TPC&) const;
  SG::ReadHandleKey<JC> m_target_container_key{
    this, "targetContainer", "", "target container to decorate"
  };
  Gaudi::Property<std::string> m_prefix{
    this, "decoratorPrefix", "parentBoson", "prefix for decorations"
  };
  Gaudi::Property<std::vector<int>> m_parent_pdgids{
    this, "parentPdgIds", {}, "PDGIDs of allowed parent particles"
  };
  Gaudi::Property<std::vector<int>> m_cascade_pdgids{
    this, "cascadePdgIds", {}, "PDGIDs of particles in the decay chain"
  };
  Gaudi::Property<bool> m_add_b{
    this, "addBsToCascade", false, "add all bhadrons to cascade"
  };
  Gaudi::Property<bool> m_add_c{
    this, "addCsToCascade", false, "add all chadrons to cascade"
  };
  Gaudi::Property<bool> m_veto_soft_lepton{
    this, "vetoSoftLeptonCascade", false,
    "veto soft lepton decays from cascade"
  };
  Gaudi::Property<bool> m_veto_soft_charm{
    this, "vetoSoftCharmCascade", false,
    "veto soft charm decays from cascade"
  };
  Gaudi::Property<float> m_match_delta_r{
    this, "matchDeltaR", -1,
    "width of delta R cone for matching (zero or less -> infinite)"
  };
  SG::ReadHandleKey<TPC> m_parents_key{
    this, "parents", "", "truth parent container"
  };
  SG::ReadHandleKeyArray<TPC> m_cascades_key{
    this, "cascades", {}, "truth hadron container"
  };
  SG::WriteDecorHandleKey<JC> m_target_pdgid_key;
  SG::WriteDecorHandleKey<JC> m_target_dr_truth_key;
  SG::WriteDecorHandleKey<JC> m_target_link_key;
  SG::WriteDecorHandleKey<JC> m_target_index_key;
  SG::WriteDecorHandleKey<JC> m_target_n_matched_key;
  SG::WriteDecorHandleKey<JC> m_target_match_mask_key;
  SG::WriteDecorHandleKey<JC> m_match_pdgid_key;
  SG::WriteDecorHandleKey<JC> m_match_children_key;
  SG::WriteDecorHandleKey<JC> m_match_link_key;

  Gaudi::Property<cascade_counter_property_t> m_counts_matching_cascade {
    this, "countChildrenInCascadeWithPdgIds", {},
    "Create one counter for each entry, named by key. Counts children "
    "with at least one overlapping value in the cascade."
  };
  SG::WriteDecorHandleKeyArray<JC> m_cascade_count_writer_keys;
  std::vector<CascadeCountDecorator> m_cascade_count_decorators;

  Gaudi::Property<std::unordered_set<int>> m_allow_missing_children_pdgids {
    this, "allowMissingChildrenPdgIds", {},
    "Allow particles matching these PDG IDs to have missing children"
  };
  Gaudi::Property<float> m_missing_children_fraction_warning_threshold {
    this, "missingChildrenFractionWarningThreshold", 0.01,
    "Turn the missing children info into a warning above this"
  };
  Gaudi::Property<std::unordered_set<int>> m_warn_missing_children_pdgids {
    this, "warnMissingChildrenPdgIds", {},
    "Warn if particles matching these PDF IDs have missing children"
  };
  mutable std::atomic<unsigned long long> m_missing_n_ignored;
  mutable std::atomic<unsigned long long> m_missing_n_warned;
  mutable std::atomic<unsigned long long> m_total_children;
};

#endif
