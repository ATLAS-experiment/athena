#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)

def RPCPatternMatchingCfg(flags, name = "RPCPatternMatching", **kwargs):

    result = ComponentAccumulator()

    alg = CompFactory.L1Muon.RPCPatternMatching(name = name,
                                          **kwargs)

    # from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    # monTool = GenericMonitoringTool(flags, 'MonTool')
    # monTool.HistPath = 'RPCPatternMatching'
    # monTool.defineHistogram('all_muon_pt', title='All muon pT; pT [GeV]; Entries', type='TH1F', path='EXPERT', xbins=100, xmin=0, xmax=100)
    # monTool.defineHistogram('all_muon_eta', title='All muon eta; eta; Entries', type='TH1F', path='EXPERT', xbins=100, xmin=-2.5, xmax=2.5)
    # monTool.defineHistogram('cand_muon_pt', title='Candidate muon pT; pT [GeV]; Entries', type='TH1F', path='EXPERT', xbins=100, xmin=0, xmax=100)
    # monTool.defineHistogram('cand_muon_eta', title='Candidate muon eta; eta; Entries', type='TH1F', path='EXPERT', xbins=100, xmin=-2.5, xmax=2.5)
    # monTool.defineHistogram('n_cand_muons', title='Number of candidate muons; Number of candidates; Entries', type='TH1I', path='EXPERT', xbins=10, xmin=-0.5, xmax=9.5)
    # monTool.defineHistogram('cand_muon_reco_pt', title='Candidate muon reconstructed pT; pT [GeV]; Entries', type='TH1F', path='EXPERT', xbins=100, xmin=0, xmax=100)
    # monTool.defineHistogram('cand_muon_reco_eta', title='Candidate muon reconstructed eta; eta; Entries', type='TH1F', path='EXPERT', xbins=100, xmin=-2.5, xmax=2.5)

    # alg.MonTool = monTool

    result.addEventAlgo(alg)
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    items = [
        "xAOD::L1RPCCandDataContainer#L1RPCCandData",
        # Persist the aux store through its base type to avoid requiring a
        # dedicated AthenaPool converter for xAOD::L1RPCCandDataAuxContainer_v1.
        "xAOD::AuxContainerBase#L1RPCCandDataAux."
    ]
    result.merge(OutputStreamCfg(flags, 
                                "RDO", 
                                ItemList=items,
                                takeItemsFromInput=True)
    )

    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    result.merge(PerfMonMTSvcCfg(flags))

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
    flags.Output.RDOFileName = "test.RDO.pool.root"

    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG

    from MuonConfig.MuonByteStreamCnvTestConfig import RpcRdoToRpcDigitCfg
    acc.merge(RpcRdoToRpcDigitCfg(flags))
    ### Create the xAOD::TruthParticles
    from DerivationFrameworkMCTruth.MCTruthCommonConfig import HepMCtoXAODTruthCfg
    acc.merge(HepMCtoXAODTruthCfg(flags))

    # example simulation alg
    acc.merge(RPCPatternMatchingCfg(flags,
                             name = "RPCPatternMatching",
                             OutputLevel = DEBUG))

    executeTest(acc)

