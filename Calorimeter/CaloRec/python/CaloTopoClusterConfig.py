# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import MeV

def caloTopoCoolFolderCfg(flags):
    result=ComponentAccumulator()
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    # rely on global tag for both MC and data; do not specify folder tags
    # use CALO_OFL only for GEO>=18
    hadCalibFolders = [
        "HadCalibration2/CaloEMFrac",
        "HadCalibration2/H1ClusterCellWeights",
        "HadCalibration2/CaloOutOfCluster",
        "HadCalibration2/CaloOutOfClusterPi0",
        "HadCalibration2/CaloDMCorr2"
    ]
    hadCalibPrefix = "/CALO/"
    hadCalibDB = "CALO_ONL"
    if flags.Input.isMC:
        hadCalibPrefix = "/CALO/Ofl/"
        hadCalibDB = "CALO_OFL"
    hadCalibFolders = [ hadCalibPrefix + foldername for foldername in hadCalibFolders ]
    result.merge(addFolders(flags, hadCalibFolders, hadCalibDB, className="CaloLocalHadCoeff"))

    return result

def getTopoClusterLocalCalibTools(flags):
    CaloLCClassificationTool, CaloLCWeightTool, CaloLCOutOfClusterTool, CaloLCDeadMaterialTool=CompFactory.getComps("CaloLCClassificationTool","CaloLCWeightTool","CaloLCOutOfClusterTool","CaloLCDeadMaterialTool",)
    CaloClusterLocalCalib=CompFactory.CaloClusterLocalCalib
    # Local cell weights
    LCClassify   = CaloLCClassificationTool("LCClassify")
    LCClassify.ClassificationKey   = "EMFracClassify"
    LCClassify.UseSpread = False
    LCClassify.MaxProbability = 0.5
    # add the moments EM_PROBABILITY, HAD_WEIGHT, OOC_WEIGHT, DM_WEIGHT to the AOD:
    LCClassify.StoreClassificationProbabilityInAOD = True
    LCClassify.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    LCWeight = CaloLCWeightTool("LCWeight")
    LCWeight.CorrectionKey       = "H1ClusterCellWeights"
    LCWeight.SignalOverNoiseCut  = 2.0
    # *****
    LCWeight.UseHadProbability   = True

    LocalCalib = CaloClusterLocalCalib ("LocalCalib")
    LocalCalib.ClusterClassificationTool     = [LCClassify]
    LocalCalib.ClusterRecoStatus             = [1,2]
    LocalCalib.LocalCalibTools               = [LCWeight]
    LocalCalib.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    # Out-of-cluster corrections
    LCOut     = CaloLCOutOfClusterTool("LCOut")
    LCOut.CorrectionKey       = "OOCCorrection"
    LCOut.UseEmProbability    = False
    LCOut.UseHadProbability   = True

    OOCCalib   = CaloClusterLocalCalib ("OOCCalib")
    OOCCalib.ClusterRecoStatus   = [1,2]
    OOCCalib.LocalCalibTools     = [LCOut]
    OOCCalib.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    LCOutPi0  = CaloLCOutOfClusterTool("LCOutPi0")
    LCOutPi0.CorrectionKey    = "OOCPi0Correction"
    LCOutPi0.UseEmProbability  = True
    LCOutPi0.UseHadProbability = False

    OOCPi0Calib   = CaloClusterLocalCalib ("OOCPi0Calib")
    OOCPi0Calib.ClusterRecoStatus   = [1,2]
    OOCPi0Calib.LocalCalibTools     = [LCOutPi0]

    OOCPi0Calib.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    # Dead material corrections
    LCDeadMaterial   = CaloLCDeadMaterialTool("LCDeadMaterial")
    LCDeadMaterial.HadDMCoeffKey       = "HadDMCoeff2"
    LCDeadMaterial.ClusterRecoStatus   = 0
    LCDeadMaterial.WeightModeDM        = 2
    LCDeadMaterial.UseHadProbability   = True
    LCDeadMaterial.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    DMCalib    = CaloClusterLocalCalib ("DMCalib")
    DMCalib.ClusterRecoStatus   = [1,2]
    DMCalib.LocalCalibTools      = [LCDeadMaterial]

    DMCalib.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    lccalibtools = [
        LocalCalib,
        OOCCalib,
        OOCPi0Calib,
        DMCalib]
    return lccalibtools

