# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TgcL0TruthValidationAlgCfg(
        flags,
        name="TgcL0TruthValidationAlg",
        **kwargs):
    """Configure the ROOT-independent TGC truth-validation calculation."""
    acc = ComponentAccumulator()
    if "TrackExtrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault(
            "TrackExtrapolator",
            acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    acc.addEventAlgo(CompFactory.L0Muon.TgcL0TruthValidationAlg(name, **kwargs))
    return acc


def TgcL0RootOutputAlgCfg(
        flags,
        name="TgcL0RootOutputAlg",
        outputFile="L0MuonTGCValidation.root",
        **kwargs):
    """Configure the MuonTester event-level TTree backend."""
    acc = ComponentAccumulator()
    histSvc = CompFactory.THistSvc(
        Output=[f"L0MUONTGCVALID DATAFILE='{outputFile}' OPT='RECREATE'"])
    acc.addService(histSvc)
    acc.addEventAlgo(CompFactory.L0Muon.TgcL0RootOutputAlg(name, **kwargs))
    return acc


def L0MuonS1TGCTruthValidationCfg(
        flags,
        outputFile="L0MuonTGCValidation.root",
        rootOutputKwargs=None,
        **kwargs):
    """Configure truth validation and its MuonTester TTree backend."""
    acc = ComponentAccumulator()
    validation_key = kwargs.setdefault(
        "OutputKey", "L0MuonTGCValidationEvent")
    root_kwargs = dict(rootOutputKwargs or {})
    root_kwargs.setdefault("InputKey", validation_key)
    acc.merge(TgcL0TruthValidationAlgCfg(flags, **kwargs))
    acc.merge(TgcL0RootOutputAlgCfg(
        flags, outputFile=outputFile, **root_kwargs))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from L0MuonEndcap.L0MuonEndcapConfig import L0MuonTGCChainCfg
    from MuonConfig.MuonConfigUtils import executeTest
    from MuonGeoModelTestR4.testGeoModel import (
        MuonPhaseIITestDefaults,
        SetupArgParser,
        setupGeoR4TestCfg,
    )

    parser = SetupArgParser()
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.RDO_R3)
    parser.set_defaults(nEvents=10)
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Common.MsgSuppression = False
    flags, acc = setupGeoR4TestCfg(args, flags)

    validation_candidate_key = "L0MuonTGCValidationCandidates"
    validation_segment_key = "L0MuonTGCValidationSegments"
    acc.merge(
        L0MuonTGCChainCfg(
            flags,
            S1TGC={
                "ValidationCandidateKey": validation_candidate_key,
                "ValidationSegmentKey": validation_segment_key,
            },
        )
    )
    acc.merge(
        L0MuonS1TGCTruthValidationCfg(
            flags,
            CandidateKey=validation_candidate_key,
            SegmentKey=validation_segment_key,
            ValidateSectorLogic=True,
        )
    )
    executeTest(acc)
