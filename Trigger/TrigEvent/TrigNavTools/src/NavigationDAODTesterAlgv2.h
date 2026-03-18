/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIG_NAVTOOLS_NAVIGATIONDAODTESTERALGV2_H
#define TRIG_NAVTOOLS_NAVIGATIONDAODTESTERALGV2_H

/**
 * @file NavigationDAODTesterAlgv2.h
 * @brief Independent verification of trigger object matching on DAOD level.
 * 
 * This algorithm compares R2 (Run2 format) and R3 (Run3 format) trigger
 * matching results for offline physics objects to validate the correctness
 * of the Run2 to Run3 navigation conversion.
 */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TriggerMatchingTool/R3MatchingTool.h"
#include "TriggerMatchingTool/MatchFromCompositeTool.h"
#include "TriggerMatchingTool/IIParticleRetrievalTool.h"
#include "xAODBase/IParticleContainer.h"

#include "CxxUtils/checker_macros.h"

#include <vector>
#include <map>
#include <set>
#include <atomic>
#include <mutex>

namespace Trig {

class NavigationDAODTesterAlgv2 : public AthReentrantAlgorithm
{
public:
    NavigationDAODTesterAlgv2(const std::string &name, ISvcLocator *pSvcLocator);
    ~NavigationDAODTesterAlgv2() override = default;

    StatusCode initialize() override;
    StatusCode execute(const EventContext &context) const override;
    StatusCode finalize() override;

private:

    // Tool handles
    PublicToolHandle<Trig::TrigDecisionTool> m_tdt{
        this, "TrigDecisionTool", "", "Read R3 navigation data from TDT"};

    PublicToolHandle<Trig::R3MatchingTool> m_matchingTool{
        this, "R3MatchingTool", "Trig::R3MatchingTool", "R3 matching tool for converted navigation"};

    PublicToolHandle<Trig::MatchFromCompositeTool> m_matchFromCompositeTool{
        this, "MatchFromCompositeTool", "Trig::MatchFromCompositeTool", "R2 matching from pre-stored TrigComposite containers"};

    // Container keys - kept for interface compatibility
    SG::ReadHandleKey<xAOD::IParticleContainer> m_containerKey{
        this, "ContainerName", "Muons", "Default offline container name"};

    SG::ReadHandleKey<xAOD::TrigCompositeContainer> m_trigcompositecontainer{
        this, "TriggerName", "TriggerName", "Trigger container key"};

    // Properties
    Gaudi::Property<std::vector<std::string>> m_chains{
        this, "Chains", {}, "The trigger chains to test"};

    Gaudi::Property<bool> m_failOnDifference{
        this, "FailOnDifference", false,
        "Return FAILURE if the navigation does not compare equal"};

    Gaudi::Property<bool> m_verifyCombinationsSize{
        this, "VerifyCombinationsSize", true,
        "Check if combinations have matching size (that is Run2 >= Run3)"};

    Gaudi::Property<bool> m_verifyCombinations{
        this, "VerifyCombinationsContent", true,
        "Check if combinations are compatible (point to same objects)"};

    Gaudi::Property<std::string> m_inputPrefix{
        this, "InputPrefix", "TrigMatch_",
        "Prefix used for pre-matched trigger combinations branches"};

    Gaudi::Property<bool> m_printSubfeatures{
        this, "PrintSubfeatures", false,
        "When true, print subfeatures (lower-pT objects from Run2->Run3 conversion) for each passing chain"};

    // Mutable counters for statistics (thread-safe)
    mutable std::map<std::string, int> m_failingChains ATLAS_THREAD_SAFE;
    mutable std::mutex m_failingChainsMutex;

    mutable std::atomic<unsigned int> m_R3R2_ok_ele{0};
    mutable std::atomic<unsigned int> m_R3_greater_R2_ele{0};
    mutable std::atomic<unsigned int> m_R2_greater_R3_ele{0};

    mutable std::atomic<unsigned int> m_R3R2_ok_mu{0};
    mutable std::atomic<unsigned int> m_R3_greater_R2_mu{0};
    mutable std::atomic<unsigned int> m_R2_greater_R3_mu{0};
    
    mutable std::atomic<unsigned int> m_R3R2_ok_tau{0};
    mutable std::atomic<unsigned int> m_R3_greater_R2_tau{0};
    mutable std::atomic<unsigned int> m_R2_greater_R3_tau{0};

    mutable std::atomic<unsigned int> m_R3R2_ok{0};
    mutable std::atomic<unsigned int> m_R3_greater_R2{0};
    mutable std::atomic<unsigned int> m_R2_greater_R3{0};

    // Type definitions
    using CombinationsVector = std::vector<std::vector<const xAOD::IParticle *>>;

    // Helper methods
    bool combinationsEmpty(const CombinationsVector& combs) const;
    std::pair<int, const xAOD::IParticle*> matchR2ToR3(
        const xAOD::IParticle* iParticleR2, 
        const std::vector<std::set<const xAOD::IParticle*>>& featuresPerLegR3) const;

}; // end class NavigationDAODTesterAlgv2

} // end namespace Trig

#endif // TRIG_NAVTOOLS_NAVIGATIONDAODTESTERALGV2_H
