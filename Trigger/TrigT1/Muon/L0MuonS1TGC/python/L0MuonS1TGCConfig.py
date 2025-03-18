#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def L0MuonTGCSimCfg(flags, name = "L0Muon.TGCSimulation", **kwargs):

    result = ComponentAccumulator()

    alg = CompFactory.L0Muon.TGCSimulation(name = name, **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('nTgcDigits', path='EXPERT', type='TH1F', title=';n_{Digit}^{TGC};Events', xbins=50, xmin=0, xmax=100)

    alg.MonTool = monTool

    histSvc = CompFactory.THistSvc(Output=["EXPERT DATAFILE='" + name + ".root' OPT='RECREATE'"])

    result.addEventAlgo(alg)
    result.addService(histSvc)
    return result
  

if __name__ == "__main__":
    
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(inputFile= ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/myRDO.R3.pool.root"])
    parser.set_defaults(nEvents = 20)

    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Common.MsgSuppression = False

    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG

    from MuonConfig.MuonByteStreamCnvTestConfig import TgcRdoToTgcDigitCfg
    acc.merge(TgcRdoToTgcDigitCfg(flags, TgcDigitContainer = "TGC_DIGITS", TgcRdoContainer = 'TGCRDO'))


    # example simulation alg
    acc.merge(L0MuonTGCSimCfg(flags, OutputLevel = DEBUG))


    executeTest(acc)
 
