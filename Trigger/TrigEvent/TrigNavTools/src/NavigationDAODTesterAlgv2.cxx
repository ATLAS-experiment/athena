/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "NavigationDAODTesterAlgv2.h"

#include <GaudiKernel/StatusCode.h>
#include <algorithm>

#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigDecisionTool/Conditions.h"
#include "TrigAnalysisHelpers/FeatureRequestDescriptor.h"
#include "TrigCompositeUtils/Combinators.h"
#include "xAODTrigger/TrigCompositeContainer.h"

namespace TCU = TrigCompositeUtils;

namespace Trig {

NavigationDAODTesterAlgv2::NavigationDAODTesterAlgv2(const std::string &name, ISvcLocator *pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode NavigationDAODTesterAlgv2::initialize() {
    ATH_CHECK(m_tdt.retrieve());
    ATH_CHECK(m_matchingTool.retrieve());
    ATH_CHECK(m_matchFromCompositeTool.retrieve());

    if (m_chains.empty()) {
        ATH_MSG_WARNING("No chains provided, algorithm will be no-op");
    }

    return StatusCode::SUCCESS;
}

std::size_t NavigationDAODTesterAlgv2::numberOfLegs(const std::string& chain) const {
    // The number of legs is the number of distinct signature blocks in the
    // chain name, i.e. the size of the multiplicities vector. Note this is NOT
    // the sum of the multiplicities (that would be the total number of physics
    // objects required by the chain over all legs).
    return ChainNameParser::multiplicities(chain).size();
}

bool NavigationDAODTesterAlgv2::matchesAnyPattern(const std::string& chain,
                                                  const std::vector<std::string>& patterns) {
    for (const std::string& pattern : patterns) {
        if (chain.find(pattern) != std::string::npos) return true;
    }
    return false;
}

const xAOD::IParticle* NavigationDAODTesterAlgv2::resolveOriginal(const xAOD::IParticle* p) {
    static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::IParticleContainer>>
        accOOL("originalObjectLink");
    if (accOOL.isAvailable(*p) && accOOL(*p).isValid()) {
        return *accOOL(*p);
    }
    return p;
}

bool NavigationDAODTesterAlgv2::matchLinearisedR2(
        std::vector<const xAOD::IParticle*> pool,
        const std::vector<const xAOD::IParticle*>& particles) {
    // Consume-once semantics, mirroring MatchFromCompositeTool::testCombination:
    // each given particle must find its own entry in the pool of individually
    // matched objects. For the single-particle calls of the per-object
    // comparison path this reduces to a plain membership test.
    for (const xAOD::IParticle* p : particles) {
        auto itr = std::find(pool.begin(), pool.end(), resolveOriginal(p));
        if (itr == pool.end()) return false;
        pool.erase(itr);
    }
    return true;
}

void NavigationDAODTesterAlgv2::dumpFeaturesPerLeg(const std::string& chain,
                                                   std::size_t nLegs,
                                                   MSG::Level level,
                                                   const std::string& linkName) const {
    const bool isSub = !linkName.empty();
    const char* tag = isSub ? "[subfeature]" : "[feature]";
    for (std::size_t leg = 0; leg < nLegs; ++leg) {
        Trig::FeatureRequestDescriptor frd;
        frd.setChainGroup(chain);
        frd.setRestrictRequestToLeg(static_cast<int>(leg));
        if (isSub) {
            frd.setLinkName(linkName);
        }
        // Default feature collection mode (lastFeatureOfType) is correct for
        // DAOD - only the final feature is kept after slimming.
        auto features = m_tdt->features<xAOD::IParticleContainer>(frd);
        msg() << level << "  Leg " << leg << " "
              << (isSub ? "subfeatures" : "features") << " (" << features.size() << "):" << endmsg;
        for (const auto& feature : features) {
            if (feature.isValid()) {
                msg() << level << "    " << tag << " pt=" << (*feature.link)->pt()
                      << " eta=" << (*feature.link)->eta()
                      << " phi=" << (*feature.link)->phi()
                      << " [" << feature.link.dataID() << ":" << feature.link.index() << "]" << endmsg;
            }
        }
    }
}

StatusCode NavigationDAODTesterAlgv2::execute(const EventContext& ctx) const {

    // Get offline particle containers
    SG::ReadHandle<xAOD::IParticleContainer> particles_muons{"Muons", ctx};
    SG::ReadHandle<xAOD::IParticleContainer> particles_electrons{"Electrons", ctx};
    SG::ReadHandle<xAOD::IParticleContainer> particles_taus{"TauJets", ctx};
    SG::ReadHandle<xAOD::IParticleContainer> particles_photons{"Photons", ctx};

    std::map<std::string, SG::ReadHandle<xAOD::IParticleContainer>*> read_handles;
    read_handles["e"] = &particles_electrons;
    read_handles["mu"] = &particles_muons;
    read_handles["tau"] = &particles_taus;
    read_handles["g"] = &particles_photons;

    if (!particles_muons.isValid() || !particles_electrons.isValid() ||
        !particles_taus.isValid() || !particles_photons.isValid()) {
        ATH_MSG_ERROR("Couldn't retrieve IParticles containers");
        return StatusCode::FAILURE;
    }

    for (const std::string& chain : m_chains) {
        if (!m_tdt->isPassed(chain, TrigDefs::Physics | TrigDefs::allowResurrectedDecision)) continue;

        // Skip chains on the exception list (substring match). These are
        // pathological chain families whose TrigMatch layout does not support
        // the generic R2/R3 comparison and have no dedicated handling (yet).
        if (matchesAnyPattern(chain, m_chainsToExclude)) {
            ATH_MSG_DEBUG("Chain " << chain << " is on the exclusion list, skipping");
            continue;
        }

        // Get the chain structure using ChainNameParser
        auto ChainMultiplicity = ChainNameParser::multiplicities(chain);
        auto ChainNameParseSignature = ChainNameParser::signatures(chain);

        // Filter to only process chains with supported signatures (e, mu, g, tau)
        bool hasSupported = false;
        for (const std::string& sig : ChainNameParseSignature) {
            if (read_handles.find(sig) != read_handles.end()) {
                hasSupported = true;
                break;
            }
        }
        if (!hasSupported) {
            ATH_MSG_DEBUG("Chain " << chain << " has no supported signatures, skipping");
            continue;
        }
        ATH_MSG_DEBUG("Passing " << chain);

        // DR=0.2 for chains with any tau leg, 0.1 otherwise. This follows the
        // convention used when the TrigMatch branches are created at DAOD
        // production time (see DerivationFrameworkPhys/PhysCommonConfig.py).
        const bool hasTau = std::find(ChainNameParseSignature.begin(),
                                      ChainNameParseSignature.end(),
                                      "tau") != ChainNameParseSignature.end();
        const double drThreshold = hasTau ? 0.2 : 0.1;

        // Retrieve the TrigMatch container (used for the R2 match and for the
        // DEBUG dump below).
        std::string containerName = m_inputPrefix + chain;
        std::replace(containerName.begin(), containerName.end(), '.', '_');
        const xAOD::TrigCompositeContainer* composites(nullptr);
        if (!evtStore()->retrieve(composites, containerName).isSuccess()) {
            ATH_MSG_DEBUG("No branch in this DAOD for " << chain);
            if (m_dumpFeaturesWithoutR2) {
                // Converter-debugging mode (e.g. directly after conversion at
                // AOD level): no R2 reference exists, but the per-leg feature
                // assignment - where the cross-leg pooling defect shows up -
                // can still be inspected.
                const std::size_t nLegs = numberOfLegs(chain);
                if (nLegs > 0) {
                    ATH_MSG_INFO("Per-leg features of " << chain << " (no R2 reference):");
                    dumpFeaturesPerLeg(chain, nLegs, MSG::INFO);
                    dumpFeaturesPerLeg(chain, nLegs, MSG::INFO, "subfeature");
                }
            }
            continue;
        }

        // nomucomb-family chains: the derivation stored the R2 matches
        // linearised - one TrigComposite entry of vector size 1 per
        // individually matched offline muon (e.g. HLT_2mu10_nomucomb with 3
        // matched muons -> 3 entries of size 1) instead of one entry of size
        // = sum of leg multiplicities per matched combination.
        // MatchFromCompositeTool requires the full combination within a single
        // entry, so it can never match there. Rebuild the R2 reference instead:
        // the pool of individually matched offline objects, gathered once per
        // chain/event.
        const bool useLinearisedR2 = matchesAnyPattern(chain, m_linearisedR2Chains);
        std::vector<const xAOD::IParticle*> r2MatchedPool;
        if (useLinearisedR2) {
            static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> accMatched("TrigMatchedObjects");
            for (const xAOD::TrigComposite* entry : *composites) {
                for (const ElementLink<xAOD::IParticleContainer>& link : accMatched(*entry)) {
                    if (!link.isValid()) continue;  // e.g. removed by thinning
                    const xAOD::IParticle* orig = resolveOriginal(*link);
                    if (std::find(r2MatchedPool.begin(), r2MatchedPool.end(), orig) == r2MatchedPool.end()) {
                        r2MatchedPool.push_back(orig);
                    }
                }
            }
            ATH_MSG_DEBUG("Chain " << chain << " uses linearised R2 matching: "
                          << composites->size() << " TrigMatch entries -> pool of "
                          << r2MatchedPool.size() << " individually matched objects");
        }

        bool isR2R3different = false;
        size_t nCombinationsTested = 0;
        bool anyPassR2 = false;
        bool anyPassR3 = false;

        if (useLinearisedR2) {
            // Per-object comparison. The linearised reference holds individually
            // matched objects with no combination structure and no online-object
            // identity, so combination-level R2 verdicts cannot be reconstructed
            // faithfully. Two benign cases would otherwise be flagged as ERRORs
            // (both observed with HLT_2mu10_nomucomb, run 284500):
            //  - a collimated offline pair matched to the SAME single online
            //    muon: per-object R2 stores both, while R3 correctly demands
            //    distinct online objects per combination;
            //  - the converter demotes the lower-pT online muon of a shared RoI
            //    to a "subfeature", which the baseline comparison excludes
            //    (IncludeSubfeatures=False).
            // Comparing per object keeps the genuine-defect signature visible:
            // an object the R2 derivation matched but whose feature the
            // converted navigation lost entirely still gives R2:1 R3:0.
            std::vector<const SG::ReadHandle<xAOD::IParticleContainer>*> doneContainers;
            for (const std::string& sig : ChainNameParseSignature) {
                auto handleItr = read_handles.find(sig);
                if (handleItr == read_handles.end()) continue;
                // Legs may share an offline container (e.g. mu6 + 2mu4):
                // process each container once.
                if (std::find(doneContainers.begin(), doneContainers.end(), handleItr->second) != doneContainers.end()) continue;
                doneContainers.push_back(handleItr->second);

                for (const xAOD::IParticle* p : **(handleItr->second)) {
                    const bool passR2 = matchLinearisedR2(r2MatchedPool, {p});
                    const bool passR3 = m_matchingTool->match(*p, chain, drThreshold, false);

                    anyPassR2 |= passR2;
                    anyPassR3 |= passR3;

                    if (passR2 && !passR3) {
                        ATH_MSG_ERROR("R2 passes but R3 fails for chain " << chain
                                      << " (per-object linearised check) - converted navigation may have lost this object");
                        ATH_MSG_ERROR("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi()
                                      << " R2:1 R3:0");
                        isR2R3different = true;

                        {
                            std::lock_guard<std::mutex> lock(m_failingChainsMutex);
                            m_failingChains[chain] += 1;
                        }
                    }
                    else if (!passR2 && passR3) {
                        ATH_MSG_DEBUG("R3 passes but R2 fails (expected) for chain " << chain
                                      << " (per-object linearised check)");
                        ATH_MSG_DEBUG("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi()
                                      << " R2:0 R3:1");
                    }
                    else {
                        ++nCombinationsTested;
                        if (passR2 && msgLvl(MSG::DEBUG)) {
                            ATH_MSG_DEBUG("R2/R3 match for chain " << chain
                                          << " (per-object linearised check) passR2: " << passR2 << " passR3: " << passR3);
                            ATH_MSG_DEBUG("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi());
                        }
                    }
                }
            }
        }
        else {

        // Setup combination generator based on multiplicities of offline physics objects for each leg
        HLT::NestedUniqueCombinationGenerator nucg;
        ATH_MSG_DEBUG("NestedUniqueCombinationGenerator: " << ChainMultiplicity.size() << " legs, " << ChainNameParseSignature.size() << " signatures");

        bool tooFewOffline = false;
        for (size_t readhandlesIndex = 0; readhandlesIndex < ChainNameParseSignature.size(); readhandlesIndex++) {
            const std::string& sig = ChainNameParseSignature[readhandlesIndex];
            if (read_handles.find(sig) == read_handles.end()) {
                ATH_MSG_DEBUG("Signature " << sig << " not supported, skipping chain " << chain);
                continue;
            }
            const size_t nOffline = (*read_handles[sig])->size();
            const size_t nRequired = static_cast<size_t>(ChainMultiplicity[readhandlesIndex]);
            ATH_MSG_DEBUG("Signature \"" << sig << "\": " << nOffline
                          << " offline objects, chain requires " << nRequired);
            // If a leg has fewer offline objects than required, no valid combination
            // can be formed. Skip to avoid out-of-range access in the generator loop.
            if (nOffline < nRequired) {
                tooFewOffline = true;
                break;
            }
            nucg.add({nOffline, nRequired});
        }
        if (tooFewOffline) {
            ATH_MSG_DEBUG("Not enough offline objects to form any combination for chain " << chain << ", skipping");
            continue;
        }

        // Loop over all possible combinations of offline physics objects
        do {
            const std::vector<size_t> combination = nucg();
            ++nucg;

            std::vector<const xAOD::IParticle *> particles;
            size_t location_in_combination = 0;

            for (size_t ChainNameIndex = 0; ChainNameIndex < ChainNameParseSignature.size(); ++ChainNameIndex) {
                const std::string& sig = ChainNameParseSignature[ChainNameIndex];
                if (read_handles.find(sig) == read_handles.end()) continue;

                for (size_t ChainMultipIndex = 0; ChainMultipIndex < static_cast<size_t>(ChainMultiplicity[ChainNameIndex]); ++ChainMultipIndex) {
                    const xAOD::IParticle* p = (*read_handles[sig])->at(combination[location_in_combination]);
                    ATH_MSG_VERBOSE("objectIndex --> " << sig << " " << combination[location_in_combination]
                                    << " pT: " << p->pt() << " eta: " << p->eta() << " phi: " << p->phi());
                    particles.push_back(p);
                    location_in_combination++;
                }
            }

            // Combination hygiene: legs sharing an offline container can draw
            // the SAME offline object into two slots (the per-leg generators
            // are independent). Such degenerate combinations are not physical
            // trigger combinations - the R2 reference never contains them -
            // while R3 can accept them through duplicated online copies (the
            // same muon reconstructed in overlapping RoIs appears several
            // times in the converted navigation). Skip them.
            bool duplicateOffline = false;
            for (size_t i = 0; i < particles.size() && !duplicateOffline; ++i) {
                for (size_t j = i + 1; j < particles.size(); ++j) {
                    if (particles[i] == particles[j]) {
                        duplicateOffline = true;
                        break;
                    }
                }
            }
            if (duplicateOffline) {
                ATH_MSG_VERBOSE("Skipping combination with a repeated offline object for chain " << chain);
                continue;
            }

            // R3: full combination matching via buildCombinations (DR-based,
            //     0.2 for tau chains, 0.1 otherwise).
            // R2: single full-combination match against the pre-stored TrigMatch
            //     container using pointer/shallow equality (configured on the
            //     MatchFromCompositeTool; its DR/rerun arguments are ignored).
            bool passR3 = m_matchingTool->match(particles, chain, drThreshold, false);
            bool passR2 = m_matchFromCompositeTool->match(particles, chain);

            anyPassR2 |= passR2;
            anyPassR3 |= passR3;

            // ERROR: R2 says pass but R3 doesn't - this indicates missing trigger information in R3
            if (passR2 && !passR3) {
                ATH_MSG_ERROR("R2 passes but R3 fails for chain " << chain
                              << " - R3 conversion may be missing trigger information");
                for (const auto& p : particles) {
                    bool r3 = m_matchingTool->match(*p, chain, drThreshold, false);
                    bool r2 = m_matchFromCompositeTool->match(*p, chain);
                    ATH_MSG_ERROR("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi()
                                  << " R2:" << r2 << " R3:" << r3);
                }
                isR2R3different = true;

                {
                    std::lock_guard<std::mutex> lock(m_failingChainsMutex);
                    m_failingChains[chain] += 1;
                }
            }
            // EXPECTED (but rare): R3 passes but R2 doesn't - R3 can form more combinations
            else if (!passR2 && passR3) {
                ATH_MSG_DEBUG("R3 passes but R2 fails (expected) for chain " << chain);
                if (msgLvl(MSG::DEBUG)) {
                    for (const auto& p : particles) {
                        bool r3 = m_matchingTool->match(*p, chain, drThreshold, false);
                        bool r2 = m_matchFromCompositeTool->match(*p, chain);
                        ATH_MSG_DEBUG("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi()
                                      << " R2:" << r2 << " R3:" << r3);
                    }
                }
            }
            // OK: both agree
            else {
                ++nCombinationsTested;
                if (msgLvl(MSG::DEBUG)) {
                    ATH_MSG_DEBUG("R2/R3 match for chain " << chain << " passR2: " << passR2 << " passR3: " << passR3);
                    for (const auto& p : particles) {
                        bool r3 = m_matchingTool->match(*p, chain, drThreshold, false);
                        bool r2 = m_matchFromCompositeTool->match(*p, chain);
                        ATH_MSG_DEBUG("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi()
                                      << " R2:" << r2 << " R3:" << r3);
                    }
                }
            }
        } while (nucg);

        } // end generic combination path

        // Chain-level sanity check: if R3 found at least one matching combination
        // then R2 should also have found at least one.
        if (anyPassR3 && !anyPassR2) {
            if (composites->empty()) {
                // The chain passed and R3 matched, but the derivation wrote NO
                // R2 reference entries at all. TrigMatch production reads the
                // original Run-2 navigation and is independent of the
                // conversion, so an empty reference cannot indicate a
                // conversion defect - the event is simply unverifiable by this
                // comparison. Verified on independent derivations of the same
                // events (e.g. HLT_g35_loose_g25_loose, HLT_e2x_..._mu8noL1).
                ATH_MSG_WARNING("Chain " << chain << ": chain passed and R3 matched, "
                                << "but the R2 TrigMatch reference is EMPTY - unverifiable event");
                std::lock_guard<std::mutex> lock(m_failingChainsMutex);
                m_emptyR2Chains[chain] += 1;
            }
            else {
                ATH_MSG_ERROR("Chain " << chain << ": R3 found at least one match but R2 found none "
                              << "- possible inconsistency between converted navigation and TrigMatch container");
                isR2R3different = true;
                // Count this in the finalize() summary as well. Chains that fail
                // only this chain-level check would otherwise be missing from
                // the "Failing chains count" deliverable of a full sweep. The two
                // increments cannot double-count within one event: the
                // per-combination one requires passR2, this one requires !anyPassR2.
                std::lock_guard<std::mutex> lock(m_failingChainsMutex);
                m_failingChains[chain] += 1;
            }
        }

        if (!isR2R3different && nCombinationsTested > 0) {
            ATH_MSG_INFO("Chain " << chain << ": R2/R3 agreement verified (" << nCombinationsTested
                         << (useLinearisedR2 ? " objects)" : " combinations)"));
        }

        // Dump TrigMatch container content at DEBUG level (composites already retrieved above)
        if (msgLvl(MSG::DEBUG)) {
            ATH_MSG_DEBUG("################################### TrigMatch container for " << chain << " (" << composites->size() << " combinations)");
            int comboIdx = 0;
            for (const xAOD::TrigComposite* combination : *composites) {
                //coverity[UNNECESSARY_STRING_COPY:FALSE]
                static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> accMatched("TrigMatchedObjects");
                const std::vector<ElementLink<xAOD::IParticleContainer>> featuresInCombination = accMatched(*combination);
                ATH_MSG_DEBUG("  Combo[" << comboIdx++ << "] (size=" << featuresInCombination.size() << "):");
                for (const ElementLink<xAOD::IParticleContainer>& f : featuresInCombination) {
                    if (f.isValid()) {
                        const xAOD::IParticle* iParticleR2 = resolveOriginal(*f);
                        ATH_MSG_DEBUG("    pt=" << iParticleR2->pt() << " eta=" << iParticleR2->eta() << " phi=" << iParticleR2->phi()
                                      << " [" << f.dataID() << ":" << f.index() << "]");
                    } else {
                        ATH_MSG_WARNING("    INVALID LINK in TrigMatch container for " << chain);
                    }
                }
            }
        }

        // Optional: print primary features and subfeatures per leg for inspection
        // (subfeatures are lower-pT objects from the Run2->Run3 conversion).
        if (m_printSubfeatures && msgLvl(MSG::DEBUG)) {
            const std::size_t nLegs = numberOfLegs(chain);
            if (nLegs == 0) {
                ATH_MSG_ERROR("Could not determine number of legs for chain " << chain);
                return StatusCode::FAILURE;
            }
            ATH_MSG_DEBUG("Subfeature inspection for " << chain);
            dumpFeaturesPerLeg(chain, nLegs, MSG::DEBUG);                 // primary features
            dumpFeaturesPerLeg(chain, nLegs, MSG::DEBUG, "subfeature");   // subfeatures
        }

        // Diagnostics dump on a failing chain
        if (isR2R3different) {
            const std::size_t nLegs = numberOfLegs(chain);
            if (nLegs == 0) {
                ATH_MSG_ERROR("Could not determine number of legs for chain " << chain);
                return StatusCode::FAILURE;
            }

            // Dump R2 pre-matched objects
            ATH_MSG_ERROR("R2 pre-matched objects for " << chain << ":");
            for (const xAOD::TrigComposite* combination : *composites) {
                static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> accMatched("TrigMatchedObjects");
                const std::vector<ElementLink<xAOD::IParticleContainer>> featuresInCombination = accMatched(*combination);
                for (const ElementLink<xAOD::IParticleContainer>& f : featuresInCombination) {
                    if (!f.isValid()) continue;
                    const xAOD::IParticle* iParticleR2 = resolveOriginal(*f);
                    ATH_MSG_ERROR("  R2 object: pT=" << iParticleR2->pt() << " eta=" << iParticleR2->eta() << " phi=" << iParticleR2->phi());
                }
            }

            // Dump R3 features and subfeatures per leg
            ATH_MSG_ERROR("R3 features per leg for " << chain << ":");
            dumpFeaturesPerLeg(chain, nLegs, MSG::ERROR);
            dumpFeaturesPerLeg(chain, nLegs, MSG::ERROR, "subfeature");
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode NavigationDAODTesterAlgv2::finalize() {
    ATH_MSG_INFO("Failing chains count: " << m_failingChains.size());

    std::vector<std::pair<std::string, int>> sortedChains(m_failingChains.begin(), m_failingChains.end());
    std::sort(sortedChains.begin(), sortedChains.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    for (const auto& chain : sortedChains) {
        ATH_MSG_INFO("Failing chain: " << chain.first << " count: " << chain.second);
    }

    // Chains with events that cannot be verified: the chain passed and the
    // converted navigation matched, but the R2 TrigMatch reference container
    // was empty (a derivation-side property, independent of the conversion).
    ATH_MSG_INFO("Chains with unverifiable events (empty R2 reference): " << m_emptyR2Chains.size());
    std::vector<std::pair<std::string, int>> sortedEmpty(m_emptyR2Chains.begin(), m_emptyR2Chains.end());
    std::sort(sortedEmpty.begin(), sortedEmpty.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    for (const auto& chain : sortedEmpty) {
        ATH_MSG_INFO("Unverifiable (empty R2) chain: " << chain.first << " count: " << chain.second);
    }

    return StatusCode::SUCCESS;
}

} // end namespace Trig
