# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import MeV, deg
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TriggerMenuMT.HLT.CommonSequences.FullScanDefs import em_clusters, lc_clusters, fs_towers, fs_cells

from TrigEDMConfig.TriggerEDM import recordable

mlog = logging.getLogger ('TrigCaloRecConfig')


def trigCaloClusterMakerMonTool(flags, doMonCells = False):
    """Monitoring tool for TrigCaloClusterMaker"""

    monTool = GenericMonitoringTool(flags, 'MonTool')

    maxNumberOfClusters = 1200 if doMonCells else 50
    maxProcTime = 150000 if doMonCells else 4500

    monTool.defineHistogram('container_size', path='EXPERT', type='TH1F',  title="Container Size; Number of Clusters; Number of Events", xbins=50, xmin=0.0, xmax=maxNumberOfClusters)
    monTool.defineHistogram('container_size_by_mu', path='EXPERT', type='TH1F',  title="Container Size; Number of Clusters; Number of Events", xbins=50, xmin=0.0, xmax=maxNumberOfClusters/60)
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title="Total Execution Time; Execution time [ us ] ; Number of runs", xbins=100, xmin=0.0, xmax=maxProcTime)
    monTool.defineHistogram('TIME_ClustMaker', path='EXPERT', type='TH1F', title="Cluster Maker Time; Execution time [ us ] ; Number of runs", xbins=100, xmin=0.0, xmax=maxProcTime)
    monTool.defineHistogram('TIME_ClustCorr', path='EXPERT', type='TH1F', title="Cluster Correction Time; Execution time [ us ] ; Number of runs", xbins=100, xmin=0.0, xmax=100)
    monTool.defineHistogram('Et', path='EXPERT', type='TH1F',  title="Cluster E_T; E_T [ MeV ] ; Number of Clusters", xbins=135, xmin=-200.0, xmax=2500.0)
    monTool.defineHistogram('Eta', path='EXPERT', type='TH1F', title="Cluster #eta; #eta ; Number of Clusters", xbins=100, xmin=-2.5, xmax=2.5)
    monTool.defineHistogram('Phi', path='EXPERT', type='TH1F', title="Cluster #phi; #phi ; Number of Clusters", xbins=64, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('Eta,Phi', path='EXPERT', type='TH2F', title="Number of Clusters; #eta ; #phi ; Number of Clusters", xbins=100, xmin=-2.5, xmax=2.5, ybins=128, ymin=-3.2, ymax=3.2)
    monTool.defineHistogram('clusterSize', path='EXPERT', type='TH1F', title="Cluster Type; Type ; Number of Clusters", xbins=13, xmin=0.5, xmax=13.5)
    monTool.defineHistogram('signalState', path='EXPERT', type='TH1F', title="Signal State; Signal State ; Number of Clusters", xbins=4, xmin=-1.5, xmax=2.5)
    monTool.defineHistogram('size', path='EXPERT', type='TH1F', title="Cluster Size; Size [Cells] ; Number of Clusters", xbins=125, xmin=0.0, xmax=250.0)
    monTool.defineHistogram('N_BAD_CELLS', path='EXPERT', type='TH1F', title="N_BAD_CELLS; N_BAD_CELLS ; Number of Clusters", xbins=250, xmin=0.5, xmax=250.5)
    monTool.defineHistogram('ENG_FRAC_MAX', path='EXPERT', type='TH1F', title="ENG_FRAC_MAX; ENG_FRAC_MAX ; Number of Clusters", xbins=50, xmin=0.0, xmax=1.1)
    monTool.defineHistogram('mu', path='EXPERT', type='TH1F',  title="mu; mu; Number of Events", xbins=50, xmin=0.0, xmax=100)
    monTool.defineHistogram('mu,container_size', path='EXPERT', type='TH2F',  title="Container Size versus #mu; #mu; cluster container size", xbins=50, xmin=20.0, xmax=70, ybins=50, ymin=0.0, ymax=maxNumberOfClusters)

    if doMonCells:
        monTool.defineHistogram('count_1thrsigma', path='EXPERT', type='TH1F',  title="count_1thrsigma; count_1thresigma; Number of Events", xbins=60, xmin=0.0, xmax=12e3)
        monTool.defineHistogram('count_2thrsigma', path='EXPERT', type='TH1F',  title="count_2thrsigma; count_2thresigma; Number of Events", xbins=60, xmin=0.0, xmax=6e3)
        monTool.defineHistogram('count_1thrsigma_by_mu2', path='EXPERT', type='TH1F',  title="count_1thrsigma_by_mu2; count_1thresigma_by_mu2; Number of Events", xbins=50, xmin=0.0, xmax=10)
        monTool.defineHistogram('count_2thrsigma_by_mu2', path='EXPERT', type='TH1F',  title="count_2thrsigma_by_mu2; count_2thresigma_by_mu2; Number of Events", xbins=50, xmin=0.0, xmax=5)
        monTool.defineHistogram('mu,count_1thrsigma', path='EXPERT', type='TH2F',  title="nCells above 1st thr versus #mu; #mu; nCells", xbins=50, xmin=20.0, xmax=70, ybins=60, ymin=0.0, ymax=12e3)
        monTool.defineHistogram('mu,count_2thrsigma', path='EXPERT', type='TH2F',  title="nCells above 2nd thr versus #mu; #mu; nCells", xbins=50, xmin=20.0, xmax=70, ybins=60, ymin=0.0, ymax=6e3)

    return monTool


@AccumulatorCache
def hltCaloCellMakerCfg(flags, name=None, roisKey='UNSPECIFIED', CellsName=None, monitorCells=False, doTau=False):
    acc = ComponentAccumulator()
    from TrigT2CaloCommon.TrigCaloDataAccessConfig import trigCaloDataAccessSvcCfg, CaloDataAccessSvcDependencies
    acc.merge(trigCaloDataAccessSvcCfg(flags))
    #choose RoI for fullscan
    if (roisKey == 'UNSPECIFIED'):
        from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
        roisKey = mapThresholdToL1RoICollection("FSNOSEED")
    # choose cells name given parameters
    cellsFromName = 'CaloCellsFS' if "FS" in name else "CaloCells"
    cells = cellsFromName if CellsName is None else CellsName

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('Cells_N', path='EXPERT', type='TH1F',  title="Cells N; NCells; events",
                            xbins=40, xmin=0, xmax=1600 if monitorCells else 240000)
    monTool.defineHistogram('TIME_exec', path='EXPERT', type='TH1F', title="Cells time; time [ us ] ; Nruns",
                            xbins=80, xmin=0, xmax=800 if monitorCells else 160000)
    if monitorCells:
        monTool.defineHistogram('Cells_eT', path='EXPERT', type='TH1F',  title="Cells E_T; E_T [ GeV ] ; Nclusters",
                                xbins=100, xmin=0.0, xmax=100.0)
        monTool.defineHistogram('Cells_eta', path='EXPERT', type='TH1F', title="Cells #eta; #eta ; Nclusters",
                                xbins=100, xmin=-2.5, xmax=2.5)
        monTool.defineHistogram('Cells_phi', path='EXPERT', type='TH1F', title="Cells #phi; #phi ; Nclusters",
                                xbins=128, xmin=-3.2, xmax=3.2)

    cellMaker = CompFactory.HLTCaloCellMaker(name,
                                             CellsName = cells,
                                             TrigDataAccessMT = acc.getService('TrigCaloDataAccessSvc'),
                                             ExtraInputs = CaloDataAccessSvcDependencies,
                                             RoIs=roisKey,
                                             monitorCells = monitorCells,
                                             MonTool = monTool,
                                             TileCellsInROI = False if not doTau else True)
    acc.addEventAlgo(cellMaker, primary=True)
    return acc

@AccumulatorCache
def hltCaloCellCorrectorCfg(flags,name='HLTCaloCellCorrector', inputEDM='CellsClusters', outputEDM='CorrectedCellsClusters', eventShape='HIEventShape'):
    acc = ComponentAccumulator()
    cellCorrector = CompFactory.HLTCaloCellCorrector(name = name,
                                                     EventShapeCollection = eventShape,
                                                     InputCellKey = inputEDM,
                                                     OutputCellKey = outputEDM)
    acc.addEventAlgo(cellCorrector)
    return acc           
  

@AccumulatorCache
def hltCaloCellSeedlessMakerCfg(flags, roisKey='UNSPECIFIED'):
    acc = ComponentAccumulator()
    hltCaloCellMakerAcc = hltCaloCellMakerCfg(flags, "CaloCellSeedLessFS",
                                                    roisKey = roisKey,
                                                    CellsName ="SeedLessFS", 
                                                    monitorCells=False)

    acc.merge(hltCaloCellMakerAcc)

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags, noisetype="electronicNoise"))
    acc.addCondAlgo(CompFactory.CaloNoiseSigmaDiffCondAlg())

    return acc


