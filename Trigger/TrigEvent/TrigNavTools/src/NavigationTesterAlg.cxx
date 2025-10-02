/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <GaudiKernel/StatusCode.h>
#include <set>
#include <algorithm>
#include <iterator>
#include "TrigCompositeUtils/ChainNameParser.h"
#include "NavigationTesterAlg.h"
#include "SpecialCases.h"


// anonymous namespace for convenience functions
namespace {
    std::set<std::set<const xAOD::IParticle *>> vectorToSet(
            const std::vector<std::vector<const xAOD::IParticle *>>& vec)
    {
        std::set<std::set<const xAOD::IParticle *>> ret;
        for (const std::vector<const xAOD::IParticle *> &combination : vec)
            ret.emplace(combination.begin(), combination.end());
        return ret;
    }

}

namespace xAOD {
    std::ostream &operator<<(std::ostream &os, const xAOD::IParticle *p)
    {
        return os << "["
            << "type = " << p->type() << ", "
            << "pt = " << p->pt() << ", "
            << "eta = " << p->eta() << ", "
            << "phi = " << p->phi() << ", "
            << "ptr = " << reinterpret_cast<const void*>(p) 
            << "]";

    }
}

namespace std {
    // Define printing operators for the set and IParticle pointers
    template <typename T>
    std::ostream &operator<<(std::ostream &os, const std::set<T> &s)
    {
        os << "{";
        for (auto itr = s.begin(); itr != s.end(); ++itr)
        {
            if (itr != s.begin())
                os << ", ";
            os << *itr;
        }
        return os << "}";
    }
}

namespace Trig {

    NavigationTesterAlg::NavigationTesterAlg(const std::string &name, ISvcLocator *pSvcLocator) :
        AthReentrantAlgorithm(name, pSvcLocator)
    {}

    StatusCode NavigationTesterAlg::initialize()
    {
        ATH_CHECK(m_tdt.retrieve());
        ATH_CHECK(m_tdtRun2.retrieve());
        ATH_CHECK(m_tdtRun3.retrieve());
        ATH_CHECK(m_toolRun2.retrieve());
        ATH_CHECK(m_toolRun3.retrieve());

        if (m_chains.size() == 0)
            ATH_MSG_WARNING("No chains provided, algorithm will be no-op");
        return StatusCode::SUCCESS;
    }

    StatusCode NavigationTesterAlg::execute(const EventContext &) const
    {
        for (const std::string &chain : m_chains)
        {
            ATH_MSG_DEBUG("Begin testing chain " << chain << (m_tdt->isPassed(chain) ? " and will dive into details as the chain passed " : " but will not do anything as the chain did not pass"));
            if (!m_tdt->isPassed(chain)) continue;

            // explicitely excluded chains
            bool isExcluded = false;
            for (const auto& excludedChain : SpecialCases::excludedChains) {
                if (chain == excludedChain) {
                    isExcluded = true;
                    break; // Break out of the exclusion check loop
                }
            }

            if (isExcluded) {
                continue; // Skip the current iteration of the main loop
            }
            
            // We assume that the navigation is ultimately a set of element links
            // We're comparing two types of navigation but they should both point to the same
            // objects.
            // We don't care about the order of the combinations, or the order within the
            // combinations, we just care that they are the same. Therefore, we can convert the
            // vectors to sets and just look at the differences between them
            CombinationsVector vecCombinationsRun2;
            ATH_MSG_DEBUG("###### checking features of CHAIN " << chain);
            ATH_CHECK(m_toolRun2->retrieveParticles(vecCombinationsRun2, chain));
            auto combsRun2 = vectorToSet(vecCombinationsRun2);
            ATH_MSG_DEBUG("Run 2 size " << combsRun2.size());
            // if Run 2 size is 0 we discard any further testing
            if (combsRun2.size() == 0)
            {
                ATH_MSG_DEBUG("Chain " << chain << " testing discarded due to detected Run 2 size == 0");
                continue;
            }
            for (auto& c : combsRun2 ) {
                ATH_MSG_DEBUG(c);
            }
            CombinationsVector vecCombinationsRun3;
            ATH_CHECK(m_toolRun3->retrieveParticles(vecCombinationsRun3, chain));
            auto combsRun3 = vectorToSet(vecCombinationsRun3);
            ATH_MSG_DEBUG("Run 3 size " << combsRun3.size());

            if (combinationsEmpty(vecCombinationsRun2) and combinationsEmpty(vecCombinationsRun3)) {
                ATH_MSG_DEBUG("Both, Run2 and Run3 combinations are effectively empty");
                continue;
            }


            for (auto& c : combsRun3 ) {
                ATH_MSG_DEBUG(c);
            }
            if ( std::regex_match(chain, SpecialCases::gammaXeChain) ) {  
                ATH_CHECK(verifyFlatContent(chain));
            } else {
                if ( m_verifyCombinationsSize ) {
                    ATH_CHECK(verifyCombinationsSize(vecCombinationsRun2, vecCombinationsRun3, chain));
                } 
                if ( m_verifyCombinations ) {
                    ATH_CHECK(verifyCombinationsContent(combsRun2, combsRun3, chain));
                }
            }

            ATH_MSG_DEBUG("Verified chain " << chain);
        }
        return StatusCode::SUCCESS;
    }