def getTopoMoments(flags):
    result=ComponentAccumulator()
    CaloClusterMomentsMaker=CompFactory.CaloClusterMomentsMaker
    TopoMoments = CaloClusterMomentsMaker ("TopoMoments")
    TopoMoments.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
    from AthenaCommon.SystemOfUnits import deg
    TopoMoments.MaxAxisAngle = 20*deg
    TopoMoments.TwoGaussianNoise = flags.Calo.TopoCluster.doTwoGaussianNoise
    TopoMoments.MinBadLArQuality = 4000
    TopoMoments.MomentsNames = ["FIRST_PHI"
                                ,"FIRST_ETA"
                                ,"SECOND_R"
                                ,"SECOND_LAMBDA"
                                ,"DELTA_PHI"
                                ,"DELTA_THETA"
                                ,"DELTA_ALPHA"
                                ,"CENTER_X"
                                ,"CENTER_Y"
                                ,"CENTER_Z"
                                ,"CENTER_MAG"
                                ,"CENTER_LAMBDA"
                                ,"LATERAL"
                                ,"LONGITUDINAL"
                                ,"FIRST_ENG_DENS"
                                ,"ENG_FRAC_EM"
                                ,"ENG_FRAC_MAX"
                                ,"ENG_FRAC_CORE"
                                ,"FIRST_ENG_DENS"
                                ,"SECOND_ENG_DENS"
                                ,"ISOLATION"
                                ,"ENG_BAD_CELLS"
                                ,"N_BAD_CELLS"
                                ,"N_BAD_CELLS_CORR"
                                ,"BAD_CELLS_CORR_E"
                                ,"BADLARQ_FRAC"
                                ,"ENG_POS"
                                ,"SIGNIFICANCE"
                                ,"CELL_SIGNIFICANCE"
                                ,"CELL_SIG_SAMPLING"
                                ,"AVG_LAR_Q"
                                ,"AVG_TILE_Q"
                                ,"PTD"
                                ,"MASS"
                                ,"SECOND_TIME"
                                ,"NCELL_SAMPLING"
                                ]


    if not flags.Common.isOnline:
        from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
        result.merge(LArElecCalibDBCfg(flags,["HVScaleCorr"]))
        if flags.Input.isMC:
            TopoMoments.LArHVFraction=CompFactory.LArHVFraction(HVScaleCorrKey="LArHVScaleCorr")
        else:
            TopoMoments.LArHVFraction=CompFactory.LArHVFraction(HVScaleCorrKey="LArHVScaleCorrRecomputed")
        TopoMoments.MomentsNames += ["ENG_BAD_HV_CELLS"
                                     ,"N_BAD_HV_CELLS"
                                     ]
    result.setPrivateTools(TopoMoments)
    return result

# a.k.a. DigiTruth
def getTopoTruthMoments(flags):
    CaloClusterMomentsMaker_DigiHSTruth=CompFactory.CaloClusterMomentsMaker_DigiHSTruth
    TopoMoments_Truth = CaloClusterMomentsMaker_DigiHSTruth ("TopoMoments_Truth")
    TopoMoments_Truth.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
    from AthenaCommon.SystemOfUnits import deg
    TopoMoments_Truth.MaxAxisAngle = 20*deg
    TopoMoments_Truth.TwoGaussianNoise = flags.Calo.TopoCluster.doTwoGaussianNoise
    TopoMoments_Truth.MinBadLArQuality = 4000
    TopoMoments_Truth.MomentsNames = ["FIRST_PHI_DigiHSTruth"
                                      ,"FIRST_ETA_DigiHSTruth"
                                      ,"SECOND_R_DigiHSTruth"
                                      ,"SECOND_LAMBDA_DigiHSTruth"
                                      ,"DELTA_PHI_DigiHSTruth"
                                      ,"DELTA_THETA_DigiHSTruth"
                                      ,"DELTA_ALPHA_DigiHSTruth"
                                      ,"CENTER_X_DigiHSTruth"
                                      ,"CENTER_Y_DigiHSTruth"
                                      ,"CENTER_Z_DigiHSTruth"
                                      ,"CENTER_MAG_DigiHSTruth"
                                      ,"CENTER_LAMBDA_DigiHSTruth"
                                      ,"LATERAL_DigiHSTruth"
                                      ,"LONGITUDINAL_DigiHSTruth"
                                      ,"ENG_FRAC_CORE_DigiHSTruth"
                                      ,"FIRST_ENG_DENS_DigiHSTruth"
                                      ,"SECOND_ENG_DENS_DigiHSTruth"
                                      ,"ISOLATION_DigiHSTruth"
                                      ,"BAD_CELLS_CORR_E_DigiHSTruth"
                                      ,"ENG_POS_DigiHSTruth"
                                      ,"SIGNIFICANCE_DigiHSTruth"
                                      ,"CELL_SIGNIFICANCE_DigiHSTruth"
                                      ,"CELL_SIG_SAMPLING_DigiHSTruth"
                                      ,"AVG_LAR_Q_DigiHSTruth"
                                      ,"AVG_TILE_Q_DigiHSTruth"
                                      ,"ENERGY_DigiHSTruth"
                                      ,"PHI_DigiHSTruth"
                                      ,"ETA_DigiHSTruth"
                                      ]
    return TopoMoments_Truth