@AccumulatorCache
def L0CaloGlobalRoIBuilderCfg(flags,DoNoiseThrRings=True):
    acc = ComponentAccumulator()
    from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import RingerReFexConfig
    nameTool='RingerGlobalFex'
    nameAlgo='L0CaloGlobalRoIBuilder'
    nameContCalo='CaloClustersGlobal'
    nameContRinger='RingerGlobal'
    if ( DoNoiseThrRings ):
        nameTool='RingerGlobal2sigFex'
        nameAlgo='L0CaloGlobalRoI2sigBuilder'
        nameContCalo='CaloClusters2sigGlobal'
        nameContRinger='Ringer2sigGlobal'
    ringer = RingerReFexConfig(flags,name=nameTool,RingerKey='NOTNEEDED',
          ClustersName=nameContCalo,DoNoiseThrRings=DoNoiseThrRings)
    L0CaloGlobalRoIBuilderAlg = CompFactory.CaloGlobalRoIBuilder(name=nameAlgo,
                      Cells ="SeedLessFS", ClustersName=nameContCalo,
                      RingerKey=nameContRinger,
                      RingerTool=ringer )
    acc.addEventAlgo(L0CaloGlobalRoIBuilderAlg)

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags))

    return acc

def CaloL0RingerPreCfg(flags,DoNoiseThrRings=True):
    flags.Trigger.ExtraEDMList+= CaloL0RingerPrepareList(DoNoiseThrRings)

