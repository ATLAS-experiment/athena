# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from TrigEDMConfig.TriggerEDMRun3 import recordable
from TrigT2CaloCalibration.EgammaCalibrationConfig import (EgammaHitsCalibrationBarrelConfig,
                                                           EgammaHitsCalibrationEndcapConfig,
                                                           EgammaGapCalibrationConfig,
                                                           EgammaTransitionRegionsConfig,
                                                           EgammaSshapeCalibrationBarrelConfig,
                                                           EgammaSshapeCalibrationEndcapConfig)

_T2CaloEgamma_ExtraInputs = {
    ('IRegSelLUTCondData', 'ConditionStore+RegSelLUTCondData_TTEM'),
    ('IRegSelLUTCondData', 'ConditionStore+RegSelLUTCondData_TTHEC'),
    ('IRegSelLUTCondData', 'ConditionStore+RegSelLUTCondData_TILE'),
    ('IRegSelLUTCondData', 'ConditionStore+RegSelLUTCondData_FCALEM'),
    ('IRegSelLUTCondData', 'ConditionStore+RegSelLUTCondData_FCALHAD'),
    ('LArBadChannelCont' , 'ConditionStore+LArBadChannel')
}

#local variable
from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
inputEDM = mapThresholdToL1RoICollection("FSNOSEED")

def RingerReFexConfig(flags, name="RingerReMaker", RingerKey="FastCaloRings",
                      ClustersName="HLT_FastCaloEMClusters"):

    from TrigT2CaloEgamma.RingerConstants import Layer
    from TrigT2CaloEgamma.RingerConstants import DETID as det

    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram( "TIME_total", title="Total Time;time [us]",xbins=100, xmin=0, xmax=500,type='TH1F', path='EXPERT')
    monTool.defineHistogram( "TIME_load_cells", title="Load Cells Time;time [us]",xbins=50, xmin=0, xmax=100,type='TH1F', path='EXPERT')

    _lenNRings = 7  # number of entries in NRings

    ringer = CompFactory.RingerReFex(
        name                 = name,
        EtaBins              = [0.0000, 2.500], # bin pairs: min < eta <= max, PS,barrel,crack,endcap
        RingerKey            = RingerKey,
        ClustersName         = ClustersName,
        GlobalCenter         = False,
        EtaSearchWindowSize  = 0.1,
        PhiSearchWindowSize  = 0.1,
        NRings               = [8, 64, 8, 8, 4, 4, 4],
        DeltaEta             = [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.1],
        DeltaPhi             = [0.098174770424681, 0.098174770424681, 0.024543692606170, 0.024543692606170,
                                0.098174770424681, 0.098174770424681, 0.098174770424681],
        UseTile              = True,
        Detectors = [ [det.TTEM], [det.TTEM], [det.TTEM], [det.TTEM], [det.TTHEC, det.TILE],
                      [det.TTHEC, det.TTHEC, det.TILE], [det.TTHEC, det.TILE] ],
        Samplings = [ [0]       , [1]       , [2]       , [3]       , [0        , -1  ],
                      [1        , 2        , -1  ]    , [3        , -1      ] ],
        Samples   = [ [ Layer.PreSamplerB,Layer.PreSamplerE ], # TTEM:0
                      [ Layer.EMB1,       Layer.EME1 ], # TTEM: 1
                      [ Layer.EMB2,       Layer.EME2 ], # TTEM: 2
                      [ Layer.EMB3,       Layer.EME3 ], # TTEM: 3
                      [ Layer.HEC0,       Layer.TileBar0, Layer.TileGap2, Layer.TileExt0 ], # TTHEC: 0, TILE
                      [ Layer.HEC1,       Layer.HEC2,     Layer.TileBar1, Layer.TileGap0, Layer.TileExt1 ], # TTHEC: 1,2, TILE
                      [ Layer.HEC3,       Layer.TileBar2, Layer.TileGap1, Layer.TileExt2 ] # TTHEC: 3, TILE
                     ],
        DumpCells         = flags.Trigger.egamma.dumpCellsFastCaloInfo,
        DoNoiseThrRings   = flags.Trigger.egamma.doNoiseThrRings,
        NoiseFactor       = flags.Trigger.egamma.sigmaNoiseFactor,
        DoQuarter         = [flags.Trigger.egamma.doQuarterRings]*_lenNRings,
        DoEtaAxesDivision = [True]*_lenNRings,
        DoPhiAxesDivision = [True]*_lenNRings,
        MonTool = monTool)
    return ringer

