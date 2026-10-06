#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)


def TruthMuonCfg(flags):
    result = ComponentAccumulator()

    from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonTruthAlgsCfg
    result.merge(MuonTruthAlgsCfg(flags, useSDO=True, recoAssoc = False))

    return result

def L0MuonRPCSimCfg(flags, name = "L0MuonRPCSim", **kwargs):
    
    result = ComponentAccumulator()
    result.merge(TruthMuonCfg(flags))

    alg = CompFactory.L1Muon.RPCSimulation(name = name, **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.HistPath = 'L0MuonRPCSim'
    monTool.defineHistogram('track_input_eta', path='EXPERT', type='TH1F', title=';#eta_{#mu}^{truth};Muons', xbins=50, xmin=-3, xmax=3)

    alg.MonTool = monTool

    result.addEventAlgo(alg)
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    
    return result
  

if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest
    parser = SetupArgParser()
    parser.set_defaults(inputFile= MuonPhaseIITestDefaults.RDO_R3)
    parser.set_defaults(nEvents = 20)

    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Common.MsgSuppression = False

    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG

    from MuonConfig.MuonByteStreamCnvTestConfig import RpcRdoToRpcDigitCfg
    acc.merge(RpcRdoToRpcDigitCfg(flags))
    ### Create the xAOD::TruthParticles
    from DerivationFrameworkMCTruth.MCTruthCommonConfig import HepMCtoXAODTruthCfg
    acc.merge(HepMCtoXAODTruthCfg(flags))

    # example simulation alg
    acc.merge(L0MuonRPCSimCfg(flags,
                             name = "L0MuonRPCSim",
                             OutputLevel = DEBUG))

    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    acc.merge(setupHistSvcCfg(flags, outFile="L0MuonRPCSim.root", outStream="EXPERT"))

    executeTest(acc)
   
