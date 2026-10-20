# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthMuonReferenceToolCfg(
        flags,
        name="TruthMuonReferenceTool",
        **kwargs):
    """Configure the shared generator-truth reference provider.

    Common defaults apply no pT or detector-eta acceptance cut. A composition
    chooses any reference selection and passes this same public tool to its
    common truth branch and all subsystem reference-position modules. Distinct
    selections need distinct tool names; they must not share one reference
    branch. Detector-specific configurations may supply their own defaults.
    """
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.L1Muon.TruthMuonReferenceTool(name, **kwargs),
        primary=True,
    )
    return acc


def TruthMuonBranchToolCfg(flags, name="TruthMuonBranchTool", **kwargs):
    """Configure the subsystem-independent selected truth-muon branch."""
    acc = ComponentAccumulator()
    if "TruthMuonReferenceTool" not in kwargs:
        kwargs["TruthMuonReferenceTool"] = acc.getPrimaryAndMerge(
            TruthMuonReferenceToolCfg(flags)
        )
    acc.setPrivateTools(CompFactory.L1Muon.TruthMuonBranchTool(name, **kwargs))
    return acc


def MuonTriggerValidationCfg(
        flags,
        name="MuonTriggerValidationAlg",
        outputFile="MuonTriggerValidation.root",
        configureHistSvc=True,
        **kwargs):
    """Configure the common MuonTester ROOT producer.

    Subsystem-specific content is supplied through ``BranchTools``.  The
    producer itself only owns common tree/EventInfo infrastructure.  Matching,
    calibration fits and performance calculations are intentionally left to
    downstream ROOT analysis.
    """
    acc = ComponentAccumulator()

    kwargs.setdefault("IsMC", flags.Input.isMC)

    if configureHistSvc:
        histSvc = CompFactory.THistSvc(
            Output=[
                f"MUONTRIGGERVALID DATAFILE='{outputFile}' OPT='RECREATE'"
            ]
        )
        acc.addService(histSvc)

    acc.addEventAlgo(
        CompFactory.L1Muon.MuonTriggerValidationAlg(name, **kwargs)
    )
    return acc
