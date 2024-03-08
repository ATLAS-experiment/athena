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
    flags.addFlagsCategory('CaloRecGPU.GlobalFlags', _createGlobalFlagsCaloRecGPU, prefix=True)
    flags.addFlagsCategory('CaloRecGPU.Default', _createDefaultSubFlagsCaloRecGPU, prefix=True)
    flags.addFlagsCategory('Trigger.CaloRecGPU.Default', _createTriggerSubFlagsCaloRecGPU, prefix=True)
    #flags.addFlagsCategory('Trigger.CaloRecGPU.LC', _createTriggerSubFlagsCaloRecGPUWithLC, prefix=True)
    flags.addFlagsCategory('CaloRecGPU.ActiveConfig', _createActiveConfigFlagsCaloRecGPU, prefix=True)
    return flags
    
def _createGlobalFlagsCaloRecGPU():
    """
    Generate top-level flags for CaloRecGPU that are meant to be global
    and independent of the ActiveConfig.
    """
    flags = AthConfigFlags()
    
    flags.addFlag('UseCaloRecGPU', False)
    flags.addFlag('UseCPUToolsInstead', False)
    
    return flags
    
def _createActiveConfigFlagsCaloRecGPU():
    """
    Generate top-level flags for CaloRecGPU that are meant to be global
    and independent of the ActiveConfig.
    """
    flags = AthConfigFlags()
    
    flags.addFlag('MeasureTimes', False)
    
    return flags