def CaloL0RingerPrepareList(DoNoiseThrRings=True):
    extraEDMList=[]
    if DoNoiseThrRings : 
       extraEDMList+=[('xAOD::TrigRingerRingsContainer#Ringer2sigGlobal',  'BS ESD AODFULL', 'Calo'), ('xAOD::TrigRingerRingsAuxContainer#Ringer2sigGlobalAux.',  'BS ESD AODFULL', 'Calo'), ('xAOD::TrigEMClusterContainer#CaloClusters2sigGlobal',  'BS ESD AODFULL', 'Calo'), ('xAOD::TrigEMClusterAuxContainer#CaloClusters2sigGlobalAux.',  'BS ESD AODFULL', 'Calo')]
    else : 
       extraEDMList+=[('xAOD::TrigRingerRingsContainer#RingerGlobal',  'BS ESD AODFULL', 'Calo'), ('xAOD::TrigRingerRingsAuxContainer#RingerGlobalAux.',  'BS ESD AODFULL', 'Calo'), ('xAOD::TrigEMClusterContainer#CaloClustersGlobal',  'BS ESD AODFULL', 'Calo'), ('xAOD::TrigEMClusterAuxContainer#CaloClustersGlobalAux.',  'BS ESD AODFULL', 'Calo')]
    return extraEDMList

