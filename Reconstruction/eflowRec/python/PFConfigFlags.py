# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createPFConfigFlags():
    pfConfigFlags=AthConfigFlags()

    pfConfigFlags.addFlag("PF.EOverPMode",False) #This defines whether we use the standard reconstruction or dedicated e/p mode used to measure e/p reference values
    pfConfigFlags.addFlag("PF.addClusterMoments",True) #This defines whether we add the cluster moments to neutral PFO
    pfConfigFlags.addFlag("PF.useClusterMoments",True) #This defines whether we add a full list of cluster moments or not. Currently only the HLT config makes use of this flag, it is not used for offline config.
    pfConfigFlags.addFlag("PF.useCalibHitTruthClusterMoments",False) #This defines whether we calculate the calibration hit moments - only possible on if running from special calibraiton hit ESD samples.
    pfConfigFlags.addFlag("PF.useElPhotLinks", lambda prevFlags : prevFlags.Reco.EnableEgamma)
    pfConfigFlags.addFlag("PF.useMuLinks", lambda prevFlags : prevFlags.Reco.EnableCombinedMuon)
    pfConfigFlags.addFlag("PF.useMLEOverP",False) #Toggle whether to use the Machine Learning based EOverP inference or not
    pfConfigFlags.addFlag("PF.EOverP_NN_Model",'/afs/cern.ch/user/m/mhodgkin/onnx_15_03_23.onnx') #Model to use in EOverP inference
    #Reference location for cell ordering and e/p lookup in particle flow. 
    pfConfigFlags.addFlag("PF.EOverP_CellOrdering_ReferenceLocation",'eflowRec/PFCellEOverPTool/Run4/v4/')
    pfConfigFlags.addFlag("PF.addCPData",False)
    #Toggle whether to use truth information to cheat the reconstruction or not - only makes sense to do this in MC samples with calibration hits and without pileup
    #Toggle whether in general we should use truth calibraiton hit information to cheat at reconstructing things - only makes sense to do this in MC samples with calibration hits and without pileup
    pfConfigFlags.addFlag("PF.useTruthCheating",False)
    #These flags decide which aspects of hte algorithm would use truth cheating
    #Toggle whether to use truth information to cheat the reconstruction or not
    pfConfigFlags.addFlag("PF.useTrackClusterTruthMatching",False)
    pfConfigFlags.addFlag("PF.useTruthForChargedShowerSubtraction",False)

    #Toggle whether to use the legacy EOverP (eflowCellEOverPTool_mc12_HLLHC.h) or not. Off by default so we use the new reference - this toggle is so we can compare old/new in the production system
    #before eventually removing the old tool entirely.
    pfConfigFlags.addFlag("PF.useLegacyEOverPRun4",False)

    #Toggle whether to use topoclusters or combined topoclusters + topotowers container
    pfConfigFlags.addFlag("PF.useTopoTowers",False)

    #Toggle thinning on and off
    pfConfigFlags.addFlag("PF.doThinning",True)

    return pfConfigFlags