def _createTriggerSubFlagsCaloRecGPU():
    """
    Generate CaloRecGPU flags for a particular flag subdomain (the trigger in this case)
 
    Calls the function to generate a new set of default CaloRecGPU flags, and then updates the defaults as required for this specific subdomain.
    """
    flags = _createDefaultSubFlagsCaloRecGPU()
    flags.UseOriginalCriteria = False
    flags.GrowingRestrictPSNeighbors = False
    flags.GPUSplittingRestrictPSNeighbors = False
    flags.doTimeCut = lambda prevFlags: prevFlags.Trigger.Calo.TopoCluster.doTimeCut
    flags.extendTimeCut = lambda prevFlags: prevFlags.Trigger.Calo.TopoCluster.extendTimeCut
    flags.useUpperLimitForTimeCut = lambda prevFlags: prevFlags.Trigger.Calo.TopoCluster.useUpperLimitForTimeCut
    flags.timeCutUpperLimit = lambda prevFlags: prevFlags.Trigger.Calo.TopoCluster.timeCutUpperLimit
    flags.MomentsToCalculate = ['FIRST_PHI',
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
    flags.MomentsUseAbsEnergy = False
    flags.doTopoClusterLocalCalib = False
    return flags


#def _createTriggerSubFlagsCaloRecGPUWithLC():
#    flags = _createTriggerSubFlagsCaloRecGPU()
#    flags.doTopoClusterLocalCalib = True
#    return flags

def _createDefaultSubFlagsCaloRecGPU():
    """
    Generate a new default CaloRecGPU flags domain
 
    Generates a full suite of CaloRecGPU flags for a specific subdomain, the prefixing of the subdomain is handled by the caller.
    Sets the most generic default parameters or default parameter lambda function logic for each flag, this can be overridden if needed by specific subdomains. 
    """
    flags = AthConfigFlags()
    
    #GPU SPECIFIC FLAGS WITH NO CPU EQUIVALENT
    
    flags.addFlag('MeasureTimes', False)
    #Output timing measurements,
    #including the breakdown
    #within the algorithms themselves.
    
    flags.addFlag('FillMissingCells', False)
    flags.addFlag('MissingCellsToFill', [])
    
    flags.addFlag('NumPreAllocatedDataHolders', 0)
    
    flags.addFlag('UseOriginalCriteria', lambda prevFlags: not prevFlags.Calo.TopoCluster.UseGPUCompatibleCriteria)
    #Restore instantiated CPU tools to their original, GPU-disagreeing criteria.
    
    #FLAGS THAT ENCAPSULATE CPU CONFIGURATION OPTIONS THAT ARE NOT FLAGS IN CPU CONFIGURATION
    #(Most of them aren't strictly necessary and could be hard-coded in the Cfg,
    # but CPU vs GPU testing benefits from having all this granularity.)
    
    flags.addFlag('SeedThreshold',4.0)
    flags.addFlag('GrowThreshold',2.0)
    flags.addFlag('TermThreshold',0.0)
    flags.addFlag('UseAbsSeedThreshold',True)
    flags.addFlag('UseAbsGrowThreshold',True)
    flags.addFlag('UseAbsTermThreshold',True)
    flags.addFlag('ClusterSize', 'Topo_420')
    #There is the "getClusterSize" function on the C++ code
    #that tries to guess based on the thresholds,
    #we could either rely on that or do it also in Python?
    
    flags.addFlag('GrowingCalorimeterNames', ["LAREM", "LARHEC", "LARFCAL", "TILE"])
    flags.addFlag('GrowingSeedSamplingNames', ["PreSamplerB", "EMB1", "EMB2", "EMB3", "PreSamplerE", "EME1", "EME2", "EME3", "HEC0", "HEC1","HEC2", "HEC3", "TileBar0", "TileBar1", "TileBar2", "TileExt0", "TileExt1", "TileExt2", "TileGap1", "TileGap2", "TileGap3", "FCAL0", "FCAL1", "FCAL2"])
    flags.addFlag('GrowingNeighborOption',"super3D")
    flags.addFlag('GrowingRestrictHECIWandFCalNeighbors',False)
    flags.addFlag('GrowingRestrictPSNeighbors',True)
    flags.addFlag('GrowingTreatL1PredictedCellsAsGood',True)
    
    flags.addFlag('GrowingTimeCutSeedThreshold',12.5*ns)
    
    flags.addFlag('PostGrowingClusterCutClustersInAbsEt', True)
    flags.addFlag('PostGrowingClusterEnergyCut', -1e-16*MeV)
    #CPU has default 0, but since the comparison is not inclusive,
    #it's better if we consider a negative value so that both CPU and GPU
    #agree 100% when doing the comparison in absolute energy,
    #because otherwise floating point making clusters have zero energy 
    #in one implementation and a tiny non-zero energy on another
    #would mean different cuts, thus different cluster numbers,
    #thus different splitting, and so on.
    #This problem will remain for non-zero energy cuts,
    #like other floating point agreement issues elsewhere.
    
    
    flags.addFlag('SplittingNeighborOption',"super3D")
    flags.addFlag('SplittingRestrictHECIWandFCalNeighbors',False)
    flags.addFlag('SplittingTreatL1PredictedCellsAsGood',True)
    
    flags.addFlag('SplittingNumberOfCellsCut', 4)
    flags.addFlag('SplittingEnergyCut', 500 * MeV)
    flags.addFlag('SplittingSamplingNames',["EMB2", "EMB3", "EME2", "EME3", "FCAL0"])
    flags.addFlag('SplittingSecondarySamplingNames',["EMB1","EME1", "TileBar0","TileBar1","TileBar2", "TileExt0","TileExt1","TileExt2", "HEC0","HEC1","HEC2","HEC3", "FCAL1","FCAL2"])
    flags.addFlag('SplittingShareBorderCells',True)
    flags.addFlag('SplittingEMShowerScale',5.0*cm)
    flags.addFlag('SplittingUseNegativeClusters', lambda prevFlags: prevFlags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute)
    
    flags.addFlag('GPUSplittingRestrictPSNeighbors',False)
    #The CPU code does not apply this restriction, mostly for testing purposes
    
    flags.addFlag('MomentsUseAbsEnergy', lambda prevFlags: prevFlags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute)
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
    
    
    #FLAGS THAT MIRROR CPU OPTIONS
    
    flags.addFlag('doTwoGaussianNoise', lambda prevFlags: prevFlags.Calo.TopoCluster.doTwoGaussianNoise)
    
    
    flags.addFlag('doTimeCut', lambda prevFlags: prevFlags.Calo.TopoCluster.doTimeCut)
    flags.addFlag('extendTimeCut', lambda prevFlags: prevFlags.Calo.TopoCluster.extendTimeCut)
    
    flags.addFlag('useUpperLimitForTimeCut', lambda prevFlags: prevFlags.Calo.TopoCluster.useUpperLimitForTimeCut)
    
    flags.addFlag('timeCutUpperLimit', lambda prevFlags: prevFlags.Calo.TopoCluster.timeCutUpperLimit)
    
    flags.addFlag('xtalkEM2', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalkEM2)
    flags.addFlag('xtalkEM2D', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalkEM2D)
    flags.addFlag('xtalkEM2n', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalkEM2n)
    flags.addFlag('xtalkEM3', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalkEM3)
    flags.addFlag('xtalkEMEta', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalkEMEta)
    flags.addFlag('xtalkDeltaT', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalkDeltaT)
    flags.addFlag('xtalk2Eratio1', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalk2Eratio1)
    flags.addFlag('xtalk2Eratio2', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalk2Eratio2)
    flags.addFlag('xtalk3Eratio', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalk3Eratio)
    flags.addFlag('xtalkEtaEratio', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalkEtaEratio)
    flags.addFlag('xtalk2DEratio', lambda prevFlags: prevFlags.Calo.TopoCluster.xtalk2DEratio)
    
    flags.addFlag('doTopoClusterLocalCalib', lambda prevFlags: prevFlags.Calo.TopoCluster.doTopoClusterLocalCalib)
    flags.addFlag('doCalibHitMoments', lambda prevFlags: prevFlags.Calo.TopoCluster.doCalibHitMoments)
    
    flags.addFlag('skipWriteList', lambda prevFlags: prevFlags.Calo.TopoCluster.skipWriteList)
    flags.addFlag('writeExtendedClusterMoments', lambda prevFlags: prevFlags.Calo.TopoCluster.writeExtendedClusterMoments)
    flags.addFlag('writeCalibHitClusterMoments', lambda prevFlags: prevFlags.Calo.TopoCluster.writeCalibHitClusterMoments)
    
    flags.addFlag('addCalibrationHitDecoration', lambda prevFlags: prevFlags.Calo.TopoCluster.addCalibrationHitDecoration)
    flags.addFlag('CalibrationHitDecorationName', lambda prevFlags: prevFlags.Calo.TopoCluster.CalibrationHitDecorationName)
    flags.addFlag('addCPData', lambda prevFlags: prevFlags.Calo.TopoCluster.addCPData)
    
    return flags