def CaloL0RingerCfg(flags,DoNoiseThrRings=True):
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
    extraContent=CaloL0RingerPrepareList(DoNoiseThrRings)
    acc = ComponentAccumulator()
    if (flags.Output.doWriteRDO):
       acc.merge(hltCaloCellSeedlessMakerCfg(flags))
       acc.merge(L0CaloGlobalRoIBuilderCfg(flags,DoNoiseThrRings=DoNoiseThrRings))

    if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
       if ( flags.Output.doWriteESD ):
          acc.merge(addToESD(flags, extraContent))
       if ( flags.Output.doWriteAOD ):
          acc.merge(addToAOD(flags, extraContent))
    return acc



def hltCaloLocalCalib(flags, name = "TrigLocalCalib"):
    det_version_is_rome = flags.GeoModel.AtlasVersion.startswith("Rome")
    localCalibTool = CompFactory.CaloLCWeightTool("TrigLCWeight",
           CorrectionKey="H1ClusterCellWeights",
           SignalOverNoiseCut=2.0, UseHadProbability=True)
    trigLCClassify = CompFactory.CaloLCClassificationTool("TrigLCClassify",
           ClassificationKey="EMFracClassify",
           UseSpread=False, MaxProbability=0.85 if det_version_is_rome else 0.5,
           UseNormalizedEnergyDensity=not det_version_is_rome,
           StoreClassificationProbabilityInAOD=True)
    tool = CompFactory.CaloClusterLocalCalib( name,
           ClusterRecoStatus=[1, 2], ClusterClassificationTool=[ trigLCClassify ],
           LocalCalibTools=[ localCalibTool ])
    return tool


def hltCaloOOCalib(flags, name = "TrigOOCCalib"):
    localCalibTool = CompFactory.CaloLCOutOfClusterTool("TrigLCOut",
           CorrectionKey="OOCCorrection",UseEmProbability=False,
           UseHadProbability=True)
    tool = CompFactory.CaloClusterLocalCalib( name,
           ClusterRecoStatus=[1, 2],
           LocalCalibTools=[ localCalibTool ] )
    return tool

def hltCaloOOCPi0Calib(flags, name = "TrigOOCPi0Calib" ):
    localCalibTool = CompFactory.CaloLCOutOfClusterTool("TrigLCOutPi0",
           CorrectionKey="OOCPi0Correction", UseEmProbability=True,
           UseHadProbability=False)
    tool = CompFactory.CaloClusterLocalCalib( name,
           ClusterRecoStatus=[1, 2],
           LocalCalibTools=[ localCalibTool ] )
    return tool

def hltCaloDMCalib(flags, name = "TrigDMCalib" ):
    localCalibTool = CompFactory.CaloLCDeadMaterialTool("TrigLCDeadMaterial",
           HadDMCoeffKey="HadDMCoeff2", ClusterRecoStatus=0,
           WeightModeDM=2,UseHadProbability=True)
    tool = CompFactory.CaloClusterLocalCalib( name,
            ClusterRecoStatus=[1, 2],
            LocalCalibTools=[ localCalibTool ] )
    return tool



