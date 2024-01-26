# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaCommon.SystemOfUnits import MeV, ns, cm, deg

def createFlagsCaloRecGPU():
    flags = AthConfigFlags()
    flags.addFlag('MeasureTimes', True)
    flags.addFlag('CellsName', "AllCalo")
    flags.addFlag('ClustersOutputName',"ClustersOut")
    flags.addFlag('FillMissingCells', False)
    flags.addFlag('MissingCellsToFill', [])
    flags.addFlag('ClusterSize', 'Topo_420')
    flags.addFlag('ClusterEtorAbsEtCut', -1e-16*MeV)
    flags.addFlag('CalorimeterNames', ["LAREM", "LARHEC", "LARFCAL", "TILE"])
    flags.addFlag('TopoClusterSeedSamplingNames', ["PreSamplerB", "EMB1", "EMB2", "EMB3", "PreSamplerE", "EME1", "EME2", "EME3", "HEC0", "HEC1","HEC2", "HEC3", "TileBar0", "TileBar1", "TileBar2", "TileExt0", "TileExt1", "TileExt2", "TileGap1", "TileGap2", "TileGap3", "FCAL0", "FCAL1", "FCAL2"])
    flags.addFlag('TopoClusterSNRSeedThreshold',4.0)
    flags.addFlag('TopoClusterSNRGrowThreshold',2.0)
    flags.addFlag('TopoClusterSNRCellThreshold',0.0)
    flags.addFlag('TopoClusterSeedCutsInAbsE',True)
    flags.addFlag('TopoClusterNeighborCutsInAbsE',True)
    flags.addFlag('TopoClusterCellCutsInAbsE',True)
    flags.addFlag('NeighborOption',"super3D")
    flags.addFlag('RestrictHECIWandFCalNeighbors',False)
    flags.addFlag('RestrictPSNeighbors',True)
    flags.addFlag('AlsoRestrictPSOnGPUSplitter',False)
    flags.addFlag('TwoGaussianNoise',True)
    flags.addFlag('SeedCutsInT',False)
    flags.addFlag('CutOOTseed',False)
    flags.addFlag('UseTimeCutUpperLimit',False)
    flags.addFlag('TimeCutUpperLimit',20.0)
    flags.addFlag('TreatL1PredictedCellsAsGood',True)
    flags.addFlag('UseEM2CrossTalk',False)
    flags.addFlag('CrossTalkDeltaT',15*ns)
    flags.addFlag('SeedThresholdOnTAbs',12.5*ns)
    flags.addFlag('SplitterNumberOfCellsCut',4)
    flags.addFlag('SplitterEnergyCut',500*MeV)
    flags.addFlag('SplitterSamplingNames',["EMB2", "EMB3", "EME2", "EME3", "FCAL0"])
    flags.addFlag('SplitterSecondarySamplingNames',["EMB1","EME1", "TileBar0","TileBar1","TileBar2", "TileExt0","TileExt1","TileExt2", "HEC0","HEC1","HEC2","HEC3", "FCAL1","FCAL2"])
    flags.addFlag('SplitterShareBorderCells',True)
    flags.addFlag('EMShowerScale',5.0*cm)
    flags.addFlag('SplitterUseNegativeClusters',True)
    flags.addFlag('UseAbsEnergyMoments',True)
    flags.addFlag('MomentsMaxAxisAngle',20*deg)
    flags.addFlag('MomentsMinBadLArQuality',4000)
    MomentsToCalculate=[ "FIRST_PHI",
                                    "FIRST_ETA",
                                    "SECOND_R",
                                    "SECOND_LAMBDA",
                                    "DELTA_PHI",
                                    "DELTA_THETA",
                                    "DELTA_ALPHA",
                                    "CENTER_X",
                                    "CENTER_Y",
                                    "CENTER_Z",
                                    "CENTER_MAG",
                                    "CENTER_LAMBDA",
                                    "LATERAL",
                                    "LONGITUDINAL",
                                    "ENG_FRAC_EM",
                                    "ENG_FRAC_MAX",
                                    "ENG_FRAC_CORE",
                                    "FIRST_ENG_DENS",
                                    "SECOND_ENG_DENS",
                                    "ISOLATION",
                                    "ENG_BAD_CELLS",
                                    "N_BAD_CELLS",
                                    "N_BAD_CELLS_CORR",
                                    "BAD_CELLS_CORR_E",
                                    "BADLARQ_FRAC",
                                    "ENG_POS",
                                    "SIGNIFICANCE",
                                    "CELL_SIGNIFICANCE",
                                    "CELL_SIG_SAMPLING",
                                    "AVG_LAR_Q",
                                    "AVG_TILE_Q",
                                    "PTD",
                                    "MASS",
                                    "SECOND_TIME",
                                    "NCELL_SAMPLING" ]
    flags.addFlag('MomentsToCalculate',MomentsToCalculate)
    flags.addFlag('MomentsMinRLateral',4*cm)
    flags.addFlag('MomentsMinLLongitudinal',10*cm)
    flags.addFlag('OutputCountsToFile',False)
    flags.addFlag('OutputClustersToFile',True)
    flags.addFlag('DoMonitoring',False)
    flags.addFlag('NumPreAllocatedDataHolders',1) # to avoid crashes
    #If True, use the original criteria
    #(which disagree with the GPU implementation)
    flags.addFlag('UseOriginalCriteria',False)
    return flags

def configFlagsCaloRecGPU(flags,categoryFlags,cellsName="AllCalo",ClustersOutputName="Clusters"):
    categoryFlags.CellsName=cellsName
    categoryFlags.ClustersOutputName=ClustersOutputName
    if (flags.hasFlag('Concurrency.NumThreads')):
        categoryFlags.NumPreAllocatedDataHolders = flags.Concurrency.NumThreads
    if ( categoryFlags.NumPreAllocatedDataHolders < 1 ):
       categoryFlags.NumPreAllocatedDataHolders=1
    if (flags.hasFlag('Calo.TopoCluster.doTwoGaussianNoise')):
        categoryFlags.TwoGaussianNoise = flags.Calo.TopoCluster.doTwoGaussianNoise
    if (flags.hasCategory('flags.Calo.TopoCluster')):
        categoryFlags.SeedCutsInT = flags.Calo.TopoCluster.doTimeCut
        categoryFlags.CutOOTseed = flags.Calo.TopoCluster.extendTimeCut and flags.Calo.TopoCluster.doTimeCut
        categoryFlags.UseTimeCutUpperLimit = flags.Calo.TopoCluster.useUpperLimitForTimeCut
    if (flags.hasCategory('flags.Calo.TopoCluster')):
         categoryFlags.SplitterUseNegativeClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
         categoryFlags.UseAbsEnergyMoments = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
    if (flags.hasCategory('flags.Common')):
        if not flags.Common.isOnline:
            categoryFlags.MomentsToCalculate += ["ENG_BAD_HV_CELLS","N_BAD_HV_CELLS"]