def getTopoCalibMoments(flags):
    CaloCalibClusterMomentsMaker2=CompFactory.CaloCalibClusterMomentsMaker2
    TopoCalibMoments = CaloCalibClusterMomentsMaker2 ("TopoCalibMoments")
    TopoCalibMoments.MomentsNames = ["ENG_CALIB_TOT"
                                     ,"ENG_CALIB_OUT_L"
                                     ,"ENG_CALIB_OUT_T"
                                     ,"ENG_CALIB_EMB0"
                                     ,"ENG_CALIB_EME0"
                                     ,"ENG_CALIB_TILEG3"
                                     ,"ENG_CALIB_DEAD_TOT"
                                     ,"ENG_CALIB_DEAD_EMB0"
                                     ,"ENG_CALIB_DEAD_TILE0"
                                     ,"ENG_CALIB_DEAD_TILEG3"
                                     ,"ENG_CALIB_DEAD_EME0"
                                     ,"ENG_CALIB_DEAD_HEC0"
                                     ,"ENG_CALIB_DEAD_FCAL"
                                     ,"ENG_CALIB_DEAD_LEAKAGE"
                                     ,"ENG_CALIB_DEAD_UNCLASS"
                                     ,"ENG_CALIB_FRAC_EM"
                                     ,"ENG_CALIB_FRAC_HAD"
                                     ,"ENG_CALIB_FRAC_REST"]

    TopoCalibMoments.CalibrationHitContainerNames = ["LArCalibrationHitInactive"
                                                     ,"LArCalibrationHitActive"
                                                     ,"TileCalibHitActiveCell"
                                                     ,"TileCalibHitInactiveCell"]
    TopoCalibMoments.DMCalibrationHitContainerNames = ["LArCalibrationHitDeadMaterial"
                                                       ,"TileCalibHitDeadMaterial"]
    return TopoCalibMoments