@AccumulatorCache
def hltTopoClusterMakerCfg(flags, name, clustersKey="HLT_TopoCaloClustersFS",
                           cellsKey=None, doLC=False):
    acc = ComponentAccumulator()
    cellsFromName = 'CaloCellsFS' if "FS" in clustersKey else "CaloCells"
    cells = cellsFromName if cellsKey is None else cellsKey

    from CaloRec.CaloTopoClusterConfig import (
        CaloTopoClusterToolCfg,
        CaloTopoClusterSplitterToolCfg,
    )

    topoMaker = acc.popToolsAndMerge(CaloTopoClusterToolCfg(flags, cellsname=cells))
    topoMaker.RestrictPSNeighbors = False
    # TODO - Don't use hasFlag here, use another concrete flag instead
    if flags.hasFlag("CaloRecGPU.ActiveConfig"):
       topoMaker.UseGPUCriteria=flags.CaloRecGPU.ActiveConfig.UseOriginalCriteria
    listClusterCorrectionTools = []
    if doLC :
       from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
       # We need the electronic noise for the LC weights
       acc.merge(CaloNoiseCondAlgCfg(flags, noisetype="electronicNoise"))
       from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
       acc.merge(caloTopoCoolFolderCfg(flags))
       listClusterCorrectionTools = [ hltCaloLocalCalib(flags), hltCaloOOCalib(flags),
             hltCaloOOCPi0Calib(flags), hltCaloDMCalib(flags) ]

    #timing
    topoMaker.SeedCutsInT = flags.Trigger.Calo.TopoCluster.doTimeCut
    topoMaker.CutOOTseed = flags.Trigger.Calo.TopoCluster.extendTimeCut and flags.Trigger.Calo.TopoCluster.doTimeCut
    topoMaker.UseTimeCutUpperLimit = flags.Trigger.Calo.TopoCluster.useUpperLimitForTimeCut
    topoMaker.TimeCutUpperLimit = flags.Trigger.Calo.TopoCluster.timeCutUpperLimit
    
    topoSplitter = acc.popToolsAndMerge(CaloTopoClusterSplitterToolCfg(flags))

    topoMoments = CompFactory.CaloClusterMomentsMaker ('TrigTopoMoments')
    topoMoments.MaxAxisAngle = 20*deg
    topoMoments.TwoGaussianNoise = flags.Calo.TopoCluster.doTwoGaussianNoise
    topoMoments.MinBadLArQuality = 4000
    topoMoments.MomentsNames = ['FIRST_PHI',
                                'FIRST_ETA',
                                'SECOND_R' ,
                                'SECOND_LAMBDA',
                                'DELTA_PHI',
                                'DELTA_THETA',
                                'DELTA_ALPHA' ,
                                'CENTER_X',
                                'CENTER_Y',
                                'CENTER_Z',
                                'CENTER_MAG',
                                'CENTER_LAMBDA',
                                'LATERAL',
                                'LONGITUDINAL',
                                'FIRST_ENG_DENS',
                                'ENG_FRAC_EM',
                                'ENG_FRAC_MAX',
                                'ENG_FRAC_CORE' ,
                                'FIRST_ENG_DENS',
                                'SECOND_ENG_DENS',
                                'ISOLATION',
                                'ENG_BAD_CELLS',
                                'N_BAD_CELLS',
                                'N_BAD_CELLS_CORR',
                                'BAD_CELLS_CORR_E',
                                'BADLARQ_FRAC',
                                'ENG_POS',
                                'SIGNIFICANCE',
                                'CELL_SIGNIFICANCE',
                                'CELL_SIG_SAMPLING',
                                'AVG_LAR_Q',
                                'AVG_TILE_Q'
                                ]

    doMonCells = "FS" in name
    alg = CompFactory.TrigCaloClusterMaker(
        name,
        Cells=cells,
        CaloClusters=recordable(clustersKey),
        CellLinks = clustersKey+"_links",
        ClusterMakerTools = [ topoMaker, topoSplitter, topoMoments], # moments are missing yet
        ClusterCorrectionTools = listClusterCorrectionTools,
        MonCells = doMonCells,
        MonTool = trigCaloClusterMakerMonTool(flags, doMonCells) )

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags))
    acc.addEventAlgo(alg, primary=True)
    return acc


