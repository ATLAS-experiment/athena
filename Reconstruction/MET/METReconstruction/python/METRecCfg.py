# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Configure all MET algorithms for standard reconstruction
def METCfg(inputFlags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result=ComponentAccumulator()
    outputList = []

    # Calo MET collections (for monitoring)
    from METReconstruction.METCalo_Cfg import METCalo_Cfg
    result.merge(METCalo_Cfg(inputFlags))
    metDefs = ['EMTopo', 'EMTopoRegions', 'LocHadTopo', 'LocHadTopoRegions', 'Calo']
    for metDef in metDefs:
        outputList.append('xAOD::MissingETContainer#MET_'+metDef)
        outputList.append('xAOD::MissingETAuxContainer#MET_'+metDef+'Aux.')

    # Track MET
    from METReconstruction.METTrack_Cfg import METTrack_Cfg
    result.merge(METTrack_Cfg(inputFlags))
    outputList.append("xAOD::MissingETContainer#MET_Track")
    outputList.append("xAOD::MissingETAuxContainer#MET_TrackAux.")

    # Truth MET
    if inputFlags.Input.isMC:
        from METReconstruction.METTruth_Cfg import METTruth_Cfg
        result.merge(METTruth_Cfg(inputFlags))
        outputList.append("xAOD::MissingETContainer#MET_Truth")
        outputList.append("xAOD::MissingETAuxContainer#MET_TruthAux.")
        outputList.append("xAOD::MissingETContainer#MET_TruthRegions")
        outputList.append("xAOD::MissingETAuxContainer#MET_TruthRegionsAux.")
        outputList.append('xAOD::MissingETComponentMap#METMap_Truth')
        outputList.append('xAOD::MissingETAuxComponentMap#METMap_TruthAux.')

    # "Normal" MET collections
    from METReconstruction.METAssociatorCfg import METAssociatorCfg
    from METUtilities.METMakerConfig import getMETMakerAlg
    metDefs = ['AntiKt4EMTopo','AntiKt4LCTopo']
    if inputFlags.MET.DoPFlow:
       metDefs.append('AntiKt4EMPFlow')
    for metDef in metDefs:
        # Build association maps and core soft terms
        result.merge(METAssociatorCfg(inputFlags, metDef))
        outputList.append('xAOD::MissingETAssociationMap#METAssoc_'+metDef)
        outputList.append('xAOD::MissingETAuxAssociationMap#METAssoc_'+metDef+'Aux.')
        outputList.append('xAOD::MissingETContainer#MET_Core_'+metDef)
        outputList.append('xAOD::MissingETAuxContainer#MET_Core_'+metDef+'Aux.')
        # Build reference MET
        result.addEventAlgo(getMETMakerAlg(metDef))
        outputList.append('xAOD::MissingETContainer#MET_Reference_'+metDef)
        outputList.append('xAOD::MissingETAuxContainer#MET_Reference_'+metDef+'Aux.-ConstitObjectLinks.-ConstitObjectWeights')

    # Add the collections to the output stream
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
    result.merge(addToESD(inputFlags, outputList))
    if inputFlags.MET.WritetoAOD:
        result.merge(addToAOD(inputFlags, outputList))

    return result

# Run with python -m METReconstruction.METRecCfg
def METRecCfgTest(flags=None):

    if flags is None:
        # Set message levels
        from AthenaCommon.Logging import log
        from AthenaCommon.Constants import DEBUG
        log.setLevel(DEBUG)

        # Config flags steer the job at various levels
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
        flags.Input.Files = defaultTestFiles.ESD_RUN3_MC
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC

        flags.fillFromArgs()
        flags.lock()

    # Get a ComponentAccumulator setting up the fundamental Athena job
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Add the components for reading in pool files
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Setup calorimeter geometry, which is needed for jet reconstruction
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))

    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

    from JetRecConfig.JetRecoSteering import JetRecoSteeringCfg
    cfg.merge(JetRecoSteeringCfg(flags))

    # We also need to build links between the newly
    # created jet constituents (GlobalFE)
    # and electrons,photons,muons and taus
    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
    cfg.merge(PFGlobalFlowElementLinkingCfg(flags))

    cfg.merge(METCfg(flags))

    cfg.run()

if __name__=="__main__":
    METRecCfgTest()
