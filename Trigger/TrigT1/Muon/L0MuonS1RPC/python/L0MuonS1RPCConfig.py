#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)


def TruthMuonCfg(flags):
    result = ComponentAccumulator()


    from MuonTruthAlgsR4.MuonTruthAlgsConfig import TruthSegmentMakerCfg, TruthSegmentToTruthPartAssocCfg, SdoMultiTruthMakerCfg
    from MuonConfig.MuonTruthAlgsConfig import TruthMuonMakerAlgCfg, MuonTruthHitCountsAlgCfg
    result.merge(TruthMuonMakerAlgCfg(flags))
    result.merge(MuonTruthHitCountsAlgCfg(flags))
    result.merge(TruthSegmentToTruthPartAssocCfg(flags))
    result.merge(TruthSegmentMakerCfg(flags))
    result.merge(TruthSegmentToTruthPartAssocCfg(flags))
    result.merge(SdoMultiTruthMakerCfg(flags, useSDO=True))


    return result
def L0MuonRPCSimCfg(flags, name = "L0MuonRPCSim", **kwargs):
    
    result = ComponentAccumulator()
    result.merge(TruthMuonCfg(flags))

    alg = CompFactory.L0Muon.RPCSimulation(name = name, **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.HistPath = 'L0MuonRPCSim'
    monTool.defineHistogram('track_input_eta', path='EXPERT', type='TH1F', title=';#eta_{#mu}^{truth};Muons', xbins=50, xmin=-3, xmax=3)

    alg.MonTool = monTool
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags, outFile=f"{name}.root", outStream="EXPERT"))


    result.addEventAlgo(alg)
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    
    return result
  

if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser
    from MuonConfig.MuonConfigUtils import executeTest
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
    ### Create the xAOD::TruthParticles
    from xAODTruthCnv.xAODTruthCnvConfig import GEN_EVNT2xAODCfg
    acc.merge(GEN_EVNT2xAODCfg(flags,name="GEN_EVNT2xAOD",AODContainerName="TruthEvent"))

    # example simulation alg
    acc.merge(L0MuonRPCSimCfg(flags,
                             name = "L0MuonRPCSim",
                             OutputLevel = DEBUG))

    executeTest(acc)
   