@AccumulatorCache
def hltCaloTopoClusterCalibratorCfg(flags, name, clustersin, clustersout, **kwargs):
    """ Create the LC calibrator """
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg

    # We need the electronic noise for the LC weights
    acc = ComponentAccumulator()
    acc.merge(CaloNoiseCondAlgCfg(flags, noisetype="electronicNoise"))

    from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
    acc.merge(caloTopoCoolFolderCfg(flags))

    calibrator = CompFactory.TrigCaloClusterCalibrator(
        name, InputClusters=clustersin, OutputClusters=clustersout,
        **kwargs
        #OutputCellLinks = clustersout+"_cellLinks", **kwargs
    )

    calibrator.ClusterCorrectionTools = [ hltCaloLocalCalib(flags), hltCaloOOCalib(flags),
             hltCaloOOCPi0Calib(flags), hltCaloDMCalib(flags) ]
    #NB: Could we take these from CaloRec.CaloTopoClusterConfig.getTopoClusterLocalCalibTools?

    # Monitoring
    monTool = GenericMonitoringTool(flags, "MonTool")
    monTool.defineHistogram('Et', path='EXPERT', type='TH1F',
                            title="Cluster E_T; E_T [ MeV ] ; Number of Clusters",
                            xbins=135, xmin=-200.0, xmax=2500.0)
    monTool.defineHistogram('Eta', path='EXPERT', type='TH1F',
                            title="Cluster #eta; #eta ; Number of Clusters",
                            xbins=100, xmin=-2.5, xmax=2.5)
    monTool.defineHistogram('Phi', path='EXPERT', type='TH1F',
                            title="Cluster #phi; #phi ; Number of Clusters",
                            xbins=64, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('Eta,Phi', path='EXPERT', type='TH2F',
                            title="Number of Clusters; #eta ; #phi ; Number of Clusters",
                            xbins=100, xmin=-2.5, xmax=2.5, ybins=128, ymin=-3.2, ymax=3.2)
    calibrator.MonTool = monTool

    acc.addEventAlgo(calibrator, primary=True)
    return acc

##################### Unifying all cluster reco algs together ##################
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys	  import getTrigEgammaKeys
TrigEgammaKeys = getTrigEgammaKeys()
TrigEgammaKeys_LRT = getTrigEgammaKeys(name = '_LRT')
TrigEgammaKeys_HI = getTrigEgammaKeys(ion = True)

def prepareFlagsGPUHLT(flags):
    flags.LAr.doHVCorr=True
    # NOTE: "HLT" flag subdomain defaults moved to CaloRecGPUFlags
    return

@AccumulatorCache
def hltCaloTopoClusteringCfg(
    flags, namePrefix=None,nameSuffix=None, CellsName=None, monitorCells=False, roisKey="UNSPECIFIED",clustersKey=None, doLCFS=False, doTau = False):
    if doTau:
        CellsName = "CaloCellsLC"
        clustersKeyFromName = "HLT_TopoCaloClustersLC"
    elif nameSuffix == "FS":
        clustersKeyFromName = em_clusters
    else:
        clustersKeyFromName = TrigEgammaKeys.precisionTopoClusterContainer
    clusters = clustersKeyFromName if clustersKey is None else clustersKey
    acc = ComponentAccumulator()
    acc.merge(
        hltCaloCellMakerCfg(flags, namePrefix + "HLTCaloCellMaker"+nameSuffix, roisKey=roisKey, CellsName=CellsName, monitorCells=monitorCells, doTau = doTau)
    )
    # TODO - Don't use hasFlag here, use another concrete flag instead
    if flags.hasFlag("CaloRecGPU.ActiveConfig") and (nameSuffix == "FS") and (not doTau):
       from CaloRecGPU.CaloRecGPUConfig import HybridClusterProcessorCfg
       hyb = HybridClusterProcessorCfg(flags, namePrefix + "HLTCaloClusterMaker"+nameSuffix)
       acc.merge(hyb)
    else : 
       calt=hltTopoClusterMakerCfg(flags, namePrefix + "HLTCaloClusterMaker"+nameSuffix,cellsKey=CellsName, clustersKey=clusters, doLC=doTau)
       acc.merge(calt)
    if doLCFS:
        acc.merge( hltCaloTopoClusterCalibratorCfg(
                                                   flags,
                                                   "HLTCaloClusterCalibratorLCFS",
                                                   clustersin=em_clusters,
                                                   clustersout=lc_clusters,
                                                   OutputCellLinks=lc_clusters + "_cellLinks",
                                                  )
                 )
    return acc

###################################EgammaSpecific TopoClustering####################################
@AccumulatorCache
def egammaTopoClusteringCfg(flags, RoIs):
  cfg = hltCaloTopoClusteringCfg(flags, namePrefix="", nameSuffix="RoI", CellsName="CaloCells",  monitorCells=True, roisKey=RoIs)
  return cfg


@AccumulatorCache
def egammaTopoClusteringCfg_LRT(flags, RoIs):
  cfg = hltCaloTopoClusteringCfg(flags, namePrefix="", nameSuffix="RoI_LRT", CellsName="CaloCells",  monitorCells=True, roisKey=RoIs, clustersKey= TrigEgammaKeys_LRT.precisionTopoClusterContainer)
  return cfg


###################################JetMetSpecific TopoClustering####################################
@AccumulatorCache
def jetmetTopoClusteringCfg(flags, RoIs):
  cfg = hltCaloTopoClusteringCfg(flags, namePrefix="", nameSuffix="FS", CellsName="CaloCellsFS",  monitorCells=False, roisKey=RoIs)
  return cfg

@AccumulatorCache
def jetmetTopoClusteringCfg_LC(flags, RoIs):
  cfg = hltCaloTopoClusteringCfg(flags, namePrefix="", nameSuffix="FS", CellsName="CaloCellsFS",  monitorCells=False, roisKey=RoIs, doLCFS=True)
  return cfg

###################################TauSpecific TopoClustering####################################
@AccumulatorCache
def tauTopoClusteringCfg(flags, RoIs):
  cfg = hltCaloTopoClusteringCfg(flags, namePrefix="Tau", nameSuffix="", CellsName="CaloCellsLC",  monitorCells=False, roisKey=RoIs, clustersKey="HLT_TopoCaloClustersLC", doTau= True)
  return cfg

@AccumulatorCache
def hltCaloTopoClusteringHICfg(
    flags, CellsName=None, roisKey="UNSPECIFIED", doLC=False,algSuffix='HIRoI', ion=True):
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import  getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(ion=ion)
    eventShape = TrigEgammaKeys.egEventShape
    clustersKey = TrigEgammaKeys.precisionTopoClusterContainer
    acc = ComponentAccumulator()
    acc.merge(hltCaloCellMakerCfg(flags, "HLTCaloCellMaker"+algSuffix, roisKey=roisKey, CellsName=CellsName, monitorCells=True))
    acc.merge(hltCaloCellCorrectorCfg(flags,name='HLTRoICaloCellCorrector', inputEDM='CaloCells', outputEDM='CorrectedRoICaloCells', eventShape=eventShape))
    acc.merge(hltTopoClusterMakerCfg(flags, "TrigCaloClusterMaker_topo"+algSuffix, clustersKey=clustersKey,cellsKey="CorrectedRoICaloCells"))
    return acc

@AccumulatorCache
def hltHICaloTowerMakerCfg(flags, name, towersKey, cellsKey="CaloCellsFS", RoIs=""):
    acc = ComponentAccumulator()
    larcmbtwrbldr = CompFactory.LArTowerBuilderTool("LArCmbTwrBldr",
                                        CellContainerName = cellsKey,
                                        IncludedCalos     = [ "LAREM", "LARHEC" ]
                                        )
    
    fcalcmbtwrbldr = CompFactory.LArFCalTowerBuilderTool("FCalCmbTwrBldr",
                                                CellContainerName = cellsKey,
                                                MinimumEt         = 0.*MeV
                                                )

    #input to  TileTowerBuilder:  cells in TILE
    tilecmbtwrbldr = CompFactory.TileTowerBuilderTool("TileCmbTwrBldr",
                                            CellContainerName = cellsKey,
                                            # debugging aid, keep for convenience
                                            #DumpTowers        = False,
                                            #DumpWeightMap     = False
                                            )



    alg = CompFactory.TrigCaloTowerMaker(name,
                                        Cells=cellsKey,
                                        CaloTowers=towersKey,
                                        NumberOfPhiTowers=64,
                                        NumberOfEtaTowers=100,
                                        EtaMin=-5.0,
                                        EtaMax=5.0,
                                        DeltaEta=1.2,
                                        DeltaPhi=1.2,
                                        RoIs=RoIs,
                                        TowerMakerTools = [ tilecmbtwrbldr, larcmbtwrbldr, fcalcmbtwrbldr ]
                                        )
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags))
    acc.addEventAlgo(alg, primary=True)
    return acc

