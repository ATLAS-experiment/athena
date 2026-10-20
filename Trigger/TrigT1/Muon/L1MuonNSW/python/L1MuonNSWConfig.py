# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def MMTriggerToolCfg(flags, **kwargs):
    """Run 4 emulation of the MM trigger processor (private tool)."""
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.getComp("L1Muon::L1MM::MMTriggerTool")(**kwargs))
    return result


def L1MuonNSWSimCfg(flags, name="L1Muon.NSWSimulation", **kwargs):
    """Run 4 NSW trigger simulation."""
    result = ComponentAccumulator()

    kwargs.setdefault("DoMM", flags.Detector.GeometryMM)
    if kwargs["DoMM"] and "MMTriggerTool" not in kwargs:
        kwargs["MMTriggerTool"] = result.popToolsAndMerge(MMTriggerToolCfg(flags))

    alg = CompFactory.getComp("L1Muon::NSWSimulation")(name=name, **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, "MonTool")
    # Names must match the Monitored::Scalar names filled in NSWSimulation::execute
    monTool.defineHistogram("nMMDigits", path="EXPERT", type="TH1F", title=";n_{Digit}^{MM};Events",
                            xbins=50, xmin=0, xmax=100)
    monTool.defineHistogram("nsTGCDigits", path="EXPERT", type="TH1F", title=";n_{Digit}^{sTGC};Events",
                            xbins=50, xmin=0, xmax=100)
    monTool.defineHistogram("nMMCandidates", path="EXPERT", type="TH1F", title=";n_{Candidate}^{MM};Events",
                            xbins=50, xmin=0, xmax=50)
    alg.MonTool = monTool

    result.addEventAlgo(alg, primary=True)
    return result


if __name__ == "__main__":

    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg, executeTest

    parser = SetupArgParser()
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.RDO_R3)
    parser.set_defaults(nEvents=20)
    args = parser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Common.MsgSuppression = False

    flags, acc = setupGeoR4TestCfg(args, flags)

    acc.merge(setupHistSvcCfg(flags, outFile="L1MuonNSW_Expert.root", outStream="EXPERT"))

    from MuonConfig.MuonByteStreamCnvTestConfig import STGC_RdoToDigitCfg, MM_RdoToDigitCfg
    if flags.Detector.GeometrysTGC:
        acc.merge(STGC_RdoToDigitCfg(flags, sTgcRdoContainer="sTGCRDO", sTgcDigitContainer="sTGC_DIGITS"))

    if flags.Detector.GeometryMM:
        acc.merge(MM_RdoToDigitCfg(flags, MmRdoContainer="MMRDO", MmDigitContainer="MM_DIGITS"))

    acc.merge(L1MuonNSWSimCfg(flags))

    executeTest(acc)
