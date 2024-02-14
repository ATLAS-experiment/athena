# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaCommon.SystemOfUnits import MeV, ns, cm, deg

def createFlagsCaloRecGPU():
    """
    Top level flag generator for CaloRecGPU
 
    The list of available CaloRecGPU flag subdomains is populated below the first time flags.CaloRecGPU is called to either set or get any sub-flag.

    The central hook to this comes from Control/AthenaConfiguration/python/AllConfigFlags.py

    The CaloRecGPU package must be compiled otherwise this will silently fail.

    All CaloRecGPU flag subdomains should be listed below. They will be similarly be populated only on-demand.
    """

    flags = AthConfigFlags()
    flags.addFlagsCategory('CaloRecGPU.Default', _createDefaultSubFlagsCaloRecGPU, prefix=True)
    flags.addFlagsCategory('CaloRecGPU.LocalCalibration', _createLCSubFlagsCaloRec, prefix=True)
    flags.addFlagsCategory('CaloRecGPU.HLT', _createHLTSubFlagsCaloRec, prefix=True)
    return flags

def _createLCSubFlagsCaloRec():
    """
    Generate CaloRecGPU flags for a particular flag subdomain
 
    Calls the function to generate a new set of default CaloRecGPU flags, and then updates the defaults as required for this specific subdomain "LocalCalibration".
    This _createLCSubFlagsCaloRec is provided as an example of a subdomain with an updated default value for the ClustersOutputName flag. 
    """
    flags = _createDefaultSubFlagsCaloRecGPU()
    flags.ClustersOutputName = "CaloTopoClusters"
    return flags 

def _createHLTSubFlagsCaloRec():
    flags = _createDefaultSubFlagsCaloRecGPU()
    flags.CellsName = "CaloCellsFS"
    flags.ClustersOutputName = "HLT_TopoCaloClustersFS"
    return flags

def _createDefaultSubFlagsCaloRecGPU():
    """
    Generate a new default CaloRecGPU flags domain
 
    Generates a full suite of CaloRecGPU flags for a specific subdomain, the prefixing of the subdomain is handled by the caller.
    Sets the most generic default parameters or default parameter lambda function logic for each flag, this can be overridden if needed by specific subdomains. 
    """
    flags = AthConfigFlags()
    flags.addFlag('MeasureTimes', True)
    flags.addFlag('CellsName', "AllCalo")
    flags.addFlag('ClustersOutputName', "CaloCalTopoClusters")
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
    flags.addFlag('TwoGaussianNoise', lambda prevFlags: prevFlags.Calo.TopoCluster.doTwoGaussianNoise)
    flags.addFlag('SeedCutsInT', lambda prevFlags: prevFlags.Calo.TopoCluster.doTimeCut)
    flags.addFlag('CutOOTseed', lambda prevFlags: prevFlags.Calo.TopoCluster.extendTimeCut and prevFlags.Calo.TopoCluster.doTimeCut)
    flags.addFlag('UseTimeCutUpperLimit', lambda prevFlags: prevFlags.Calo.TopoCluster.useUpperLimitForTimeCut)
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
    flags.addFlag('SplitterUseNegativeClusters', lambda prevFlags: prevFlags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute)
    flags.addFlag('UseAbsEnergyMoments', lambda prevFlags: prevFlags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute)
    flags.addFlag('MomentsMaxAxisAngle',20*deg)
    flags.addFlag('MomentsMinBadLArQuality',4000)
    MomentsToCalculateOnline=[ "FIRST_PHI",
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
    MomentsToCalculateOffline = MomentsToCalculateOnline + ["ENG_BAD_HV_CELLS","N_BAD_HV_CELLS"]
    flags.addFlag('MomentsToCalculate', lambda prevFlags: MomentsToCalculateOnline if prevFlags.Common.isOnline else MomentsToCalculateOffline )
    flags.addFlag('MomentsMinRLateral',4*cm)
    flags.addFlag('MomentsMinLLongitudinal',10*cm)
    flags.addFlag('OutputCountsToFile',False)
    flags.addFlag('OutputClustersToFile',True)
    flags.addFlag('DoMonitoring',False)
    flags.addFlag('NumPreAllocatedDataHolders', lambda prevFlags: max(prevFlags.Concurrency.NumThreads, 1)) # Avoids 0 when running serial athena
    #If True, use the original criteria
    #(which disagree with the GPU implementation)
    flags.addFlag('UseOriginalCriteria',False)
    return flags