#=======================================================================

def AsymRingerReFexConfig(flags, name="AsymRingerReMaker"):
    ringer = RingerReFexConfig(flags, name, RingerKey="FastCaloAsymRings")
    ringer.DoQuarter = [True]*len(ringer.NRings),
    return ringer

#=======================================================================

def t2CaloEgamma_AllCfg(flags, name="T2CaloEgamma_All",RoIs=inputEDM,ExtraInputs=set(), ClustersName="HLT_FastCaloEMClusters"):
    acc = ComponentAccumulator()
    tool = CompFactory.EgammaAllFex("EgammaAllFex",
                                    IncludeHad=True,
                                    ExtraInputs={('TileEMScale','ConditionStore+TileEMScale'),
                                                 ('TileBadChannels','ConditionStore+TileBadChannels')})
    alg = CompFactory.T2CaloEgammaReFastAlgo(name,
                                             IReAlgToolList= [tool],
                                             EtaWidth = 0.1,
                                             PhiWidth = 0.1,
                                             ExtraInputs = _T2CaloEgamma_ExtraInputs)
    alg.RoIs=RoIs
    alg.ExtraInputs |= ExtraInputs
    alg.ClustersName = recordable(ClustersName)
    acc.addEventAlgo(alg)

    return acc

#=======================================================================

def t2CaloEgamma_AllEmCfg(flags, name="T2CaloEgamma_AllEm", RoIs=inputEDM, ExtraInputs=set(), ClustersName="HLT_FastCaloEMClusters"):
    acc = ComponentAccumulator()
    tool = CompFactory.EgammaAllFex("EgammaAllEmFex",
                                    ExtraInputs={('TileEMScale','ConditionStore+TileEMScale'),
                                                 ('TileBadChannels','ConditionStore+TileBadChannels')})

    alg = CompFactory.T2CaloEgammaReFastAlgo(name,
                                             IReAlgToolList= [tool],
                                             EtaWidth = 0.1,
                                             PhiWidth = 0.1,
                                             ExtraInputs = _T2CaloEgamma_ExtraInputs)
    alg.RoIs=RoIs
    alg.ExtraInputs |= ExtraInputs
    alg.ClustersName = recordable(ClustersName)
    acc.addEventAlgo(alg)
    return acc

#=======================================================================

