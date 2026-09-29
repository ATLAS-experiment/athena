/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#include "PartonHistory/PartonSchemeConfig.h"

#include <map>
#include <stdexcept>

namespace CP {

const PartonSchemeConfig& getSchemeConfig(const std::string& schemeName) {

  static const std::map<std::string, PartonSchemeConfig> registry = {

      // ================================================================== //
      // HOW TO ADD A NEW PARTON HISTORY SCHEME
      // ================================================================== //
      //
      // Copy the template below, uncomment it, fill in the fields, and add
      // a corresponding entry to TRUTH_BRANCHES in python/PartonHistoryConfig.py.
      //
      // {"MySchemeName", {
      //
      //   // 1. truthCollections — which xAOD truth containers to merge into
      //   //    the working TruthParticleContainer for this event.
      //   //    Standard containers available in DAOD_PHYS:
      //   //      "TruthTop"                       top quarks
      //   //      "TruthBottom"                    b quarks
      //   //      "TruthCharm"                     c quarks
      //   //      "TruthBosonsWithDecayParticles"  W/Z/H with decay products
      //   //      "TruthElectrons"                 electrons
      //   //      "TruthMuons"                     muons
      //   //      "TruthTaus"                      taus
      //   //      "TruthNeutrinos"                 neutrinos
      //   //      "TruthPhotons"                   photons
      //   //    Add only what your process actually needs; extra collections
      //   //    slow down the container-building step.
      //   {"TruthTop", "TruthBosonsWithDecayParticles"},
      //
      //   // 2. decoratorGroups — which sets of EventInfo decorators to
      //   //    initialise (one Initialize*Decorators() call each).
      //   //    Available groups (see PartonHistoryDecorators.cxx):
      //   //      DecoratorGroup::Top              MC_t_*
      //   //      DecoratorGroup::AntiTop          MC_tbar_*
      //   //      DecoratorGroup::FourTop          MC_t1/t2/tbar1/tbar2_*
      //   //      DecoratorGroup::Ttbar            MC_ttbar_*
      //   //      DecoratorGroup::Bottom           MC_b_* (scalar)
      //   //      DecoratorGroup::AntiBottom       MC_bbar_* (scalar)
      //   //      DecoratorGroup::VectorBottom     MC_b_* (vector)
      //   //      DecoratorGroup::VectorAntiBottom MC_bbar_* (vector)
      //   //      DecoratorGroup::Charm            MC_c_* (scalar)
      //   //      DecoratorGroup::AntiCharm        MC_cbar_* (scalar)
      //   //      DecoratorGroup::VectorCharm      MC_c_* (vector)
      //   //      DecoratorGroup::VectorAntiCharm  MC_cbar_* (vector)
      //   //      DecoratorGroup::Photon           MC_gamma_*
      //   //      DecoratorGroup::Higgs            MC_H_*, MC_Hdecay{1,2}_*
      //   {DecoratorGroup::Top, DecoratorGroup::AntiTop,
      //   DecoratorGroup::Ttbar},
      //
      //   // 3. decoratorZWs — parameterised Z/W decorator groups.
      //   //    Each entry is a DecoratorZW with fields:
      //   //      type    : DecoratorZW::Z or DecoratorZW::W
      //   //      count   : number of bosons (1 or 2)
      //   //      extended: (Z only) true to also initialise tau-decay
      //   //                sub-products (MC_Zdecay1_decay{1,2,3}_*)
      //   //    Leave the list empty ({}) if no Z/W decorators are needed.
      //   {{DecoratorZW::Z, 1, false}},
      //
      //   // 4. specialFills — calls to dedicated Fill*PartonHistory methods,
      //   //    executed in order. Each entry is a SpecialFillOp with fields:
      //   //      type  :
      //   SpecialFillType::{Top,AntiTop,Ttbar,Z,Ztautau,W,Higgs,Gamma}
      //   //      parent: (Z/W) parent particle string, e.g. "t", "tbar",
      //   //              or "" for a standalone boson
      //   //      mode  : (Z/W/H) "resonant"       — boson present in truth
      //   record
      //   //                      "non_resonant"    — reconstruct from decay
      //   products
      //   //                      "non_resonant_WW" — H→WW off-shell (Higgs
      //   only)
      //   //                      "single_top"      — H in single-top context
      //   //      count : (Z/W) number of bosons to reconstruct (default 1)
      //   //    Leave the list empty ({}) if all filling is done via
      //   genericFills.
      //   {
      //       {SpecialFillType::Top},
      //       {SpecialFillType::AntiTop},
      //       {SpecialFillType::Ttbar},
      //       {SpecialFillType::Z, "", "resonant", 1},
      //   },
      //
      //   // 5. genericFills — calls to FillGenericPartonHistory, executed
      //   after
      //   //    all specialFills. Each entry is a GenericFillOp with fields:
      //   //      retrievalKeys : list of m_particleMap keys to try in order
      //   //                      (first successful hit wins); keys are bare
      //   //                      suffixes — the scheme prefix is prepended
      //   //                      automatically, e.g. "MC_t_beforeFSR" becomes
      //   //                      "MySchemeName_MC_t_beforeFSR" at runtime.
      //   //                      Common key patterns built by TraceParticles:
      //   //                        MC_{particle}_beforeFSR / _afterFSR
      //   //                        MC_{parent}_{particle}_{fsr}
      //   //                        MC_{particle}Decay{1,2}_{fsr}  (W/Z/H
      //   daughters)
      //   //      decorationKey : bare output branch name written to EventInfo
      //   //                      (prefix is prepended automatically).
      //   //      idx           : occurrence index into the m_particleMap vector,
      //   //                      for processes with multiple identical
      //   particles
      //   //                      (e.g. idx=0 for first top, idx=1 for second).
      //   //      isVector      : true → use FillGenericVectorPartonHistory,
      //   //                      which writes a std::vector<float/int> branch
      //   //                      collecting all entries in the map key.
      //   {
      //       {{"MC_t_beforeFSR", "MC_tbar_beforeFSR"}, "MC_t_beforeFSR", 0},
      //       {{"MC_b_beforeFSR", "MC_bbar_beforeFSR"}, "MC_b_beforeFSR", 0},
      //   }
      //
      // }},
      // ================================================================== //

      // ------------------------------------------------------------------ //
      // Ttbar
      // ------------------------------------------------------------------ //
      {"Ttbar",
       {{"TruthTop"},
        {DecoratorGroup::Top, DecoratorGroup::AntiTop, DecoratorGroup::Ttbar},
        {},
        {
            {SpecialFillType::Top},
            {SpecialFillType::AntiTop},
            {SpecialFillType::Ttbar},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // TtbarFCNC: t -> q X, X -> decay1 decay2
      // ------------------------------------------------------------------ //
      {"TtbarFCNC",
       {{"TruthTop"},
        {DecoratorGroup::TopFCNC, DecoratorGroup::AntiTopFCNC, DecoratorGroup::Ttbar},
        {},
        {
            {SpecialFillType::TopFCNC},
            {SpecialFillType::AntiTopFCNC},
            {SpecialFillType::TtbarFCNC},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // Ttbarbbbar
      // ------------------------------------------------------------------ //
      {"Ttbarbbbar",
       {{"TruthTop", "TruthBottom"},
        {DecoratorGroup::Top, DecoratorGroup::AntiTop,
         DecoratorGroup::VectorBottom, DecoratorGroup::VectorAntiBottom,
         DecoratorGroup::Ttbar},
        {},
        {
            {SpecialFillType::Top},
            {SpecialFillType::AntiTop},
            {SpecialFillType::Ttbar},
        },
        {
            {{"MC_b_beforeFSR"}, "MC_b_beforeFSR", 0, true},
            {{"MC_bbar_beforeFSR"}, "MC_bbar_beforeFSR", 0, true},
            {{"MC_b_afterFSR"}, "MC_b_afterFSR", 0, true},
            {{"MC_bbar_afterFSR"}, "MC_bbar_afterFSR", 0, true},
        },
        {}}},

      // ------------------------------------------------------------------ //
      // Ttbarccbar
      // ------------------------------------------------------------------ //
      {"Ttbarccbar",
       {{"TruthTop", "TruthCharm"},
        {DecoratorGroup::Top, DecoratorGroup::AntiTop,
         DecoratorGroup::VectorCharm, DecoratorGroup::VectorAntiCharm,
         DecoratorGroup::Ttbar},
        {},
        {
            {SpecialFillType::Top},
            {SpecialFillType::AntiTop},
            {SpecialFillType::Ttbar},
        },
        {
            {{"MC_c_beforeFSR"}, "MC_c_beforeFSR", 0, true},
            {{"MC_cbar_beforeFSR"}, "MC_cbar_beforeFSR", 0, true},
            {{"MC_c_afterFSR"}, "MC_c_afterFSR", 0, true},
            {{"MC_cbar_afterFSR"}, "MC_cbar_afterFSR", 0, true},
        },
        {}}},

      // ------------------------------------------------------------------ //
      // Ttz
      // ------------------------------------------------------------------ //
      {"Ttz",
       {{"TruthTop", "TruthBosonsWithDecayParticles", "TruthElectrons",
         "TruthMuons", "TruthTaus"},
        {DecoratorGroup::Top, DecoratorGroup::AntiTop, DecoratorGroup::Ttbar},
        {{DecoratorZW::Z, 1, false}},
        {
            {SpecialFillType::Top},
            {SpecialFillType::AntiTop},
            {SpecialFillType::Ttbar},
            {SpecialFillType::Z, "", "resonant", 1},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // Ttw
      // ------------------------------------------------------------------ //
      {"Ttw",
       {{"TruthTop", "TruthBosonsWithDecayParticles"},
        {DecoratorGroup::Top, DecoratorGroup::AntiTop, DecoratorGroup::Ttbar},
        {{DecoratorZW::W, 1}},
        {
            {SpecialFillType::Top},
            {SpecialFillType::AntiTop},
            {SpecialFillType::Ttbar},
            // Associated W (either charge: both map to the MC_W_* keys)
            {SpecialFillType::W, "", "resonant", 1},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // Tth
      // ------------------------------------------------------------------ //
      {"Tth",
       {{"TruthTop", "TruthBosonsWithDecayParticles", "TruthBottom"},
        {DecoratorGroup::Top, DecoratorGroup::AntiTop, DecoratorGroup::Ttbar,
         DecoratorGroup::Higgs},
        {},
        {
            {SpecialFillType::Top},
            {SpecialFillType::AntiTop},
            {SpecialFillType::Ttbar},
            {SpecialFillType::Higgs, "", "resonant"},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // Ttgamma
      // ------------------------------------------------------------------ //
      {"Ttgamma",
       {{"TruthTop", "TruthBosonsWithDecayParticles", "TruthPhotons"},
        {DecoratorGroup::Top, DecoratorGroup::AntiTop, DecoratorGroup::Ttbar,
         DecoratorGroup::Photon},
        {},
        {
            {SpecialFillType::Top},
            {SpecialFillType::AntiTop},
            {SpecialFillType::Ttbar},
            {SpecialFillType::Gamma, ""},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // Tq
      // ------------------------------------------------------------------ //
      {"Tq",
       {{"TruthTop", "TruthBosonsWithDecayParticles", "TruthBottom"},
        {DecoratorGroup::Top, DecoratorGroup::Bottom},
        {},
        {},
        {
            // Top (flavour-agnostic)
            {{"MC_t_beforeFSR", "MC_tbar_beforeFSR"}, "MC_t_beforeFSR", 0},
            {{"MC_t_b_beforeFSR", "MC_tbar_bbar_beforeFSR"},
             "MC_b_beforeFSR_from_t",
             0},
            {{"MC_t_afterFSR", "MC_tbar_afterFSR"}, "MC_t_afterFSR", 0},
            {{"MC_t_b_afterFSR", "MC_tbar_bbar_afterFSR"},
             "MC_b_afterFSR_from_t",
             0},
            // W from top
            {{"MC_t_W_beforeFSR", "MC_tbar_W_beforeFSR"},
             "MC_W_beforeFSR_from_t",
             0},
            {{"MC_t_W_afterFSR", "MC_tbar_W_afterFSR"},
             "MC_W_afterFSR_from_t",
             0},
            {{"MC_t_WDecay1_beforeFSR", "MC_tbar_WDecay1_beforeFSR"},
             "MC_Wdecay1_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay1_afterFSR", "MC_tbar_WDecay1_afterFSR"},
             "MC_Wdecay1_afterFSR_from_t",
             0},
            {{"MC_t_WDecay2_beforeFSR", "MC_tbar_WDecay2_beforeFSR"},
             "MC_Wdecay2_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay2_afterFSR", "MC_tbar_WDecay2_afterFSR"},
             "MC_Wdecay2_afterFSR_from_t",
             0},
            // Spectator b
            {{"MC_b_beforeFSR", "MC_bbar_beforeFSR"}, "MC_b_beforeFSR", 0},
            {{"MC_b_afterFSR", "MC_bbar_afterFSR"}, "MC_b_afterFSR", 0},
        },
        {}}},

      // ------------------------------------------------------------------ //
      // Tzq
      // ------------------------------------------------------------------ //
      {"Tzq",
       {{"TruthTop", "TruthBosonsWithDecayParticles", "TruthBottom"},
        {DecoratorGroup::Top, DecoratorGroup::Bottom},
        {{DecoratorZW::Z, 1, false}},
        {
            {SpecialFillType::Z, "", "resonant", 1},
        },
        {
            // Top (flavour-agnostic: try t then tbar)
            {{"MC_t_beforeFSR", "MC_tbar_beforeFSR"}, "MC_t_beforeFSR", 0},
            {{"MC_t_b_beforeFSR", "MC_tbar_bbar_beforeFSR"},
             "MC_b_beforeFSR_from_t",
             0},
            {{"MC_t_afterFSR", "MC_tbar_afterFSR"}, "MC_t_afterFSR", 0},
            {{"MC_t_b_afterFSR", "MC_tbar_bbar_afterFSR"},
             "MC_b_afterFSR_from_t",
             0},
            // W from top
            {{"MC_t_W_beforeFSR", "MC_tbar_W_beforeFSR"},
             "MC_W_beforeFSR_from_t",
             0},
            {{"MC_t_W_afterFSR", "MC_tbar_W_afterFSR"},
             "MC_W_afterFSR_from_t",
             0},
            {{"MC_t_WDecay1_beforeFSR", "MC_tbar_WDecay1_beforeFSR"},
             "MC_Wdecay1_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay1_afterFSR", "MC_tbar_WDecay1_afterFSR"},
             "MC_Wdecay1_afterFSR_from_t",
             0},
            {{"MC_t_WDecay2_beforeFSR", "MC_tbar_WDecay2_beforeFSR"},
             "MC_Wdecay2_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay2_afterFSR", "MC_tbar_WDecay2_afterFSR"},
             "MC_Wdecay2_afterFSR_from_t",
             0},
            // Spectator b (flavour-agnostic)
            {{"MC_b_beforeFSR", "MC_bbar_beforeFSR"}, "MC_b_beforeFSR", 0},
            {{"MC_b_afterFSR", "MC_bbar_afterFSR"}, "MC_b_afterFSR", 0},
        },
        {}}},

      // ------------------------------------------------------------------ //
      // Thq
      // ------------------------------------------------------------------ //
      {"Thq",
       {{"TruthTop", "TruthBosonsWithDecayParticles", "TruthBottom"},
        {DecoratorGroup::Top, DecoratorGroup::Bottom, DecoratorGroup::Higgs},
        {{DecoratorZW::W, 1}},
        {
            {SpecialFillType::Higgs, "", "single_top"},
        },
        {
            // Top — 4-key alternatives (b-initiated chains)
            {{"MC_t_beforeFSR", "MC_tbar_beforeFSR", "MC_b_t_beforeFSR",
              "MC_bbar_tbar_beforeFSR"},
             "MC_t_beforeFSR",
             0},
            {{"MC_t_b_beforeFSR", "MC_tbar_bbar_beforeFSR",
              "MC_b_t_b_beforeFSR", "MC_bbar_tbar_bbar_beforeFSR"},
             "MC_b_beforeFSR_from_t",
             0},
            {{"MC_t_W_beforeFSR", "MC_tbar_W_beforeFSR", "MC_b_t_W_beforeFSR",
              "MC_bbar_tbar_W_beforeFSR"},
             "MC_W_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay1_beforeFSR", "MC_tbar_WDecay1_beforeFSR",
              "MC_b_t_WDecay1_beforeFSR", "MC_bbar_tbar_WDecay1_beforeFSR"},
             "MC_Wdecay1_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay2_beforeFSR", "MC_tbar_WDecay2_beforeFSR",
              "MC_b_t_WDecay2_beforeFSR", "MC_bbar_tbar_WDecay2_beforeFSR"},
             "MC_Wdecay2_beforeFSR_from_t",
             0},
            {{"MC_t_afterFSR", "MC_tbar_afterFSR", "MC_b_t_afterFSR",
              "MC_bbar_tbar_afterFSR"},
             "MC_t_afterFSR",
             0},
            {{"MC_t_b_afterFSR", "MC_tbar_bbar_afterFSR", "MC_b_t_b_afterFSR",
              "MC_bbar_tbar_bbar_afterFSR"},
             "MC_b_afterFSR_from_t",
             0},
            {{"MC_t_W_afterFSR", "MC_tbar_W_afterFSR", "MC_b_t_W_afterFSR",
              "MC_bbar_tbar_W_afterFSR"},
             "MC_W_afterFSR_from_t",
             0},
            {{"MC_t_WDecay1_afterFSR", "MC_tbar_WDecay1_afterFSR",
              "MC_b_t_WDecay1_afterFSR", "MC_bbar_tbar_WDecay1_afterFSR"},
             "MC_Wdecay1_afterFSR_from_t",
             0},
            {{"MC_t_WDecay2_afterFSR", "MC_tbar_WDecay2_afterFSR",
              "MC_b_t_WDecay2_afterFSR", "MC_bbar_tbar_WDecay2_afterFSR"},
             "MC_Wdecay2_afterFSR_from_t",
             0},
            // Spectator b
            {{"MC_b_beforeFSR", "MC_bbar_beforeFSR"}, "MC_b_beforeFSR", 0},
            {{"MC_b_afterFSR", "MC_bbar_afterFSR"}, "MC_b_afterFSR", 0},
            // W from b-initiated chains or standalone
            {{"MC_b_W_beforeFSR", "MC_bbar_W_beforeFSR", "MC_W_beforeFSR"},
             "MC_W_beforeFSR",
             0},
            {{"MC_b_W_afterFSR", "MC_bbar_W_afterFSR", "MC_W_afterFSR"},
             "MC_W_afterFSR",
             0},
            {{"MC_b_WDecay1_beforeFSR", "MC_bbar_WDecay1_beforeFSR",
              "MC_WDecay1_beforeFSR"},
             "MC_Wdecay1_beforeFSR",
             0},
            {{"MC_b_WDecay2_beforeFSR", "MC_bbar_WDecay2_beforeFSR",
              "MC_WDecay2_beforeFSR"},
             "MC_Wdecay2_beforeFSR",
             0},
            {{"MC_b_WDecay1_afterFSR", "MC_bbar_WDecay1_afterFSR",
              "MC_WDecay1_afterFSR"},
             "MC_Wdecay1_afterFSR",
             0},
            {{"MC_b_WDecay2_afterFSR", "MC_bbar_WDecay2_afterFSR",
              "MC_WDecay2_afterFSR"},
             "MC_Wdecay2_afterFSR",
             0},
        },
        {
            {{"MC_b_W_beforeFSR", "MC_bbar_W_beforeFSR", "MC_W_beforeFSR"},
             "MC_W_IsOnShell"},
        }}},

      // ------------------------------------------------------------------ //
      // Tqgamma
      // ------------------------------------------------------------------ //
      {"Tqgamma",
       {{"TruthTop", "TruthBosonsWithDecayParticles", "TruthPhotons",
         "TruthBottom"},
        {DecoratorGroup::Top, DecoratorGroup::Bottom, DecoratorGroup::Photon},
        {},
        {
            {SpecialFillType::Gamma, ""},
        },
        {
            // Top (flavour-agnostic)
            {{"MC_t_beforeFSR", "MC_tbar_beforeFSR"}, "MC_t_beforeFSR", 0},
            {{"MC_t_b_beforeFSR", "MC_tbar_bbar_beforeFSR"},
             "MC_b_beforeFSR_from_t",
             0},
            {{"MC_t_afterFSR", "MC_tbar_afterFSR"}, "MC_t_afterFSR", 0},
            {{"MC_t_b_afterFSR", "MC_tbar_bbar_afterFSR"},
             "MC_b_afterFSR_from_t",
             0},
            // W from top
            {{"MC_t_W_beforeFSR", "MC_tbar_W_beforeFSR"},
             "MC_W_beforeFSR_from_t",
             0},
            {{"MC_t_W_afterFSR", "MC_tbar_W_afterFSR"},
             "MC_W_afterFSR_from_t",
             0},
            {{"MC_t_WDecay1_beforeFSR", "MC_tbar_WDecay1_beforeFSR"},
             "MC_Wdecay1_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay1_afterFSR", "MC_tbar_WDecay1_afterFSR"},
             "MC_Wdecay1_afterFSR_from_t",
             0},
            {{"MC_t_WDecay2_beforeFSR", "MC_tbar_WDecay2_beforeFSR"},
             "MC_Wdecay2_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay2_afterFSR", "MC_tbar_WDecay2_afterFSR"},
             "MC_Wdecay2_afterFSR_from_t",
             0},
            // Spectator b
            {{"MC_b_beforeFSR", "MC_bbar_beforeFSR"}, "MC_b_beforeFSR", 0},
            {{"MC_b_afterFSR", "MC_bbar_afterFSR"}, "MC_b_afterFSR", 0},
        },
        {}}},

      // ------------------------------------------------------------------ //
      // Wtb
      // ------------------------------------------------------------------ //
      {"Wtb",
       {{"TruthTop", "TruthBosonsWithDecayParticles", "TruthBottom"},
        {DecoratorGroup::Top, DecoratorGroup::Bottom},
        {{DecoratorZW::W, 1}},
        {
            // Spectator W (associated, not from top decay)
            {SpecialFillType::W, "", "resonant", 1},
        },
        {
            // Top — 4-key alternatives
            {{"MC_t_beforeFSR", "MC_tbar_beforeFSR", "MC_b_t_beforeFSR",
              "MC_bbar_tbar_beforeFSR"},
             "MC_t_beforeFSR",
             0},
            {{"MC_t_b_beforeFSR", "MC_tbar_bbar_beforeFSR",
              "MC_b_t_b_beforeFSR", "MC_bbar_tbar_bbar_beforeFSR"},
             "MC_b_beforeFSR_from_t",
             0},
            {{"MC_t_W_beforeFSR", "MC_tbar_W_beforeFSR",
              "MC_b_t_W_beforeFSR", "MC_bbar_tbar_W_beforeFSR"},
             "MC_W_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay1_beforeFSR", "MC_tbar_WDecay1_beforeFSR",
              "MC_b_t_WDecay1_beforeFSR", "MC_bbar_tbar_WDecay1_beforeFSR"},
             "MC_Wdecay1_beforeFSR_from_t",
             0},
            {{"MC_t_WDecay2_beforeFSR", "MC_tbar_WDecay2_beforeFSR",
              "MC_b_t_WDecay2_beforeFSR", "MC_bbar_tbar_WDecay2_beforeFSR"},
             "MC_Wdecay2_beforeFSR_from_t",
             0},
            {{"MC_t_afterFSR", "MC_tbar_afterFSR", "MC_b_t_afterFSR",
              "MC_bbar_tbar_afterFSR"},
             "MC_t_afterFSR",
             0},
            {{"MC_t_b_afterFSR", "MC_tbar_bbar_afterFSR", "MC_b_t_b_afterFSR",
              "MC_bbar_tbar_bbar_afterFSR"},
             "MC_b_afterFSR_from_t",
             0},
            {{"MC_t_W_afterFSR", "MC_tbar_W_afterFSR", "MC_b_t_W_afterFSR",
              "MC_bbar_tbar_W_afterFSR"},
             "MC_W_afterFSR_from_t",
             0},
            {{"MC_t_WDecay1_afterFSR", "MC_tbar_WDecay1_afterFSR",
              "MC_b_t_WDecay1_afterFSR", "MC_bbar_tbar_WDecay1_afterFSR"},
             "MC_Wdecay1_afterFSR_from_t",
             0},
            {{"MC_t_WDecay2_afterFSR", "MC_tbar_WDecay2_afterFSR",
              "MC_b_t_WDecay2_afterFSR", "MC_bbar_tbar_WDecay2_afterFSR"},
             "MC_Wdecay2_afterFSR_from_t",
             0},
            // Spectator b
            {{"MC_b_beforeFSR", "MC_bbar_beforeFSR"}, "MC_b_beforeFSR", 0},
            {{"MC_b_afterFSR", "MC_bbar_afterFSR"}, "MC_b_afterFSR", 0},
        },
        {}}},

      // ------------------------------------------------------------------ //
      // FourTop
      // ------------------------------------------------------------------ //
      {"FourTop",
       {{"TruthTop"},
        {DecoratorGroup::FourTop},
        {},
        {},  // no special fills
        {
            // Top 1 (idx=0)
            {{"MC_t_beforeFSR"}, "MC_t1_beforeFSR", 0},
            {{"MC_t_b_beforeFSR"}, "MC_b_beforeFSR_from_t1", 0},
            {{"MC_t_W_beforeFSR"}, "MC_W_beforeFSR_from_t1", 0},
            {{"MC_t_WDecay1_beforeFSR"}, "MC_Wdecay1_beforeFSR_from_t1", 0},
            {{"MC_t_WDecay2_beforeFSR"}, "MC_Wdecay2_beforeFSR_from_t1", 0},
            {{"MC_t_afterFSR"}, "MC_t1_afterFSR", 0},
            {{"MC_t_b_afterFSR"}, "MC_b_afterFSR_from_t1", 0},
            {{"MC_t_W_afterFSR"}, "MC_W_afterFSR_from_t1", 0},
            {{"MC_t_WDecay1_afterFSR"}, "MC_Wdecay1_afterFSR_from_t1", 0},
            {{"MC_t_WDecay2_afterFSR"}, "MC_Wdecay2_afterFSR_from_t1", 0},
            // Top 2 (idx=1)
            {{"MC_t_beforeFSR"}, "MC_t2_beforeFSR", 1},
            {{"MC_t_b_beforeFSR"}, "MC_b_beforeFSR_from_t2", 1},
            {{"MC_t_W_beforeFSR"}, "MC_W_beforeFSR_from_t2", 1},
            {{"MC_t_WDecay1_beforeFSR"}, "MC_Wdecay1_beforeFSR_from_t2", 1},
            {{"MC_t_WDecay2_beforeFSR"}, "MC_Wdecay2_beforeFSR_from_t2", 1},
            {{"MC_t_afterFSR"}, "MC_t2_afterFSR", 1},
            {{"MC_t_b_afterFSR"}, "MC_b_afterFSR_from_t2", 1},
            {{"MC_t_W_afterFSR"}, "MC_W_afterFSR_from_t2", 1},
            {{"MC_t_WDecay1_afterFSR"}, "MC_Wdecay1_afterFSR_from_t2", 1},
            {{"MC_t_WDecay2_afterFSR"}, "MC_Wdecay2_afterFSR_from_t2", 1},
            // AntiTop 1 (idx=0)
            {{"MC_tbar_beforeFSR"}, "MC_tbar1_beforeFSR", 0},
            {{"MC_tbar_bbar_beforeFSR"}, "MC_bbar_beforeFSR_from_tbar1", 0},
            {{"MC_tbar_W_beforeFSR"}, "MC_W_beforeFSR_from_tbar1", 0},
            {{"MC_tbar_WDecay1_beforeFSR"},
             "MC_Wdecay1_beforeFSR_from_tbar1",
             0},
            {{"MC_tbar_WDecay2_beforeFSR"},
             "MC_Wdecay2_beforeFSR_from_tbar1",
             0},
            {{"MC_tbar_afterFSR"}, "MC_tbar1_afterFSR", 0},
            {{"MC_tbar_bbar_afterFSR"}, "MC_bbar_afterFSR_from_tbar1", 0},
            {{"MC_tbar_W_afterFSR"}, "MC_W_afterFSR_from_tbar1", 0},
            {{"MC_tbar_WDecay1_afterFSR"}, "MC_Wdecay1_afterFSR_from_tbar1", 0},
            {{"MC_tbar_WDecay2_afterFSR"}, "MC_Wdecay2_afterFSR_from_tbar1", 0},
            // AntiTop 2 (idx=1)
            {{"MC_tbar_beforeFSR"}, "MC_tbar2_beforeFSR", 1},
            {{"MC_tbar_bbar_beforeFSR"}, "MC_bbar_beforeFSR_from_tbar2", 1},
            {{"MC_tbar_W_beforeFSR"}, "MC_W_beforeFSR_from_tbar2", 1},
            {{"MC_tbar_WDecay1_beforeFSR"},
             "MC_Wdecay1_beforeFSR_from_tbar2",
             1},
            {{"MC_tbar_WDecay2_beforeFSR"},
             "MC_Wdecay2_beforeFSR_from_tbar2",
             1},
            {{"MC_tbar_afterFSR"}, "MC_tbar2_afterFSR", 1},
            {{"MC_tbar_bbar_afterFSR"}, "MC_bbar_afterFSR_from_tbar2", 1},
            {{"MC_tbar_W_afterFSR"}, "MC_W_afterFSR_from_tbar2", 1},
            {{"MC_tbar_WDecay1_afterFSR"}, "MC_Wdecay1_afterFSR_from_tbar2", 1},
            {{"MC_tbar_WDecay2_afterFSR"}, "MC_Wdecay2_afterFSR_from_tbar2", 1},
        },
        {}}},

      // ------------------------------------------------------------------ //
      // WW_nonresonant
      // NOTE: the truthCollections list of this scheme has not been validated
      // ------------------------------------------------------------------ //
      {"WW_nonresonant",
       {{"TruthBosonsWithDecayParticles", "TruthElectrons", "TruthMuons",
         "TruthTaus", "TruthNeutrinos"},
        {},
        {{DecoratorZW::W, 2}},
        {
            {SpecialFillType::W, "", "non_resonant", 2},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // HWW
      // NOTE: the truthCollections list of this scheme has not been validated
      // ------------------------------------------------------------------ //
      {"HWW",
       {{"TruthBosonsWithDecayParticles"},
        {DecoratorGroup::Higgs},
        {},
        {
            {SpecialFillType::Higgs, "", "resonant"},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // HWW_nonresonant
      // ------------------------------------------------------------------ //
      {"HWW_nonresonant",
       {{"TruthBosonsWithDecayParticles", "TruthElectrons", "TruthMuons",
         "TruthTaus", "TruthNeutrinos"},
        {DecoratorGroup::Higgs},
        {},
        {
            {SpecialFillType::Higgs, "", "non_resonant_WW"},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // HZZ
      // NOTE: the truthCollections list of this scheme has not been validated
      // ------------------------------------------------------------------ //
      {"HZZ",
       {{"TruthBosonsWithDecayParticles"},
        {DecoratorGroup::Higgs},
        {},
        {
            {SpecialFillType::Higgs, "", "resonant"},
        },
        {},
        {}}},

      // ------------------------------------------------------------------ //
      // Zb
      // NOTE: idx=2 and idx=1 reproduce the original code exactly
      // ------------------------------------------------------------------ //
      {"Zb",
       {{"TruthBosonsWithDecayParticles", "TruthBottom", "TruthMuons",
         "TruthElectrons"},
        {DecoratorGroup::Bottom, DecoratorGroup::AntiBottom},
        {{DecoratorZW::Z, 1, false}},
        {
            {SpecialFillType::Z, "", "resonant", 1},
        },
        {
            {{"MC_b_beforeFSR", "MC_bbar_beforeFSR"}, "MC_b_beforeFSR", 2},
            {{"MC_b_afterFSR", "MC_bbar_afterFSR"}, "MC_b_afterFSR", 1},
            {{"MC_bbar_beforeFSR"}, "MC_bbar_beforeFSR", 0},
            {{"MC_bbar_afterFSR"}, "MC_bbar_afterFSR", 0},
        },
        {}}},

      // ------------------------------------------------------------------ //
      // Ztautau
      // ------------------------------------------------------------------ //
      {"Ztautau",
       {{"TruthBosonsWithDecayParticles", "TruthTaus", "TruthNeutrinos"},
        {},
        {{DecoratorZW::Z, 1, true}},  // extended=true for tau decay products
        {
            {SpecialFillType::Ztautau, "", "non_resonant", 1},
        },
        {},
        {}}},

  };  // end registry

  auto it = registry.find(schemeName);
  if (it == registry.end()) {
    throw std::runtime_error("Unknown parton scheme: " + schemeName);
  }
  return it->second;
}

}  // namespace CP
