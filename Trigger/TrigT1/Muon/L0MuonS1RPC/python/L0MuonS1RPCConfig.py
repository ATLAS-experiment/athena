#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)

def L0MuonRPCSimCfg(flags, name = "L0MuonRPCSim", **kwargs):
    
    result = ComponentAccumulator()

    alg = CompFactory.L0Muon.RPCSimulation(name = name, **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.HistPath = 'L0MuonRPCSim'
    monTool.defineHistogram('track_input_eta', path='EXPERT', type='TH1F', title=';#eta_{#mu}^{truth};Muons', xbins=50, xmin=-3, xmax=3)

    alg.MonTool = monTool

    histSvc = CompFactory.THistSvc(Output=["EXPERT DATAFILE='" + name + ".root' OPT='RECREATE'"])

    result.addEventAlgo(alg)
    result.addService(histSvc)
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    
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

    from MuonConfig.MuonByteStreamCnvTestConfig import RpcRdoToRpcDigitCfg
    acc.merge(RpcRdoToRpcDigitCfg(flags))
    # example simulation alg
    acc.merge(L0MuonRPCSimCfg(flags,
                             name = "L0MuonRPCSim",
                             OutputLevel = DEBUG))

    executeTest(acc)
   
