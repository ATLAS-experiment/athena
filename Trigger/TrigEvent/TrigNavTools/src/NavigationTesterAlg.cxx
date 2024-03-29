/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <set>
#include <algorithm>
#include <iterator>
#include "TrigCompositeUtils/ChainNameParser.h"
#include "NavigationTesterAlg.h"


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
            ATH_MSG_DEBUG("Begin testing chain " << chain);
            if (!m_tdt->isPassed(chain)) continue;

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
            for (auto& c : combsRun2 ) {
                ATH_MSG_DEBUG(c);
            }
            CombinationsVector vecCombinationsRun3;
            ATH_CHECK(m_toolRun3->retrieveParticles(vecCombinationsRun3, chain));
            auto combsRun3 = vectorToSet(vecCombinationsRun3);
            ATH_MSG_DEBUG("Run 3 size " << combsRun3.size());

            for (auto& c : combsRun3 ) {
                ATH_MSG_DEBUG(c);
            }
             
            if ( m_verifyCombinationsSize ) {
                auto status = verifyCombinationsSize(vecCombinationsRun2, vecCombinationsRun3, chain);
                if ( status.isFailure() and m_failOnDifference) {
                    ATH_MSG_ERROR("Failed when verifying combinations size");
                    return StatusCode::FAILURE;
                }

            } 
            if ( m_verifyCombinations ) {
                auto status = verifyCombinationsContent(combsRun2, combsRun3, chain);
                if ( status.isFailure() and m_failOnDifference) {
                    ATH_MSG_ERROR("Failed when verifying combinations content");
                    return StatusCode::FAILURE;
                }
            }
            ATH_MSG_DEBUG("Verified chain " << chain);
        }
        return StatusCode::SUCCESS;
    }
    StatusCode NavigationTesterAlg::verifyCombinationsSize(const CombinationsVector& run2, const CombinationsVector& run3, const std::string& chain) const {
        if (run2.size() > run3.size()) { // in Run3 we do not use decision per RoI but per object. For single RoI there is more than one object we will have more combinations in Run3
            ATH_MSG_WARNING("Issue in combination sizes for chain " << chain  
                        << " using Run 2 navigation " << run2.size() 
                        << " Run 3 navigation " << run3.size());
            if ( m_failOnDifference ) {
                ATH_MSG_ERROR("Mismatched sizes of combinations for chain " << chain << " (enable WARNING messages for more details)");    
                return StatusCode::FAILURE;
            }
        }
        return StatusCode::SUCCESS;
    }

    StatusCode NavigationTesterAlg::verifyCombinationsContent(const CombinationsSet& run2, const CombinationsSet& run3, const std::string& chain) const {
        // compare combinations
        bool isSubset = std::includes(run3.begin(), run3.end(), run2.begin(), run2.end());
        if (run2 != run3) 
        {
            ATH_MSG_WARNING("Difference in combinations between Run2 and Run3 format for chain: " << chain << " parsed multiplicities " << ChainNameParser::multiplicities(chain));
            ATH_MSG_WARNING("Run2 combs: " << run2);
            ATH_MSG_WARNING("Run3 combs: " << run3);
        }
        if (not isSubset) 
        {
            ATH_MSG_WARNING("NOT PASSED not isSubset failed, Run2 is not a subset of Run3 for chain: " << chain << " parsed multiplicities " << ChainNameParser::multiplicities(chain));
            ATH_MSG_WARNING("Run2 combs: " << run2);
            ATH_MSG_WARNING("Run3 combs: " << run3);
            return StatusCode::FAILURE;
        }

        for ( auto& combRun2: run2 ) {
            bool foundMatching = false;
            for ( auto& combRun3 : run3 ) {     
                ATH_MSG_DEBUG("Available Run 2 combinations: " );
                for ( auto& c: combRun2 ){
                    ATH_MSG_DEBUG("  " << c );
                }
                ATH_MSG_DEBUG("Available Run 3 combinations: " );
                for ( auto& c: combRun3 ){
                    ATH_MSG_DEBUG("  " << c );
                }
                ATH_MSG_DEBUG("COMPARISON combRun2 == combRun3 are " <<  ( combRun2 == combRun3  ? "identical" : "distinct"));
                if ( combRun2 == combRun3 ) {
                    ATH_MSG_DEBUG("Found matching combinations, run2 " << combRun2 
                                << " run3 " << combRun3 );
                    foundMatching = true;
                    break;
                } 
            }
            if ( not foundMatching ) {
                ATH_MSG_WARNING("Specific combination for chain " << chain << " can not be found in Run 3");
                ATH_MSG_WARNING("Run 2 combination: " << combRun2 );
                ATH_MSG_WARNING("Available Run 3 combinations: " );
                for ( auto& c: run3 ){
                    ATH_MSG_WARNING("  " << c );
                }
                if ( m_failOnDifference ) {
                    ATH_MSG_ERROR("When checking combinations in details found differences, (enable WARNING message for more details)");
                    return StatusCode::FAILURE;
                }
            }
        }
        return StatusCode::SUCCESS;
    }

} //> end namespace Trig


