# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger

# I'm not sure if we should import from here or vice versa.
# For now though import from here to avoid duplication of code.
from FPGATrackSimConfTools import FPGATrackSimAnalysisConfig

log = AthenaLogger(__name__)

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    
    ############################################
    # Flags used in the prototrack chain
    FinalProtoTrackChainxAODTracksKey="xAODFPGAProtoTracks"
    flags.Detector.EnableCalo = False

    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True

    flags.Acts.doRotCorrection = False

    # IDPVM flags
    flags.PhysVal.IDPVM.doExpertOutput   = True
    flags.PhysVal.IDPVM.doPhysValOutput  = True
    flags.PhysVal.IDPVM.doHitLevelPlots = True
    flags.PhysVal.IDPVM.runDecoration = True
    flags.PhysVal.IDPVM.validateExtraTrackCollections = [f"{FinalProtoTrackChainxAODTracksKey}TrackParticles"]
    flags.PhysVal.IDPVM.doTechnicalEfficiency = False # should figure out if 'True' is needed and what's missing to enable it
    flags.PhysVal.OutputFileName = "IDPVM.root"
    ############################################
    flags.Concurrency.NumThreads=1
    flags.Scheduler.ShowDataDeps=True
    # flags.Exec.DebugStage="exec" # useful option to debug the execution of the job - we want it commented out for production
    flags.fillFromArgs()
    if isinstance(flags.Trigger.FPGATrackSim.wrapperFileName, str):
        log.info("wrapperFile is string, converting to list")
        flags.Trigger.FPGATrackSim.wrapperFileName = [flags.Trigger.FPGATrackSim.wrapperFileName]
        flags.Input.Files = lambda f: [f.Trigger.FPGATrackSim.wrapperFileName]

    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")
    acc=MainServicesCfg(flags)

    acc.addService(CompFactory.THistSvc(Output = ["EXPERT DATAFILE='monitoring.root', OPT='RECREATE'"]))
    acc.addService(CompFactory.THistSvc(Output = ["MONITOROUT DATAFILE='dataflow.root', OPT='RECREATE'"]))

    if not flags.Trigger.FPGATrackSim.wrapperFileName:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))
    
        if flags.Input.isMC:
            from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
            acc.merge(GEN_AOD2xAODCfg(flags))

            from JetRecConfig.JetRecoSteering import addTruthPileupJetsToOutputCfg # TO DO: check if this is indeed necessary for pileup samples
            acc.merge(addTruthPileupJetsToOutputCfg(flags))
        
        if flags.Detector.EnableCalo:
            from CaloRec.CaloRecoConfig import CaloRecoCfg
            acc.merge(CaloRecoCfg(flags))

        if not flags.Reco.EnableTrackOverlay:
            from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
            acc.merge(InDetTrackRecoCfg(flags))

    # Use the imported configuration function for the data prep algorithm.
    acc.merge(FPGATrackSimAnalysisConfig.FPGATrackSimDataPrepAlgCfg(flags))
    acc.store(open('DataPrepConfig.pkl','wb'))
    
    statusCode = acc.run(flags.Exec.MaxEvents)
    sys.exit(not statusCode.isSuccess())