@AccumulatorCache
def hltHICaloClusterMakerCfg(flags, name, towersKey, cellsKey, clustersKey) :
    """Function to equip HLT HI cluster builder from towers and cells, adds to output AOD stream"""
    acc = ComponentAccumulator()

    
    alg=CompFactory.HIClusterMaker(name,
                          InputTowerKey=towersKey,
                          CaloCellContainerKey=cellsKey,
                          OutputContainerKey=clustersKey
                          )
    acc.addEventAlgo(alg, primary=True)
    return acc

@AccumulatorCache
def HICaloTowerCfg(flags):
    """ Create the towers for heavy ion """
    acc = ComponentAccumulator()
    acc.merge(
              hltCaloCellMakerCfg(flags, "HLTCaloCellMakerFS", roisKey='')
             )
    # Then build the towers
    acc.merge(
              hltHICaloTowerMakerCfg(
              flags,
              "HLTHICaloTowerMakerFS",
              towersKey=fs_towers,
              cellsKey=fs_cells,
              )
    )
    # Then build the clusters
    acc.merge(
              hltHICaloClusterMakerCfg(
              flags,
              "HLTHICaloClusterMakerFS",
              towersKey=fs_towers,
              cellsKey=fs_cells,
              clustersKey = "HLT_HICaloClustersFS"
              )
    )

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = "CONDBR2-ES1PA-2022-07"
    flags.Common.isOnline = True
    outputContainers = ["CaloCellContainer#SeedLessFS",
              "xAOD::EventInfo#EventInfo",
              "xAOD::TrigEMClusterContainer#CaloClustersGlobal",
              "xAOD::TrigEMClusterAuxContainer#CaloClustersGlobalAux.",
              "xAOD::TrigRingerRingsContainer#RingerGlobal",
              "xAOD::TrigRingerRingsAuxContainer#RingerGlobalAux."]
    flags.Output.ESDFileName='TrigCaloRecCheck'
    
    flags.fillFromArgs()
    flags.dump()
    flags.lock()    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    cfg = MainServicesCfg(flags)

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    cfg.merge(DetDescrCnvSvcCfg(flags))

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))
    cfg.getService("ByteStreamCnvSvc").ROD2ROBmap=["-1"]

    storeGateSvc = cfg.getService("StoreGateSvc")
    storeGateSvc.Dump=True
    theL0CaloGlobalRoIBuilderCfg = L0CaloGlobalRoIBuilderCfg(flags)
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    
    CAs = [hltCaloCellSeedlessMakerCfg(flags,roisKey=''),
           theL0CaloGlobalRoIBuilderCfg,
           hltCaloCellMakerCfg(flags, "SthFS",roisKey=''),
           OutputStreamCfg(flags,flags.Output.ESDFileName,ItemList=outputContainers)]
           #hltTopoClusterMakerCfg(flags, "TrigCaloClusterMaker_topoFS")]

    for ca in CAs:
        ca.printConfig(withDetails=True, summariseProps=True)
        #ca.wasMerged()
        cfg.merge(ca)


    cfg.run(50)
