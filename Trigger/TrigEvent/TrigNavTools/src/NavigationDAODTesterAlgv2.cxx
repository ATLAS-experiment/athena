/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "NavigationDAODTesterAlgv2.h"

#include <GaudiKernel/StatusCode.h>
#include <set>
#include <algorithm>
#include <iterator>
#include <numeric>

#include "TrigCompositeUtils/ChainNameParser.h"
#include "TrigDecisionTool/FeatureContainer.h"
#include "TrigDecisionTool/Conditions.h"
#include "TrigAnalysisHelpers/FeatureRequestDescriptor.h"
#include "TrigCompositeUtils/Combinators.h"
#include "TrigDecisionTool/ExpertMethods.h"
#include "xAODTrigger/TrigCompositeContainer.h"

namespace TCU = TrigCompositeUtils;

namespace Trig {

NavigationDAODTesterAlgv2::NavigationDAODTesterAlgv2(const std::string &name, ISvcLocator *pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode NavigationDAODTesterAlgv2::initialize() {
    ATH_CHECK(m_containerKey.initialize());
    ATH_CHECK(m_trigcompositecontainer.initialize());
    ATH_CHECK(m_tdt.retrieve());

    ATH_CHECK(m_matchingTool.retrieve());
    ATH_CHECK(m_matchFromCompositeTool.retrieve());

    if (m_chains.empty()) {
        ATH_MSG_WARNING("No chains provided, algorithm will be no-op");
    }

    // Initialize counters
    m_R3R2_ok_ele = 0;
    m_R3_greater_R2_ele = 0;
    m_R2_greater_R3_ele = 0;

    m_R3R2_ok_mu = 0;
    m_R3_greater_R2_mu = 0;
    m_R2_greater_R3_mu = 0;

    m_R3R2_ok_tau = 0;
    m_R3_greater_R2_tau = 0;
    m_R2_greater_R3_tau = 0;

    m_R3R2_ok = 0;
    m_R3_greater_R2 = 0;
    m_R2_greater_R3 = 0;

    return StatusCode::SUCCESS;
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

        // Get number of legs in chain using ChainNameParser
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
        ATH_MSG_DEBUG("Passing " << chain << " " << m_tdt->isPassed(chain));

        // Setup combination generator based on multiplicities of offline physics objects for each leg
        HLT::NestedUniqueCombinationGenerator nucg;
        ATH_MSG_DEBUG("NestedUniqueCombinationGenerator: " << ChainMultiplicity.size() << " legs, " << ChainNameParseSignature.size() << " signatures");
        
        for (size_t readhandlesIndex = 0; readhandlesIndex < ChainNameParseSignature.size(); readhandlesIndex++) {
            const std::string& sig = ChainNameParseSignature[readhandlesIndex];
            if (read_handles.find(sig) == read_handles.end()) {
                ATH_MSG_DEBUG("Signature " << sig << " not supported, skipping chain " << chain);
                continue;
            }
            ATH_MSG_DEBUG("Signature \"" << sig << "\": " << (*read_handles[sig])->size()
                          << " offline objects, chain requires " << ChainMultiplicity[readhandlesIndex]);
            nucg.add({(*read_handles[sig])->size(), static_cast<size_t>(ChainMultiplicity[readhandlesIndex])});
        }

        bool isR2R3different = false;
        size_t nCombinationsTested = 0;

        // Retrieve TrigMatch container early to determine matching strategy
        // TrigMatch combo size varies by chain:
        //   - size=1 entries (e.g., HLT_2mu10_nomucomb): stores individual matched particles
        //   - size=N entries (e.g., HLT_e17_lhloose_nod0_2e9_lhloose_nod0): stores full N-particle combos
        std::string containerName = m_inputPrefix + chain;
        std::replace(containerName.begin(), containerName.end(), '.', '_');
        const xAOD::TrigCompositeContainer* composites(nullptr);
        bool hasTrigMatch = evtStore()->retrieve(composites, containerName).isSuccess();
        if (!hasTrigMatch) {
            ATH_MSG_DEBUG("No branch in this DAOD for " << chain);
            continue;
        }

        size_t trigMatchComboSize = 0;
        if (!composites->empty()) {
            static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> accMatchedSize("TrigMatchedObjects");
            trigMatchComboSize = accMatchedSize(*composites->at(0)).size();
        }
        const bool usePerParticleR2 = (trigMatchComboSize <= 1);
        ATH_MSG_DEBUG("TrigMatch combo size=" << trigMatchComboSize
                      << " → using " << (usePerParticleR2 ? "per-particle" : "vector-based") << " R2 matching");

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

            // Compare R3 (converted navigation) vs R2 (pre-stored composites)
            // R3 matching uses full combination matching via buildCombinations.
            // R2 matching strategy depends on TrigMatch combo size:
            //   - size=1: per-particle check (each particle individually in TrigMatch)
            //   - size>1: vector-based check (full combination must exist in TrigMatch)
            bool passR3 = m_matchingTool->match(particles, chain, 0.1, false);
            bool passR2;
            if (usePerParticleR2) {
                // TrigMatch stores individual particles (e.g., HLT_2mu10_nomucomb)
                passR2 = true;
                for (const auto* p : particles) {
                    if (!m_matchFromCompositeTool->match(*p, chain, 0.1, false)) {
                        passR2 = false;
                        break;
                    }
                }
            } else {
                // TrigMatch stores full N-particle combinations (e.g., HLT_e17_lhloose_nod0_2e9_lhloose_nod0)
                passR2 = m_matchFromCompositeTool->match(particles, chain, 0.1, false);
            }

            // ERROR: R2 says pass but R3 doesn't - this indicates missing trigger information in R3
            if (passR2 && !passR3) {
                ATH_MSG_ERROR("R2 passes but R3 fails for chain " << chain
                              << " — R3 conversion may be missing trigger information");
                for (const auto& p : particles) {
                    bool r3 = m_matchingTool->match(*p, chain, 0.1, false);
                    bool r2 = m_matchFromCompositeTool->match(*p, chain, 0.1, false);
                    ATH_MSG_ERROR("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi()
                                  << " R2:" << r2 << " R3:" << r3);
                }
                isR2R3different = true;
                
                {
                    std::lock_guard<std::mutex> lock(m_failingChainsMutex);
                    if (m_failingChains.find(chain) != m_failingChains.end()) {
                        m_failingChains[chain] = m_failingChains[chain] + 1;
                    } else {
                        m_failingChains[chain] = 1;
                    }
                }
            }
            // EXPECTED: R3 passes but R2 doesn't - R3 has more combinations (by design)
            else if (!passR2 && passR3) {
                // Expected: R3 has more combinations due to architectural differences
                ATH_MSG_DEBUG("R3 passes but R2 fails (expected) for chain " << chain);
                for (const auto& p : particles) {
                    bool r3 = m_matchingTool->match(*p, chain, 0.1, false);
                    bool r2 = m_matchFromCompositeTool->match(*p, chain, 0.1, false);
                    ATH_MSG_DEBUG("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi()
                                  << " R2:" << r2 << " R3:" << r3);
                }
            }
            // OK: Both agree
            else {
                ++nCombinationsTested;
                ATH_MSG_DEBUG("R2/R3 match for chain " << chain << " passR2: " << passR2 << " passR3: " << passR3);
                for (const auto& p : particles) {
                    bool r3 = m_matchingTool->match(*p, chain, 0.1, false);
                    bool r2 = m_matchFromCompositeTool->match(*p, chain, 0.1, false);
                    ATH_MSG_DEBUG("  Particle pT=" << p->pt() << " eta=" << p->eta() << " phi=" << p->phi() << " R2:" << r2 << " R3:" << r3);
                }
            }
        } while (nucg);

        if (!isR2R3different && nCombinationsTested > 0) {
            ATH_MSG_INFO("Chain " << chain << ": R2/R3 agreement verified (" << nCombinationsTested << " combinations)");
        }

        // Dump TrigMatch container content at DEBUG level (composites already retrieved above)
        ATH_MSG_DEBUG("TrigMatch container for " << chain << " (" << composites->size() << " combinations)");
        int comboIdx = 0;
        for (const xAOD::TrigComposite* combination : *composites) {
            static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> accMatched("TrigMatchedObjects");
            const std::vector<ElementLink<xAOD::IParticleContainer>> featuresInCombination = accMatched(*combination);
            ATH_MSG_DEBUG("  Combo[" << comboIdx++ << "] (size=" << featuresInCombination.size() << "):");
            for (const ElementLink<xAOD::IParticleContainer>& f : featuresInCombination) {
                if (f.isValid()) {
                    const xAOD::IParticle* iParticleR2 = *f;
                    ATH_MSG_DEBUG("    pt=" << iParticleR2->pt() << " eta=" << iParticleR2->eta() << " phi=" << iParticleR2->phi()
                                  << " [" << f.dataID() << ":" << f.index() << "]");
                } else {
                    ATH_MSG_WARNING("    INVALID LINK in TrigMatch container for " << chain);
                }
            }
        }

        // Print subfeatures if enabled - these are lower-pT objects from Run2->Run3 conversion
        // that were in the same RoI but not selected as the primary "feature" (highest-pT)
        if (m_printSubfeatures) {
            ATH_MSG_DEBUG("Subfeature inspection for " << chain);

            // Get number of legs for per-leg subfeature display
            const TrigConf::HLTChain* hltChain = m_tdt->ExperimentalAndExpertMethods().getChainConfigurationDetails(chain);
            std::size_t nLegs = 0;
            if (hltChain) {
                nLegs = hltChain->leg_multiplicities().size();
            }
            if (nLegs == 0) {
                nLegs = std::accumulate(ChainMultiplicity.begin(), ChainMultiplicity.end(), static_cast<size_t>(0));
            }

            // Print features per leg for reference
            for (std::size_t leg = 0; leg < nLegs; ++leg) {
                Trig::FeatureRequestDescriptor frd;
                frd.setChainGroup(chain);
                frd.setRestrictRequestToLeg(static_cast<int>(leg));
                frd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
                auto features = m_tdt->features<xAOD::IParticleContainer>(frd);
                ATH_MSG_DEBUG("  Leg " << leg << " features (" << features.size() << "):");
                for (const auto& feature : features) {
                    if (feature.isValid()) {
                        ATH_MSG_DEBUG("    [feature] pt=" << (*feature.link)->pt()
                                      << " eta=" << (*feature.link)->eta()
                                      << " phi=" << (*feature.link)->phi()
                                      << " [" << feature.link.dataID() << ":" << feature.link.index() << "]");
                    }
                }
            }

            // Print subfeatures per leg
            for (std::size_t leg = 0; leg < nLegs; ++leg) {
                Trig::FeatureRequestDescriptor subFrd;
                subFrd.setChainGroup(chain);
                subFrd.setRestrictRequestToLeg(static_cast<int>(leg));
                subFrd.setLinkName("subfeature");
                subFrd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
                auto subfeatures = m_tdt->features<xAOD::IParticleContainer>(subFrd);
                ATH_MSG_DEBUG("  Leg " << leg << " subfeatures (" << subfeatures.size() << "):");
                for (const auto& sf : subfeatures) {
                    if (sf.isValid()) {
                        ATH_MSG_DEBUG("    [subfeature] pt=" << (*sf.link)->pt()
                                      << " eta=" << (*sf.link)->eta()
                                      << " phi=" << (*sf.link)->phi()
                                      << " [" << sf.link.dataID() << ":" << sf.link.index() << "]");
                    }
                }
            }

            // Total subfeatures without leg restriction
            Trig::FeatureRequestDescriptor allSubFrd;
            allSubFrd.setChainGroup(chain);
            allSubFrd.setLinkName("subfeature");
            allSubFrd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
            auto allSubfeatures = m_tdt->features<xAOD::IParticleContainer>(allSubFrd);
            ATH_MSG_DEBUG("  Total subfeatures (all legs): " << allSubfeatures.size());
        }

        if (isR2R3different) {
            // Dump R2 pre-matched objects for the failing chain
            ATH_MSG_ERROR("R2 pre-matched objects for " << chain << ":");
            for (const xAOD::TrigComposite* combination : *composites) {
                static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> accMatched("TrigMatchedObjects");
                const std::vector<ElementLink<xAOD::IParticleContainer>> featuresInCombination = accMatched(*combination);
                for (const ElementLink<xAOD::IParticleContainer>& f : featuresInCombination) {
                    if (!f.isValid()) continue;
                    const xAOD::IParticle* iParticleR2 = *f;
                    ATH_MSG_ERROR("  R2 object: pT=" << iParticleR2->pt() << " eta=" << iParticleR2->eta() << " phi=" << iParticleR2->phi());
                }
            }

            // Dump R3 features per leg
            const TrigConf::HLTChain* hltChain = m_tdt->ExperimentalAndExpertMethods().getChainConfigurationDetails(chain);
            std::size_t nLegs = 0;
            if (hltChain) {
                nLegs = hltChain->leg_multiplicities().size();
            }
            if (nLegs == 0) {
                nLegs = std::accumulate(ChainMultiplicity.begin(), ChainMultiplicity.end(), 0);
            }

            ATH_MSG_ERROR("R3 features per leg for " << chain << ":");
            for (std::size_t leg = 0; leg < nLegs; ++leg) {
                Trig::FeatureRequestDescriptor frd;
                frd.setChainGroup(chain);
                frd.setRestrictRequestToLeg(static_cast<int>(leg));
                frd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
                auto features = m_tdt->features<xAOD::IParticleContainer>(frd);
                for (const auto& feature : features) {
                    ATH_MSG_ERROR("  Leg " << leg << ": pT=" << (*feature.link)->pt()
                                  << " eta=" << (*feature.link)->eta()
                                  << " phi=" << (*feature.link)->phi()
                                  << " [" << feature.link.dataID() << ":" << feature.link.index() << "]");
                }
            }

            // Check for subfeatures
            Trig::FeatureRequestDescriptor subFrd;
            subFrd.setChainGroup(chain);
            subFrd.setLinkName("subfeature");
            subFrd.setFeatureCollectionMode(TrigDefs::allFeaturesOfType);
            auto subfeatures = m_tdt->features<xAOD::IParticleContainer>(subFrd);
            ATH_MSG_ERROR("Subfeatures for " << chain << ": " << subfeatures.size() << " found");
            for (const auto& sf : subfeatures) {
                if (sf.isValid()) {
                    ATH_MSG_ERROR("  [subfeature] pT=" << (*sf.link)->pt()
                                  << " eta=" << (*sf.link)->eta()
                                  << " phi=" << (*sf.link)->phi()
                                  << " [" << sf.link.dataID() << ":" << sf.link.index() << "]");
                }
            }
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode NavigationDAODTesterAlgv2::finalize() {
    ATH_MSG_ALWAYS("Failing chains count: " << m_failingChains.size());
    
    std::vector<std::pair<std::string, int>> sortedChains(m_failingChains.begin(), m_failingChains.end());
    std::sort(sortedChains.begin(), sortedChains.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    for (const auto& chain : sortedChains) {
        ATH_MSG_ALWAYS("Failing chain: " << chain.first << " count: " << chain.second);
    }

    return StatusCode::SUCCESS;
}

std::pair<int, const xAOD::IParticle*> NavigationDAODTesterAlgv2::matchR2ToR3(
    const xAOD::IParticle* iParticleR2, 
    const std::vector<std::set<const xAOD::IParticle*>>& featuresPerLegR3) const 
{
    for (unsigned leg = 0; leg < featuresPerLegR3.size(); ++leg) {
        for (const xAOD::IParticle* r3 : featuresPerLegR3.at(leg)) {
            if (iParticleR2->p4().DeltaR(r3->p4()) < 0.1) {
                return std::make_pair(leg, r3);
            }
        }
    }
    return std::make_pair(-1, nullptr);
}

bool NavigationDAODTesterAlgv2::combinationsEmpty(const CombinationsVector& combs) const {
    size_t counter = 0;
    for (const std::vector<const xAOD::IParticle*>& outerc : combs) {
        counter += outerc.size();
    }
    return counter == 0;
}

} // end namespace Trig