    StatusCode NavigationTesterAlg::verifyFlatContent(const std::string& chain) const {
        const auto &run3 = m_tdtRun3->features<xAOD::IParticleContainer>(chain);
        std::set<const xAOD::IParticle*> particlesRun3;
        for ( auto l: run3) {
            if (  l.link.isValid() )
                particlesRun3.insert(*(l.link));
        }

        CombinationsVector vecCombinationsRun2;
        ATH_CHECK(m_toolRun2->retrieveParticles(vecCombinationsRun2, chain));
        std::set<const xAOD::IParticle*> particlesRun2;
        for ( auto& comb: vecCombinationsRun2) {
            for ( auto el: comb) {
                particlesRun2.insert(el);
            }
        }

        for ( auto f2: particlesRun2 ) {
            bool found=false;
            for ( auto f3: particlesRun3 ) {
                ATH_MSG_DEBUG("Serial set of features " << f3 );
                if ( f2 == f3)
                    found = true;
            }
            if ( not found ) {
                ATH_MSG_ERROR("Missing feature in Run 3 that is present in Run 2 " <<  f2 << " chain " << chain << " enable DEBUG to see more details" );
                if ( m_failOnDifference ) {
                    return StatusCode::FAILURE;
                } 
            }
        }
        return StatusCode::SUCCESS;
    }


    StatusCode NavigationTesterAlg::verifyCombinationsSize(const CombinationsVector& run2, const CombinationsVector& run3, const std::string& chain) const {
        if (run2.size() > run3.size()) { // in Run3 we do not use decision per RoI but per object. For single RoI there is more than one object we will have more combinations in Run3
            ATH_MSG_WARNING("Issue in combination sizes for chain " << chain  
                        << " using Run 2 navigation " << run2.size() 
                        << " Run 3 navigation " << run3.size());
            ATH_MSG_WARNING("Mismatched sizes of combinations for chain " << chain << " (enable WARNING messages for more details), this may be a false positive if chain is incorrectly decoded");    
            if ( m_failOnDifference ) {
                return StatusCode::FAILURE;
            }
        }
        return StatusCode::SUCCESS;
    }

    StatusCode NavigationTesterAlg::verifyCombinationsContent(const CombinationsSet& run2, const CombinationsSet& run3, const std::string& chain) const {
        // compare combinations

        using xAODParticle = const xAOD::IParticle;

        auto isSubsetPresent = [](const std::set<xAODParticle*>& subset, const CombinationsSet& run2) {
            for (const auto& setInRun2 : run2) {
                // Manual check for all particles in subset
                bool allFound = true;
                for (auto particle : subset) {
                    if (setInRun2.find(particle) == setInRun2.end()) {
                        allFound = false;
                        break; // If any particle is not found, no need to check further
                    }
                }
                if (allFound) return true; // Found all particles in this subset of Run2
            }
            return false; // Did not find the subset
        };


        auto isAnySubsetPresent = [&isSubsetPresent](const CombinationsSet& run3, const CombinationsSet& run2) {
            for (const auto& subset : run3) {
                if (isSubsetPresent(subset, run2)) {
                    return true; // At least one subset from Run3 is found in Run2
                }
            }
            return false; // No subset from Run3 was found in Run2
        };
        
        bool result { false };
        // hack for "HLT_e26_lhmedium_nod0_mu8noL1" case
        // hack for "HLT_e24_lhmedium_L1EM20VHI_mu8noL1" type case
        // hack for "HLT_mu11_2mu4noL1_nscan03_L1MU11_2MU6" type case
        if ( std::regex_match(chain, SpecialCases::specialEchain) or
             std::regex_match(chain, SpecialCases::specialElMuChain) or
             std::regex_match(chain, SpecialCases::mu2MunoL1Special) ) {  
            result = isAnySubsetPresent(run3, run2);
        } else {
            // now subset checked on a level of objects, instead of group of objects
            result = isAnySubsetPresent(run2, run3);
        }

        if (run2 != run3) 
        {
            ATH_MSG_WARNING("Difference in combinations between Run2 and Run3 format for chain: " << chain << " parsed multiplicities " << ChainNameParser::multiplicities(chain));
            ATH_MSG_WARNING("Run2 combs: " << run2);
            ATH_MSG_WARNING("Run3 combs: " << run3);
        }

        if (not result) // previous not isSubset, loosened condition
        {
            ATH_MSG_WARNING("NOT PASSED: failed, Run2 objects are not within a subset of Run3 objects for chain: " << chain << " parsed multiplicities " << ChainNameParser::multiplicities(chain));
            ATH_MSG_WARNING("Run2 combs: " << run2);
            ATH_MSG_WARNING("Run3 combs: " << run3);
            if ( m_failOnDifference ) {
                return StatusCode::FAILURE;
            }
        }

        return StatusCode::SUCCESS;
    }

    bool NavigationTesterAlg::combinationsEmpty(const CombinationsVector& combs) const {
        size_t counter = 0;
        for ( const std::vector<const xAOD::IParticle*>& outerc: combs ) 
            counter += outerc.size();
        return counter == 0;
    }

} //> end namespace Trig


