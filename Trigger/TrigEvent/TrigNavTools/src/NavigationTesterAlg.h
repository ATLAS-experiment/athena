/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "TriggerMatchingTool/IIParticleRetrievalTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

#include <vector>

namespace Trig {
    class NavigationTesterAlg : public AthReentrantAlgorithm
    {
    public:
        NavigationTesterAlg(const std::string &name, ISvcLocator *pSvcLocator);
        ~NavigationTesterAlg() override = default;

        StatusCode initialize() override;
        StatusCode execute(const EventContext &context) const override;

    private:
        PublicToolHandle<Trig::TrigDecisionTool> m_tdt{this, "TrigDecisionTool", "", "When enabled read navigation from TDT/off by default"};
        PublicToolHandle<Trig::TrigDecisionTool> m_tdtRun3{this, "TrigDecisionToolRun3", "", "Use for directly accessing objects"};
        PublicToolHandle<Trig::TrigDecisionTool> m_tdtRun2{this, "TrigDecisionToolRun2", "", "Use for directly accessing objects"};

        ToolHandle<Trig::IIParticleRetrievalTool> m_toolRun2{
            this, "RetrievalToolRun2Nav", "", "The tool configured to use Run 2 format"};
        ToolHandle<Trig::IIParticleRetrievalTool> m_toolRun3{
            this, "RetrievalToolRun3Nav", "", "The tool configured to use Run 3 format"};
        Gaudi::Property<std::vector<std::string>> m_chains{
            this, "Chains", {}, "The chains to test"};
        Gaudi::Property<bool> m_failOnDifference{
            this, "FailOnDifference", false,
            "Return FAILURE if the navigation does not compare equal"};
        Gaudi::Property<bool> m_verifyCombinationsSize{
            this, "VerifyCombinationsSize", true,
            "Check if combinations have matching size (that is Run2 >= Run3)"};
        Gaudi::Property<bool> m_verifyCombinations{
            this, "VerifyCombinationsContent", true,
            "Check if combinations are compatible (point to same objects)"};

        using CombinationsVector=std::vector<std::vector<const xAOD::IParticle *>>;
        using CombinationsSet=std::set<std::set<const xAOD::IParticle *>>;
        StatusCode verifyCombinationsSize(const CombinationsVector& run2, const CombinationsVector& run3, const std::string& chain) const;
        StatusCode verifyCombinationsContent(const CombinationsSet& run2, const CombinationsSet& run3, const std::string& chain) const;
        bool combinationsEmpty(const CombinationsVector& combs) const;

    }; //> end class AthAlgorithm
}
