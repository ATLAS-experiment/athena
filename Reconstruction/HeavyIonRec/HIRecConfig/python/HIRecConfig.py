# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
__log = logging.getLogger('HIRecConfig')


def HIRecCfg(flags):
    '''Configures entire HI reconstruction'''
    acc = ComponentAccumulator()
    
    
    if flags.HeavyIon.doGlobal:
        from HIGlobal.HIGlobalConfig import HIGlobalRecCfg
        acc.merge(HIGlobalRecCfg(flags))

    if flags.HeavyIon.doJet:
        from HIJetRec.HIJetRecConfigCA import HIJetRecCfg
        acc.merge(HIJetRecCfg(flags))
        if flags.Input.isMC:
            from JetRecConfig.JetRecConfig import JetRecCfg
            from JetRecConfig.JetRecoSteering import addJetsToOutputCfg
            from JetRecConfig.StandardSmallRJets import AntiKt4Truth, AntiKt2Truth
            jetdefs = [AntiKt4Truth,AntiKt2Truth]
            for jd in jetdefs:
                acc.merge(JetRecCfg(flags, jd))
            acc.merge(addJetsToOutputCfg(flags, jetdefs, toAOD=True, toESD=False))

    if flags.HeavyIon.Egamma.doSubtractedClusters:
        from HIJetRec.HIEgammaRecConfigCA import (
            HIEgammaRecCfg)
        acc.merge(HIEgammaRecCfg(flags))

    return acc



if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    flags = initConfigFlags()

# testing for Run 2:
    # flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data"]
    # flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-RUN2-09"
    # flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2

# testing for Run 3:
    flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data22_hi/RAWFiles/data22_hi.00440101.physics_MinBias.daq.RAW/data22_hi.00440101.physics_MinBias.daq.RAW._lb0214._SFO-11._0001.data"]
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2022-09" 
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

# set more flags:
    flags.Exec.MaxEvents=5
    flags.Exec.SkipEvents=0
    flags.Concurrency.NumThreads=1
    flags.Output.doWriteAOD = True
    flags.Output.AODFileName = "myAOD.pool.root"
    flags.Output.doWriteESD = True
    flags.Output.ESDFileName = "myESD.pool.root"
    
    flags.Trigger.triggerConfig = "DB"

    from HIRecConfig.HIModeFlags import HImode
    HImode(flags)
    
# enables unit tests to switch only parts of reco such as (note the absence of spaces around equal signs): 
### python -m HIRecConfig.HIRecConfig HeavyIon.doGlobal="False" GeoModel.AtlasVersion="ATLAS-R3S-2021-03-01-00"
    flags.fillFromArgs() 
    
    flags.lock()

    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
    if flags.Reco.EnableBeamSpotDecoration:
        from xAODEventInfoCnv.EventInfoBeamSpotDecoratorAlgConfig import EventInfoBeamSpotDecoratorAlgCfg
        acc.merge(EventInfoBeamSpotDecoratorAlgCfg(flags))
    if flags.HeavyIon.redoTracking:
        from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
        acc.merge(InDetTrackRecoCfg(flags))
    if flags.HeavyIon.redoEgamma:
        from egammaConfig.egammaSteeringConfig import EGammaSteeringCfg
        acc.merge(EGammaSteeringCfg(flags))
    
    ## To avoid error coming from jet modifiers
    from JetRecConfig.JetConfigFlags import jetInternalFlags
    jetInternalFlags.isRecoJob = True

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo
    EMjet = AntiKt4EMTopo.clone()
    EMjet.ghostdefs = ["Track"]
    acc.merge(JetRecCfg(flags,EMjet))

    acc.merge(HIRecCfg(flags))
    

    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    acc.merge(PoolWriteCfg(flags))
    
# debug output level can be switched for particular algorithms:
    # from AthenaCommon.Constants import DEBUG
    # acc.foreach_component("*HISubtractedCellMakerTool*").OutputLevel=DEBUG
    # acc.foreach_component("*HICellCopyTool*").OutputLevel=DEBUG

    acc.printConfig(withDetails=True, summariseProps=True)
    
    flags.dump()
    

    import sys
    sys.exit(acc.run().isFailure())