def CaloTopoClusterToolCfg(flags, cellsname):
    result=ComponentAccumulator()
    # maker tools
    TopoMaker = CompFactory.CaloTopoClusterMaker("TopoMaker")

    TopoMaker.CellsName = cellsname
    TopoMaker.CalorimeterNames=["LAREM",
                                "LARHEC",
                                "LARFCAL",
                                "TILE"]
    # cells from the following samplings will be able to form
    # seeds. By default no sampling is excluded
    TopoMaker.SeedSamplingNames = ["PreSamplerB", "EMB1", "EMB2", "EMB3",
                                   "PreSamplerE", "EME1", "EME2", "EME3",
                                   "HEC0", "HEC1","HEC2", "HEC3",
                                   "TileBar0", "TileBar1", "TileBar2",
                                   "TileExt0", "TileExt1", "TileExt2",
                                   "TileGap1", "TileGap2", "TileGap3",
                                   "FCAL0", "FCAL1", "FCAL2"]
    TopoMaker.NeighborOption = "super3D"
    TopoMaker.RestrictHECIWandFCalNeighbors  = False
    TopoMaker.RestrictPSNeighbors  = True
    TopoMaker.CellThresholdOnEorAbsEinSigma     =    0.0
    TopoMaker.NeighborThresholdOnEorAbsEinSigma =    2.0
    TopoMaker.SeedThresholdOnEorAbsEinSigma     =    4.0

    #timing
    TopoMaker.SeedCutsInT = flags.Calo.TopoCluster.doTimeCut
    TopoMaker.CutOOTseed = flags.Calo.TopoCluster.extendTimeCut and flags.Calo.TopoCluster.doTimeCut
    TopoMaker.UseTimeCutUpperLimit = flags.Calo.TopoCluster.useUpperLimitForTimeCut
    TopoMaker.TimeCutUpperLimit = flags.Calo.TopoCluster.timeCutUpperLimit
    TopoMaker.XTalkEM2 = flags.Calo.TopoCluster.xtalkEM2
    TopoMaker.XTalkEM2D = flags.Calo.TopoCluster.xtalkEM2D
    TopoMaker.XTalkEM2n = flags.Calo.TopoCluster.xtalkEM2n
    TopoMaker.XTalkEM3 = flags.Calo.TopoCluster.xtalkEM3
    TopoMaker.XTalkEMEta = flags.Calo.TopoCluster.xtalkEMEta
    TopoMaker.XTalkDeltaT = flags.Calo.TopoCluster.xtalkDeltaT
    TopoMaker.XTalk2Eratio1 = flags.Calo.TopoCluster.xtalk2Eratio1
    TopoMaker.XTalk2Eratio2 = flags.Calo.TopoCluster.xtalk2Eratio2
    TopoMaker.XTalk3Eratio = flags.Calo.TopoCluster.xtalk3Eratio
    TopoMaker.XTalkEtaEratio = flags.Calo.TopoCluster.xtalkEtaEratio
    TopoMaker.XTalk2DEratio = flags.Calo.TopoCluster.xtalk2DEratio
    
    # note E or AbsE
    #
    # the following property must be set to TRUE in order to make double
    # sided cuts on the seed and the cluster level
    #
    TopoMaker.SeedCutsInAbsE                 = True
    TopoMaker.ClusterEtorAbsEtCut            = 0.0*MeV
    # use 2-gaussian or single gaussian noise for TileCal
    TopoMaker.TwoGaussianNoise = flags.Calo.TopoCluster.doTwoGaussianNoise
    result.setPrivateTools(TopoMaker)
    return result

def CaloTopoClusterSplitterToolCfg(flags):
    result=ComponentAccumulator()
    TopoSplitter = CompFactory.CaloTopoClusterSplitter("TopoSplitter")
    # cells from the following samplings will be able to form local
    # maxima. The excluded samplings are PreSamplerB, EMB1,
    # PreSamplerE, EME1, all Tile samplings, all HEC samplings and the
    # two rear FCal samplings.
    #
    TopoSplitter.SamplingNames = ["EMB2", "EMB3",
                                  "EME2", "EME3",
                                  "FCAL0"]
    # cells from the following samplings will also be able to form
    # local maxima but only if they are not overlapping in eta and phi
    # with local maxima in previous samplings from the primary list.
    #
    TopoSplitter.SecondarySamplingNames = ["EMB1","EME1",
                                           "TileBar0","TileBar1","TileBar2",
                                           "TileExt0","TileExt1","TileExt2",
                                           "HEC0","HEC1","HEC2","HEC3",
                                           "FCAL1","FCAL2"]
    TopoSplitter.ShareBorderCells = True
    TopoSplitter.RestrictHECIWandFCalNeighbors  = False
    TopoSplitter.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
    result.setPrivateTools(TopoSplitter)
    return result

