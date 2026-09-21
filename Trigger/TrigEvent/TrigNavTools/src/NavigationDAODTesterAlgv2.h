/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include <string>
#include <mutex>


namespace Trig {

class NavigationDAODTesterAlgv2 : public AthReentrantAlgorithm
{
public:
    NavigationDAODTesterAlgv2(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~NavigationDAODTesterAlgv2() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &context) const override;
    virtual StatusCode finalize() override;

private:

    // Tool handles
    PublicToolHandle<Trig::TrigDecisionTool> m_tdt{
        this, "TrigDecisionTool", "", "Read R3 navigation data from TDT"};

    PublicToolHandle<Trig::R3MatchingTool> m_matchingTool{
        this, "R3MatchingTool", "Trig::R3MatchingTool", "R3 matching tool for converted navigation"};

    PublicToolHandle<Trig::MatchFromCompositeTool> m_matchFromCompositeTool{
        this, "MatchFromCompositeTool", "Trig::MatchFromCompositeTool", "R2 matching from pre-stored TrigComposite containers"};

    // Properties
    Gaudi::Property<std::vector<std::string>> m_chains{
        this, "Chains", {}, "The trigger chains to test"};

    Gaudi::Property<std::vector<std::string>> m_chainsToExclude{
        this, "ChainsToExclude", {},
        "Chain name patterns to skip (substring match), e.g. 'noL1'. "
        "DAOD-tester-local exception list for pathological chain families whose "
        "TrigMatch layout does not support the generic R2/R3 comparison. "
        "Note: nomucomb chains no longer need excluding - they are handled by "
        "the dedicated linearised path (see ChainsWithLinearisedR2Matching)."};

    Gaudi::Property<std::vector<std::string>> m_linearisedR2Chains{
        this, "ChainsWithLinearisedR2Matching", {"nomucomb"},
        "Chain name patterns (substring match) whose TrigMatch container uses "
        "the linearised layout: N entries of vector size 1, one per "
        "individually matched offline object, instead of one entry of size "
        "= sum of leg multiplicities per matched combination. These chains "
        "are compared PER OBJECT (pool of individually matched objects vs "
        "single-object R3 matching) instead of per combination, since the "
        "reference carries no combination structure."};


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

    Gaudi::Property<bool> m_dumpFeaturesWithoutR2{
        this, "DumpFeaturesWithoutR2", false,
        "Diagnostic: when the TrigMatch_<chain> container is absent (e.g. when "
        "running directly on the conversion output at AOD level, where no R2 "
        "matching reference exists), still print the per-leg R3 features of "
        "passed chains. The cross-leg feature pooling converter defect shows "
        "up directly in this dump (identical pooled lists on every leg)."};

    // Mutable counters for statistics (thread-safe)
    mutable std::map<std::string, int> m_failingChains ATLAS_THREAD_SAFE;
    /// Events where the chain passed and R3 matched, but the R2 TrigMatch
    /// reference container is EMPTY. TrigMatch production is independent of
    /// the navigation conversion, so such events cannot indicate a conversion
    /// defect - there is nothing to compare against. Counted separately.
    mutable std::map<std::string, int> m_emptyR2Chains ATLAS_THREAD_SAFE;
    mutable std::mutex m_failingChainsMutex;

    // Helper methods

    /// Number of legs of a chain, derived from ChainNameParser::multiplicities().
    /// Returns 0 if the multiplicities could not be determined (caller should
    /// treat this as an error).
    std::size_t numberOfLegs(const std::string& chain) const;

    /// Dump the per-leg R3 features of a chain at the given message level.
    /// Used both for diagnostics on failing chains and for the optional
    /// subfeature printout. The linkName selects "feature" (default) or
    /// "subfeature".
    void dumpFeaturesPerLeg(const std::string& chain,
                            std::size_t nLegs,
                            MSG::Level level,
                            const std::string& linkName = "") const;

    /// True if the chain name contains any of the patterns (substring match).
    static bool matchesAnyPattern(const std::string& chain,
                                  const std::vector<std::string>& patterns);

    /// Resolve a possible shallow copy to its original object via the
    /// "originalObjectLink" decoration (mirrors the MatchShallow behaviour
    /// of MatchFromCompositeTool::areTheSame, plus a defensive validity
    /// check on the link).
    static const xAOD::IParticle* resolveOriginal(const xAOD::IParticle* p);

    /// R2 verdict for chains with linearised TrigMatch layout (e.g. nomucomb):
    /// true if every given particle consumes a distinct object from the pool
    /// of individually matched offline objects (the pool is taken by value and
    /// consumed during the test). Used with a single particle in the
    /// per-object comparison path.
    static bool matchLinearisedR2(std::vector<const xAOD::IParticle*> pool,
                                  const std::vector<const xAOD::IParticle*>& particles);

}; // end class NavigationDAODTesterAlgv2


} // end namespace Trig

#endif // TRIG_NAVTOOLS_NAVIGATIONDAODTESTERALGV2_H