def t2CaloEgamma_ReFastAlgoCfg(flags, name="T2CaloEgamma_ReFastAlgo", ClustersName="HLT_FastCaloEMClusters",
                               doRinger=False, RingerKey="HLT_FastCaloRinger", RoIs=inputEDM, ExtraInputs=set()):
    acc = ComponentAccumulator()

    samp2 = CompFactory.EgammaReSamp2Fex("ReFaAlgoSamp2FexConfig",
                                         MaxDetaHotCell=0.15, MaxDphiHotCell=0.15 )
    samp1 = CompFactory.EgammaReSamp1Fex("ReFaAlgoSamp1FexConfig")
    sampe = CompFactory.EgammaReEmEnFex("ReFaAlgoEmEnFexConfig")
    samph = CompFactory.EgammaReHadEnFex("ReFaAlgoHadEnFexConfig",
                                         ExtraInputs={('TileEMScale','ConditionStore+TileEMScale'),
                                                      ('TileBadChannels','ConditionStore+TileBadChannels')})
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('TrigEMCluster_eT', path='EXPERT', type='TH1F', title="T2Calo Egamma E_T; E_T [ GeV ] ; Nclusters", xbins=80, xmin=0.0, xmax=80.0)
    monTool.defineHistogram('TrigEMCluster_had1', path='EXPERT', type='TH1F', title="T2Calo Egamma had E_T samp1; had E_T samp1 [ GeV ] ; Nclusters", xbins=80, xmin=0.0, xmax=8.0)
    monTool.defineHistogram('TrigEMCluster_eta', path='EXPERT', type='TH1F', title="T2Calo Egamma #eta; #eta ; Nclusters", xbins=100, xmin=-2.5, xmax=2.5)
    monTool.defineHistogram('TrigEMCluster_phi', path='EXPERT', type='TH1F', title="T2Calo Egamma #phi; #phi ; Nclusters", xbins=128, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('TrigEMCluster_eta,TrigEMCluster_phi', path='EXPERT', type='TH2F', title="T2Calo Egamma Number of Clusters; #eta ; #phi ; Number of Clusters", xbins=100, xmin=-2.5, xmax=2.5, ybins=128, ymin=-3.2, ymax=3.2)
    monTool.defineHistogram('TrigEMCluster_rEta', path='EXPERT', type='TH1F', title="T2Calo Egamma rEta; rEta (e237/e277) ; Nclusters", xbins=140, xmin=-0.2, xmax=1.2)
    monTool.defineHistogram('TIME_exec', path='EXPERT', type='TH1F', title="T2Calo Egamma time; time [ us ] ; Nruns", xbins=80, xmin=0.0, xmax=8000.0)
    monTool.defineHistogram('TrigEMCluster_eta,TIME_exec', path='EXPERT', type='TH2F', title="T2Calo Egamma time vs #eta ; #eta ; time [ us ]", xbins=100, xmin=-2.5, xmax=2.5, ybins=80, ymin=0.0, ymax=8000.0)

    alg = CompFactory.T2CaloEgammaReFastAlgo(
        name,
        IReAlgToolList = [ samp2, samp1, sampe, samph ],
        ExtraInputs = _T2CaloEgamma_ExtraInputs,
        EtaWidth = 0.2,
        PhiWidth = 0.2,
        MonTool = monTool,
        CalibListBarrel = [EgammaSshapeCalibrationBarrelConfig(),
                           EgammaHitsCalibrationBarrelConfig(),
                           EgammaGapCalibrationConfig(),
                           EgammaTransitionRegionsConfig()],
        CalibListEndcap = [EgammaSshapeCalibrationEndcapConfig(),
                           EgammaHitsCalibrationEndcapConfig(),
                           EgammaGapCalibrationConfig()])

    if doRinger:
        ringer = RingerReFexConfig(flags, 'ReFaAlgoRingerFexConfig',
                                   RingerKey= RingerKey,
                                   ClustersName = ClustersName)
        alg.IReAlgToolList += [ringer]


    alg.RoIs=RoIs
    alg.ExtraInputs |= ExtraInputs
    alg.ClustersName = recordable(ClustersName)
    acc.addEventAlgo(alg)
    return acc

#=======================================================================

def t2CaloEgamma_ReFastFWDAlgoCfg(flags,  name="T2CaloEgamma_ReFastFWDAlgo",
                                  ClustersName="HLT_FWDFastCaloEMClusters",
                                  doRinger=False, RingerKey="HLT_FWDFastCaloRinger", RoIs=inputEDM, ExtraInputs=set()):
    acc = ComponentAccumulator()

    alg = CompFactory.T2CaloEgammaForwardReFastAlgo(name,
                                                    IReAlgToolList = [],
                                                    ExtraInputs = _T2CaloEgamma_ExtraInputs,
                                                    EtaWidth = 0.2,
                                                    PhiWidth = 0.2)
    alg.RoIs=RoIs
    alg.ExtraInputs |= ExtraInputs
    alg.ClustersName = recordable(ClustersName)
    acc.addEventAlgo(alg)
    return acc

#=======================================================================

# Test:
if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.lock()
    t2CaloEgamma_ReFastAlgoCfg(flags, "FastCaloL2EgammaAlgo_noRinger").printConfig(withDetails=True,summariseProps=True)
    t2CaloEgamma_ReFastAlgoCfg(flags, "FastCaloL2EgammaAlgo", doRinger=True).printConfig(withDetails=True,summariseProps=True)
    t2CaloEgamma_AllCfg(flags).printConfig(withDetails=True,summariseProps=True)
    t2CaloEgamma_AllEmCfg(flags).printConfig(withDetails=True,summariseProps=True)
    t2CaloEgamma_ReFastFWDAlgoCfg(flags, ClustersName="HLT_FastCaloEMClusters", doRinger=True).printConfig(withDetails=True,summariseProps=True)