def CaloTopoClusterCfg(flags, cellsname="AllCalo", clustersname=None, clustersnapname="CaloTopoClusters"):
    """
    Configures topo clustering

    If output writing is enabled (ESD,AOD) the topo clusters are added to them
    """

    doLCCalib = flags.Calo.TopoCluster.doTopoClusterLocalCalib
        
    if clustersname is None:
        clustersname = "CaloCalTopoClusters" if doLCCalib else "CaloTopoClusters"


    if clustersname=="CaloTopoClusters" and doLCCalib is True: 
        raise RuntimeError("Inconistent arguments: Name must not be 'CaloTopoClusters' if doLCCalib is True")

    result=ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    # Schedule total noise cond alg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))

    CaloClusterMaker, CaloClusterSnapshot=CompFactory.getComps("CaloClusterMaker","CaloClusterSnapshot",)

    result.merge(LArGMCfg(flags))

    result.merge(TileGMCfg(flags))

    TopoMaker = result.popToolsAndMerge( CaloTopoClusterToolCfg(flags, cellsname=cellsname))
    TopoSplitter = result.popToolsAndMerge( CaloTopoClusterSplitterToolCfg(flags) )
    #
    # the following options are not set, since these are the default
    # values
    #
    # NeighborOption                = "super3D",
    # NumberOfCellsCut              = 4,
    # EnergyCut                     = 500*MeV,

    CaloTopoCluster=CaloClusterMaker(clustersname+"Maker")
    CaloTopoCluster.ClustersOutputName=clustersname

    CaloTopoCluster.ClusterMakerTools = [TopoMaker, TopoSplitter]

    from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
    caloBadChanTool = result.popToolsAndMerge( CaloBadChanToolCfg(flags) )
    CaloClusterBadChannelList=CompFactory.CaloClusterBadChannelList
    BadChannelListCorr = CaloClusterBadChannelList(badChannelTool = caloBadChanTool)
    CaloTopoCluster.ClusterCorrectionTools += [BadChannelListCorr]

    momentsMaker=result.popToolsAndMerge(getTopoMoments(flags))
    CaloTopoCluster.ClusterCorrectionTools += [momentsMaker]

    if flags.Calo.TopoCluster.doCalibHitMoments:
        calibHitsMomentsMaker=getTopoCalibMoments(flags)
        CaloTopoCluster.ClusterCorrectionTools += [calibHitsMomentsMaker]

    CaloTopoCluster.ClustersOutputName=clustersname
    
    if doLCCalib:
        theCaloClusterSnapshot=CaloClusterSnapshot(OutputName=clustersnapname,SetCrossLinks=True)        
        CaloTopoCluster.ClusterCorrectionTools += [theCaloClusterSnapshot]
        #if not clustersname:
        CaloTopoCluster.ClusterCorrectionTools += getTopoClusterLocalCalibTools(flags)

        from CaloRec.CaloTopoClusterConfig import caloTopoCoolFolderCfg
        result.merge(caloTopoCoolFolderCfg(flags))


    result.addEventAlgo(CaloTopoCluster,primary=True)

    if CaloTopoCluster.ClustersOutputName in flags.Calo.TopoCluster.skipWriteList:
        # don't add these clusters to ESD and AOD
        return result
    

    #Output config:
    AODMoments=[ "SECOND_R" 
                 ,"SECOND_LAMBDA"
                 ,"CENTER_MAG"
                 ,"CENTER_LAMBDA"
                 ,"FIRST_ENG_DENS"
                 ,"ENG_FRAC_MAX" 
                 ,"ISOLATION"
                 ,"ENG_BAD_CELLS"
                 ,"N_BAD_CELLS"
                 ,"BADLARQ_FRAC"
                 ,"ENG_POS"
                 ,"SIGNIFICANCE"
                 ,"AVG_LAR_Q"
                 ,"AVG_TILE_Q"
                 ,"EM_PROBABILITY"
                 ,"BadChannelList"
                 ,"SECOND_TIME"
                 ,"NCELL_SAMPLING"]

    if flags.Calo.TopoCluster.writeExtendedClusterMoments:
        AODMoments += ["LATERAL"
                       ,"LONGITUDINAL"
                       ,"CELL_SIGNIFICANCE"
                       ,"PTD"
                       ,"MASS"]

    if flags.Reco.EnableHI:
        AODMoments += ["CELL_SIG_SAMPLING"]

    if flags.Calo.TopoCluster.writeCalibHitClusterMoments:
        AODMoments += ["ENG_CALIB_TOT"
                       ,"ENG_CALIB_OUT_L"
                       ,"ENG_CALIB_OUT_T"
                       ,"ENG_CALIB_EMB0"
                       ,"ENG_CALIB_EME0"
                       ,"ENG_CALIB_TILEG3"
                       ,"ENG_CALIB_DEAD_TOT"
                       ,"ENG_CALIB_DEAD_EMB0"
                       ,"ENG_CALIB_DEAD_TILE0"
                       ,"ENG_CALIB_DEAD_TILEG3"
                       ,"ENG_CALIB_DEAD_EME0"
                       ,"ENG_CALIB_DEAD_HEC0"
                       ,"ENG_CALIB_DEAD_FCAL"
                       ,"ENG_CALIB_DEAD_LEAKAGE"
                       ,"ENG_CALIB_DEAD_UNCLASS"
                       ,"ENG_CALIB_FRAC_EM"
                       ,"ENG_CALIB_FRAC_HAD"
                       ,"ENG_CALIB_FRAC_REST"]


    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
    toESD = [f"xAOD::CaloClusterContainer#{CaloTopoCluster.ClustersOutputName}",
             f"xAOD::CaloClusterAuxContainer#{CaloTopoCluster.ClustersOutputName}Aux.",
             f"CaloClusterCellLinkContainer#{CaloTopoCluster.ClustersOutputName}_links"]
    toAOD = [f"xAOD::CaloClusterContainer#{CaloTopoCluster.ClustersOutputName}",
             f"CaloClusterCellLinkContainer#{CaloTopoCluster.ClustersOutputName}_links"]

    AODMoments.append("CellLink") #Add data-link to cell-link container
    if flags.Calo.TopoCluster.addCalibrationHitDecoration: #Add calib hit deco if requried 
        AODMoments.append(flags.Calo.TopoCluster.CalibrationHitDecorationName)

    if flags.Calo.TopoCluster.addCPData:
        AODMoments += ["ClusterWidthEta","ClusterWidthPhi"]

    auxItems = f"xAOD::CaloClusterAuxContainer#{CaloTopoCluster.ClustersOutputName}Aux."
    auxItems+= ".".join(AODMoments)    

    toAOD.append(auxItems)
 
    result.merge(addToESD(flags, toESD))
    result.merge(addToAOD(flags, toAOD))

    return result

