#Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

_log = logging.getLogger(__name__)

def L0MuonNSWSimCfg(flags, name = "L0Muon.NSWSimulation", **kwargs):

    result = ComponentAccumulator()

    alg = CompFactory.L0Muon.NSWSimulation(name = name, **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('nNSWDigits', path='EXPERT', type='TH1F', title=';n_{Digit}^{NSW};Events', xbins=50, xmin=0, xmax=100)

    alg.MonTool = monTool

    result.addEventAlgo(alg, primary=True)
    return result
  

if __name__ == "__main__":
    
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg, executeTest
    
    parser = SetupArgParser()
    parser.set_defaults(inputFile= MuonPhaseIITestDefaults.RDO_R3)
    parser.set_defaults(nEvents = 20)
    args = parser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Common.MsgSuppression = False

    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG

    acc.merge(setupHistSvcCfg(flags, outFile="L0MuonNSW_Expert.root", outStream="EXPERT"))
    
    from MuonConfig.MuonByteStreamCnvTestConfig import STGC_RdoToDigitCfg, MM_RdoToDigitCfg
    if flags.Detector.GeometrysTGC:
        acc.merge(STGC_RdoToDigitCfg(flags, sTgcRdoContainer="sTGCRDO", sTgcDigitContainer="sTGC_DIGITS"))

    if flags.Detector.GeometryMM:
        acc.merge(MM_RdoToDigitCfg(flags, MmRdoContainer="MMRDO", MmDigitContainer="MM_DIGITS"))

    acc.merge(L0MuonNSWSimCfg(flags, OutputLevel = DEBUG))

    executeTest(acc)
