# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def getStreamHITS_ItemList(flags):
    #--------------------------------------------------------------
    # Specify collections for output HIT files, as not all are required.
    #--------------------------------------------------------------
    ItemList = []

    if "McEventCollection#TruthEvent" in flags.Input.TypedCollections:
        ItemList += ["McEventCollection#TruthEvent"] # mc truth (hepmc)

    if "TrackRecordCollection#MuonEntryLayer" in flags.Input.TypedCollections:
        ItemList += ["TrackRecordCollection#MuonEntryLayer"] # others not used in pileup

    if "xAOD::JetContainer#AntiKt4TruthJets" in flags.Input.TypedCollections:
        ItemList += ["xAOD::JetContainer#AntiKt4TruthJets",
                     "xAOD::AuxContainerBase!#AntiKt4TruthJetsAux.-constituentLinks.-constituentWeights"]

    if "xAOD::JetContainer#AntiKt6TruthJets" in flags.Input.TypedCollections:
        ItemList += ["xAOD::JetContainer#AntiKt6TruthJets",
                     "xAOD::AuxContainerBase!#AntiKt6TruthJetsAux.-constituentLinks.-constituentWeights"]

    if "xAOD::TruthParticleContainer#TruthPileupParticles" in flags.Input.TypedCollections:
        ItemList += ["xAOD::TruthParticleContainer#TruthPileupParticles",
                     "xAOD::TruthParticleAuxContainer#TruthPileupParticlesAux."]

    if "xAOD::EventInfo#EventInfo" in flags.Input.TypedCollections:
        ItemList += ["xAOD::EventInfo#EventInfo",
                     "xAOD::EventAuxInfo#EventInfoAux.",
                     "xAOD::EventInfoContainer#*",
                     "xAOD::EventInfoAuxContainer#*"]
    else:
        ItemList += ["EventInfo#*"]

    #PLR
    if flags.Detector.EnablePLR:
        ItemList += ["SiHitCollection#PLR_Hits"]
    #BCM
    if flags.Detector.EnableBCM:
        ItemList += ["SiHitCollection#BCMHits"]
    #Pixels
    if flags.Detector.EnablePixel:
        ItemList += ["SiHitCollection#PixelHits"]
    #SCT
    if flags.Detector.EnableSCT:
        ItemList += ["SiHitCollection#SCT_Hits"]
    #TRT
    if flags.Detector.EnableTRT:
        ItemList += ["TRTUncompressedHitCollection#TRTUncompressedHits"]
    #ITk Pixels
    if flags.Detector.EnableITkPixel:
        ItemList += ["SiHitCollection#ITkPixelHits"]
    #ITk Strip
    if flags.Detector.EnableITkStrip:
        ItemList += ["SiHitCollection#ITkStripHits"]
    #LAr
    if flags.Detector.EnableLAr:
        ItemList += ["LArHitContainer#LArHitEMB"]
        ItemList += ["LArHitContainer#LArHitEMEC"]
        ItemList += ["LArHitContainer#LArHitHEC"]
        ItemList += ["LArHitContainer#LArHitFCAL"]
    #Tile
    if flags.Detector.EnableTile:
        ItemList += ["TileHitVector#TileHitVec"]
    # MBTS
    if flags.Detector.EnableMBTS:
        ItemList += ["TileHitVector#MBTSHits"]
    # HGTD
    if flags.Detector.EnableHGTD:
        ItemList += ["SiHitCollection#HGTD_Hits"]

    if flags.Detector.GeometryMuon:
        if flags.Muon.usePhaseIIGeoSetup:
            from MuonSensitiveDetectorsR4.SensitiveDetectorsCfg import OutputSimContainersCfg
            ItemList += OutputSimContainersCfg(flags)
        else:
            from MuonG4SD.MuonG4SDConfig import OutputSimContainersCfg
            ItemList += OutputSimContainersCfg(flags)
    return ItemList


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    log = logging.getLogger('FilterHit_tf')
    log.info('****************** STARTING HIT FILTERING *****************')

    log.info('**** Transformation run arguments')
    log.info(str(runArgs))

    log.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    commonRunArgsToFlags(runArgs, flags)

    if hasattr(runArgs, 'inputHITSFile'):
        flags.Input.Files = runArgs.inputHITSFile
    else:
        raise RuntimeError('No input HITS file defined')

    # Generate detector list and setup detector flags
    from SimuJobTransforms.SimulationHelpers import getDetectorsFromRunArgs
    detectors = getDetectorsFromRunArgs(flags, runArgs)
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, use_metadata=True, toggle_geometry=True, keep_beampipe=True)

    if hasattr(runArgs, 'outputHITS_FILTFile'):
        if runArgs.outputHITS_FILTFile == 'None':
            flags.Output.HITSFileName = ''
            # TODO decide if we need a specific HITS_FILTFileName flag
        else:
            flags.Output.HITSFileName  = runArgs.outputHITS_FILTFile
    else:
        raise RuntimeError('No outputHITS_FILTFile defined')

    # force TreeAutoFlush=1 as events will be accessed randomly
    flags.Output.TreeAutoFlush = {'HITS': 1}

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Identifiers
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    cfg.merge(DetDescrCnvSvcCfg(flags))

    # add LArHitFilter + AddressRemappingSvc
    if flags.Detector.EnableLAr:
        from LArHitAlgs.LArHitAlgsConfig import LArHitFilterCfg
        cfg.merge(LArHitFilterCfg(flags)) # TODO add output configuration here?

    if hasattr(runArgs, 'TruthReductionScheme'):
        if runArgs.TruthReductionScheme != 'SingleGenParticle':
            log.warning(f'Unknown TruthReductionScheme ({runArgs.TruthReductionScheme}). Currently just a dummy value, but please check.')

        # add McEventCollectionFtiler + AddressRemappingSvc
        from McEventCollectionFilter.McEventCollectionFilterConfig import McEventCollectionFilterCfg
        cfg.merge(McEventCollectionFilterCfg(flags))
        # Check for Truth Containers
        for entry in flags.Input.Collections:
            if 'AntiKt4TruthJets' == entry:
                from McEventCollectionFilter.McEventCollectionFilterConfig import DecoratePileupAntiKt4TruthJetsCfg
                cfg.merge(DecoratePileupAntiKt4TruthJetsCfg(flags))
            if 'AntiKt6TruthJets' == entry:
                from McEventCollectionFilter.McEventCollectionFilterConfig import DecoratePileupAntiKt6TruthJetsCfg
                cfg.merge(DecoratePileupAntiKt6TruthJetsCfg(flags))
            if 'TruthPileupParticles' == entry:
                from McEventCollectionFilter.McEventCollectionFilterConfig import DecorateTruthPileupParticlesCfg
                cfg.merge(DecorateTruthPileupParticlesCfg(flags))

        # ID
        if flags.Detector.EnableBCM:
            from McEventCollectionFilter.McEventCollectionFilterConfig import BCM_HitsTruthRelinkCfg
            cfg.merge(BCM_HitsTruthRelinkCfg(flags))
        if flags.Detector.EnablePixel:
            from McEventCollectionFilter.McEventCollectionFilterConfig import PixelHitsTruthRelinkCfg
            cfg.merge(PixelHitsTruthRelinkCfg(flags))
        if flags.Detector.EnableSCT:
            from McEventCollectionFilter.McEventCollectionFilterConfig import SCT_HitsTruthRelinkCfg
            cfg.merge(SCT_HitsTruthRelinkCfg(flags))
        if flags.Detector.EnableTRT:
            from McEventCollectionFilter.McEventCollectionFilterConfig import TRT_HitsTruthRelinkCfg
            cfg.merge(TRT_HitsTruthRelinkCfg(flags))
        # ITk
        if flags.Detector.EnableITkPixel:
            from McEventCollectionFilter.McEventCollectionFilterConfig import ITkPixelHitsTruthRelinkCfg
            cfg.merge(ITkPixelHitsTruthRelinkCfg(flags))
        if flags.Detector.EnableITkPixel:
            from McEventCollectionFilter.McEventCollectionFilterConfig import ITkPixelHitsTruthRelinkCfg
            cfg.merge(ITkPixelHitsTruthRelinkCfg(flags))
        if flags.Detector.EnablePLR:
            from McEventCollectionFilter.McEventCollectionFilterConfig import PLR_HitsTruthRelinkCfg
            cfg.merge(PLR_HitsTruthRelinkCfg(flags))
        # HGTD
        if flags.Detector.EnableHGTD:
            from McEventCollectionFilter.McEventCollectionFilterConfig import HGTD_HitsTruthRelinkCfg
            cfg.merge(HGTD_HitsTruthRelinkCfg(flags))
        # Muons
        if flags.Detector.EnableCSC:
            from McEventCollectionFilter.McEventCollectionFilterConfig import CSC_HitsTruthRelinkCfg
            cfg.merge(CSC_HitsTruthRelinkCfg(flags))
        if flags.Detector.EnableMDT:
            from McEventCollectionFilter.McEventCollectionFilterConfig import MDT_HitsTruthRelinkCfg
            cfg.merge(MDT_HitsTruthRelinkCfg(flags))
        if flags.Detector.EnableMM:
            from McEventCollectionFilter.McEventCollectionFilterConfig import MM_HitsTruthRelinkCfg
            cfg.merge(MM_HitsTruthRelinkCfg(flags))
        if flags.Detector.EnableRPC:
            from McEventCollectionFilter.McEventCollectionFilterConfig import RPC_HitsTruthRelinkCfg
            cfg.merge(RPC_HitsTruthRelinkCfg(flags))
        if flags.Detector.EnableTGC:
            from McEventCollectionFilter.McEventCollectionFilterConfig import TGC_HitsTruthRelinkCfg
            cfg.merge(TGC_HitsTruthRelinkCfg(flags))
        if flags.Detector.EnablesTGC:
            from McEventCollectionFilter.McEventCollectionFilterConfig import sTGC_HitsTruthRelinkCfg
            cfg.merge(sTGC_HitsTruthRelinkCfg(flags))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge( OutputStreamCfg(flags, "HITS", ItemList=getStreamHITS_ItemList(flags), disableEventTag="xAOD::EventInfo#EventInfo" not in flags.Input.TypedCollections) )

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory

    cfg.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "HITS",
            createMetadata=[
                MetadataCategory.IOVMetaData,
            ],
        )
    )

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs))

    import time
    tic = time.time()
    # Run the final accumulator
    sc = cfg.run()
    log.info("Ran FilterHit_tf in " + str(time.time()-tic) + " seconds")

    sys.exit(not sc.isSuccess())