def addSnapshot(topomaker, corrName,contName):
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger('CaloTopoClusterConfig:addSnapshot')
    corrTools=topomaker.ClusterCorrectionTools
    newCorrTools=[]
    found=False
    for t in corrTools:
        newCorrTools.append(t)
        if (t.getName()==corrName):
            newSnapshot=CompFactory.CaloClusterSnapshot("Snapshot_"+corrName,OutputName=contName)
            newCorrTools.append(newSnapshot)
            found=True
    if not found:
        mlog.error("Did not find cluster correction tool %s", corrName)
    else:
        mlog.info("Added cluster snapshot after correction tool %s", corrName)
        topomaker.ClusterCorrectionTools = newCorrTools
        topomaker.ClusterCorrectionTools += [newSnapshot]
    return   


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root"]  
    flags.Output.ESDFileName="esdOut.pool.root"

    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    theKey="CaloCalTopoClustersNew"

    topoAcc=CaloTopoClusterCfg(flags)
    topoAlg = topoAcc.getPrimary()
    topoAlg.ClustersOutputName=theKey

    cfg.merge(topoAcc)

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(flags,"xAOD", ItemList=["xAOD::CaloClusterContainer#CaloCalTopoClusters*",#+theKey,
                                                            "xAOD::CaloClusterAuxContainer#*CaloCalTopoClusters*Aux.",#+theKey+"Aux.",
                                                            # "CaloClusterCellLinkContainer#"+theKey+"_links"
                                                           ]))

    ThinNegativeEnergyCaloClustersAlg=CompFactory.ThinNegativeEnergyCaloClustersAlg
    theNegativeEnergyCaloClustersThinner = ThinNegativeEnergyCaloClustersAlg(
        "ThinNegativeEnergyCaloClustersAlg",
        CaloClustersKey=theKey,
        ThinNegativeEnergyCaloClusters = True,
        StreamName = 'StreamAOD'
    )
    cfg.addEventAlgo(theNegativeEnergyCaloClustersThinner,"AthAlgSeq")

    
    cfg.addEventAlgo(CompFactory.ClusterDumper("TopoDumper",ContainerName=theKey,FileName="NewTopoClusters.txt"),sequenceName="AthAlgSeq")

#    cfg.getService("StoreGateSvc").Dump=True

    cfg.run(10)
    #f=open("CaloTopoCluster.pkl","wb")
    #cfg.store(f)
    #f.close()
