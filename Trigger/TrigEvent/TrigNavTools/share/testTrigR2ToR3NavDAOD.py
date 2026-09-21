#!/usr/bin/env python
#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Test script for NavigationDAODTesterAlgv2 - verifies R2 to R3 navigation conversion on DAOD level.

if __name__ == "__main__":
    import sys
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # flags.Exec.VerboseMessageComponents = ["*Trigger*"]  # uncomment for debugging

    # Input DAOD: the file must contain the R2 TrigMatch_<chain> branches AND
    # the R2->R3 converted HLTNav_Summary_DAODSlimmed branch. No central file
    # can serve as a default input: set the path here or on the command line
    # (--filesInput=<file>, handled by flags.fillFromArgs()).
    flags.Input.Files = [""]
    flags.Exec.MaxEvents = -1
    flags.Exec.SkipEvents = 0
    flags.Common.MsgSuppression = False
    flags.fillFromArgs()
    flags.lock()

    # Initialize main services and merge necessary configurations.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.ComponentFactory import CompFactory
    # from AthenaCommon.Constants import DEBUG  # for the optional debug OutputLevel below

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
    cfg.merge(CutFlowSvcCfg(flags))

    # MetaDataSvc is scheduled by TrigDecisionToolCfg, no need to add it explicitly.

    # Obtain the default Trigger Decision Tool (configured from the file metadata).
    # HLTSummary and NavigationFormat are auto-configured by TrigDecisionToolCfg.
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = cfg.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    # Create matching and composite tools
    r3MatchingTool = CompFactory.Trig.R3MatchingTool("R3MatchingTool")
    r3MatchingTool.TrigDecisionTool = tdt
    # Subfeatures OFF while confirming R2 == R3 equivalence. The TrigMatch
    # branches only contain the primary (highest-pT) feature, so the R3 side
    # must use the primary feature only for a like-for-like comparison.
    r3MatchingTool.IncludeSubfeatures = False

    matchFromCompositeTool = CompFactory.Trig.MatchFromCompositeTool("MatchFromCompositeTool")
    matchFromCompositeTool.InputPrefix = "TrigMatch_"
    # Pointer/shallow equality for the R2 match (DRThreshold < 0 disables DR
    # matching), following the physics-analysis usage of MatchFromCompositeTool.
    matchFromCompositeTool.DRThreshold = -1
    matchFromCompositeTool.MatchShallow = True


    # Select triggers to test - the definitive list comes from the FILE itself:
    # every TrigMatch_<chain> container in the DAOD is a chain with a pre-stored
    # R2 matching reference to compare against.
    # Do NOT select the list via TriggerListsHelper + flags.Trigger.EDMVersion:
    # a converted DAOD reports EDMVersion 3 ("HLTNav_Summary.* found in POOL
    # file"), so the helper returns the Run-3 menu names, none of which exist
    # in a Run-2 file -> a full sweep would vacuously pass (found 2026-06-12).
    list_triggers = sorted(c[len("TrigMatch_"):] for c in flags.Input.Collections
                           if c.startswith("TrigMatch_"))

    print("########################### Testing triggers:", list_triggers)

    # Note: a single R3-configured TDT is sufficient when running from a DAOD -
    # the converted navigation is already in the file, there is no Run2 payload
    # left, so no separate Run2/Run3 TDT pair is needed (unlike the AOD test).

    # --- Create NavigationDAODTesterAlgv2 ---
    # This algorithm compares R2 (from pre-stored composites) and R3 (from converted navigation)
    # matching results for offline physics objects.

    checker = CompFactory.Trig.NavigationDAODTesterAlgv2(
        TrigDecisionTool = tdt,
        R3MatchingTool = r3MatchingTool,
        MatchFromCompositeTool = matchFromCompositeTool,
        # Exception list: chains for which the R2/R3 comparison CANNOT succeed
        # by construction (substring match against chain names).
        # 1) Chains listed in the converter's SpecialCases.h excludedChains are
        #    deliberately NOT converted: isPassed (decision bits) stays true and
        #    the R2 TrigMatch reference exists, but the R3 navigation is
        #    intentionally absent -> a guaranteed, EXPECTED "R2 passes R3 fails".
        #    Keep this list in sync with SpecialCases.h.
        # Note: "nomucomb" itself no longer needs excluding - its linearised
        # TrigMatch layout is handled by the per-object path
        # (ChainsWithLinearisedR2Matching, default ["nomucomb"]).
        ChainsToExclude = [
            # === SpecialCases.h excludedChains (not converted by design) ===
            "HLT_mu20_msonly_mu6noL1_msonly_nscan05",
            "HLT_mu6_dRl1_mu20_msonly_iloosems_mu6noL1_dRl1_msonly",
            "HLT_g45_loose_6j45_0eta240",
            "HLT_mu18_2mu4_JpsimumuL2",
            "HLT_mu18_2mu0noL1_JpsimumuFS",
            "HLT_mu20_2mu0noL1_JpsimumuFS",
            "HLT_mu20_2mu2noL1_JpsimumuFS",
            "HLT_mu20_2mu4_JpsimumuL2",
            "HLT_mu20_2mu4noL1",
            "HLT_2mu4_bJpsimumu",
            "HLT_2mu4_bUpsimumu",
            "HLT_2mu6_bJpsimumu",          # also matches the _delayed variants
            "HLT_2mu6_bUpsimumu",          # also matches the _delayed variants
            "HLT_mu11_nomucomb_2mu4noL1_nscan03_L1MU11_2MU6",  # also matches _bTau
            # === known open issues, deferred to a follow-up (substring match) ===
            # 2) The muon noL1 family: the noL1 leg is a full-scan reconstruction
            #    with a joint MultiComb hypo deciding all thresholds at once, so
            #    Run 2 stores no per-leg structure to convert; in addition the R2
            #    TrigMatch reference is often empty for these chains. Note that
            #    the plain 2mu4noL1 chains (mu18_2mu4noL1, mu11_2mu4noL1_nscan03*)
            #    validate cleanly and stay in the test.
            "mu8noL1",           # mu18/mu20/mu22/mu20_ivarmedium + e24/e26 variants
            "mu6noL1_nscan03",   # l2idonly / nomucomb / mu11_bTau variants
            "msonly_nscan05",    # mu20_msonly_mu10noL1 / mu15noL1 variants
            # 3) Individual chains with unresolved chain-level disagreements:
            "HLT_e26_lhtight_nod0_e15_etcut_L1EM7_Zee",  # etcut leg + Zee topological special case
            "HLT_2mu6_10invm30_pt2_z10",                 # invariant-mass selection
        ],

        PrintSubfeatures = False)  # set True (+ OutputLevel = DEBUG) to inspect subfeatures
    checker.Chains = list_triggers

    cfg.addEventAlgo(checker)


    # Alter the MessageSvc output.
    msg = cfg.getService('MessageSvc')
    msg.Format = '% F%35W%C% F%9W%e%7W%R%T %0W%M'

    cfg.printConfig(withDetails=True, summariseProps=False)
    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
