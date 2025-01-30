#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
from TrigEDMConfig.TriggerEDM import recordable

import math

log = logging.getLogger('HLTSeedingConfig')

_mapL1ThresholdToDecisionCollection = {
    # Full-scan
    "FSNOSEED": "HLTNav_L1FSNOSEED",
    # Muon
    "MU" : "HLTNav_L1MU",
    "PROBEMU"  : "HLTNav_L1PROBEMU",
    # Phase-1 L1Calo
    "eEM": "HLTNav_L1eEM",
    "eTAU": "HLTNav_L1eTAU",
    "jEM": "HLTNav_L1jEM",
    "jTAU": "HLTNav_L1jTAU",
    "cTAU": "HLTNav_L1cTAU",
    "jJ": "HLTNav_L1jJ",
    "jLJ": "HLTNav_L1jLJ",
    "gJ": "HLTNav_L1gJ",
    "gLJ": "HLTNav_L1gLJ",
    "PROBEeEM" : "HLTNav_L1PROBEeEM",
    "PROBEjEM" : "HLTNav_L1PROBEjEM",
    "PROBEeTAU" : "HLTNav_L1PROBEeTAU",
    "PROBEjTAU" : "HLTNav_L1PROBEjTAU",
    "PROBEcTAU" : "HLTNav_L1PROBEcTAU",
    # Run-2 L1Calo
    "EM" : "HLTNav_L1EM",
    "J"  : "HLTNav_L1J",
    "TAU": "HLTNav_L1TAU",
    "XE" : "HLTNav_L1MET",
    "XS" : "HLTNav_L1MET",
    "TE" : "HLTNav_L1MET",
    "PROBEEM"  : "HLTNav_L1PROBEEM",
    "PROBETAU" : "HLTNav_L1PROBETAU",
}

_mapL1ThresholdToRoICollection = {
    # Full-scan
    "FSNOSEED": "HLT_FSRoI",
    # Muon
    "MU" : "HLT_MURoIs",
    "PROBEMU"  : "HLT_MURoIs",
    # Phase-1 L1Calo
    "eEM": "HLT_eEMRoIs",
    "eTAU": "HLT_eTAURoIs",
    "jEM": "HLT_jEMRoIs",
    "jTAU": "HLT_jTAURoIs",
    "cTAU": "HLT_cTAURoIs",
    "jJ": "HLT_jJRoIs",
    "jLJ": "HLT_jLJRoIs",
    "gJ": "HLT_gJRoIs",
    "gLJ": "HLT_gLJRoIs",
    "PROBEeEM" : "HLT_eEMRoIs",
    "PROBEeTAU": "HLT_eTAURoIs",
    "PROBEjEM": "HLT_jEMRoIs",
    "PROBEjTAU": "HLT_jTAURoIs",
    "PROBEcTAU": "HLT_cTAURoIs",
    # Run-2 L1Calo
    "EM" : "HLT_EMRoIs",
    "J"  : "HLT_JETRoI",
    "TAU": "HLT_TAURoI",
    "XE" : "HLT_FSRoI",
    "XS" : "HLT_FSRoI",
    "TE" : "HLT_FSRoI",
    "PROBEEM"  : "HLT_EMRoIs",
    "PROBETAU" : "HLT_TAURoI",
}


def mapThresholdToL1DecisionCollection(threshold):
    """
    Translates L1 threshold  name of the DecisionsContainer name in the HLTSeeding unpacking tools
    """
    # remove actual threshold value from L1 threshold string
    for (thresholdType, l1Collection) in _mapL1ThresholdToDecisionCollection.items():
        if threshold.startswith( thresholdType ):
            return l1Collection

    log.error("Threshold \""+ threshold + "\" not mapped to any Decision collection! Available are: " + str(_mapL1ThresholdToDecisionCollection.values()))


def mapThresholdToL1RoICollection(threshold):
    """
    Translates L1 threshold  name of the RoIDescriptor name in the HLTSeeding unpacking tools
    """
    # remove actual threshold value from L1 threshold string
    for (thresholdType, l1Collection) in _mapL1ThresholdToRoICollection.items():
        if threshold.startswith( thresholdType ):
            return l1Collection

    log.error("Threshold \""+ threshold + "\" not mapped to any ROI collection! Available are: " + str(_mapL1ThresholdToRoICollection.values()))


def createLegacyCaloRoIUnpackers(flags):
    from HLTSeeding.HLTSeedingMonitoring import RoIsUnpackingMonitoring
    emUnpacker = CompFactory.EMRoIsUnpackingTool(Decisions = mapThresholdToL1DecisionCollection("EM"),
                                                 DecisionsProbe = mapThresholdToL1DecisionCollection("PROBEEM"),
                                                 OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("EM")),
                                                 MonTool = RoIsUnpackingMonitoring( flags, prefix="EM", maxCount=30, etaOffset=0.05, phiOffset=math.pi/64., maxEta=2.5 ))

    metUnpacker = CompFactory.METRoIsUnpackingTool(Decisions = mapThresholdToL1DecisionCollection("XE"))


    tauUnpacker = CompFactory.TAURoIsUnpackingTool(Decisions = mapThresholdToL1DecisionCollection("TAU"),
                                                   DecisionsProbe = mapThresholdToL1DecisionCollection("PROBETAU"),
                                                   OutputTrigRoIs = recordable("HLT_TAURoI"))

    tauUnpacker.MonTool = RoIsUnpackingMonitoring( flags, prefix="TAU", maxCount=30, etaOffset=0.05, phiOffset=math.pi/64., maxEta=2.5  )

    jUnpacker = CompFactory.JRoIsUnpackingTool(Decisions = mapThresholdToL1DecisionCollection("J"),
                                               OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("J")) )

    jUnpacker.MonTool = RoIsUnpackingMonitoring( flags, prefix="J", maxCount=30, maxEta=5 )

    return [emUnpacker, metUnpacker, tauUnpacker, jUnpacker ]

def createCaloRoIUnpackers(flags):
    from HLTSeeding.HLTSeedingMonitoring import RoIsUnpackingMonitoring
    tools = []

    if flags.Trigger.L1.doeFex:
        maxRoICount_eFex = 150  # used for histogram range, 144 is the hardware limit
        eFexEMUnpacker = CompFactory.eFexEMRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("eEM"),
            DecisionsProbe = mapThresholdToL1DecisionCollection("PROBEeEM"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("eEM")),
            RoIHalfWidthEta = 0.2,
            RoIHalfWidthPhi = 0.2,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="eEM", maxCount=maxRoICount_eFex, maxEta=2.5))
        eFexTauUnpacker = CompFactory.eFexTauRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("eTAU"),
            DecisionsProbe = mapThresholdToL1DecisionCollection("PROBEeTAU"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("eTAU")),
            RoIHalfWidthEta = 0.4,
            RoIHalfWidthPhi = math.pi/8,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="eTAU", maxCount=maxRoICount_eFex, maxEta=2.5))
        tools += [eFexEMUnpacker, eFexTauUnpacker]

    if flags.Trigger.L1.dojFex:
        maxRoICount_jFex = 200  # used for histogram range
        jFexEMUnpacker = CompFactory.jFexFwdElRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("jEM"),
            DecisionsProbe = mapThresholdToL1DecisionCollection("PROBEjEM"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("jEM")),
            RoIHalfWidthEta = 0.4,
            RoIHalfWidthPhi = math.pi/8,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="jEM", maxCount=maxRoICount_jFex))
        jFexTauUnpacker = CompFactory.jFexTauRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("jTAU"),
            DecisionsProbe = mapThresholdToL1DecisionCollection("PROBEjTAU"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("jTAU")),
            RoIHalfWidthEta = 0.4,
            RoIHalfWidthPhi = math.pi/8,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="jTAU", maxCount=maxRoICount_jFex))
        jFexSRJetUnpacker = CompFactory.jFexSRJetRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("jJ"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("jJ")),
            RoIHalfWidthEta = 0.1,
            RoIHalfWidthPhi = 0.1,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="jJ", maxCount=maxRoICount_jFex, maxEta=5))
        jFexLRJetUnpacker = CompFactory.jFexLRJetRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("jLJ"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("jLJ")),
            RoIHalfWidthEta = 0.1,
            RoIHalfWidthPhi = 0.1,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="jLJ", maxCount=maxRoICount_jFex, maxEta=5))
        tools += [jFexEMUnpacker, jFexTauUnpacker, jFexSRJetUnpacker, jFexLRJetUnpacker]

    if flags.Trigger.L1.dogFex:
        maxRoICount_gFex = 100  # used for histogram range
        gFexSRJetUnpacker = CompFactory.gFexSRJetRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("gJ"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("gJ")),
            RoIHalfWidthEta = 0.1,
            RoIHalfWidthPhi = 0.1,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="gJ", maxCount=maxRoICount_gFex, maxEta=5))
        gFexLRJetUnpacker = CompFactory.gFexLRJetRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("gLJ"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("gLJ")),
            RoIHalfWidthEta = 0.1,
            RoIHalfWidthPhi = 0.1,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="gLJ", maxCount=maxRoICount_gFex, maxEta=5))
        tools += [gFexSRJetUnpacker, gFexLRJetUnpacker]

    # Need both eFex and jFex for cTAU
    if flags.Trigger.L1.doeFex and flags.Trigger.L1.dojFex:
        maxRoICount_eFex = 150  # used for histogram range
        cTauUnpacker = CompFactory.cTauRoIsUnpackingTool(
            Decisions = mapThresholdToL1DecisionCollection("cTAU"),
            DecisionsProbe = mapThresholdToL1DecisionCollection("PROBEcTAU"),
            OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("cTAU")),
            RoIHalfWidthEta = 0.4,
            RoIHalfWidthPhi = math.pi/8,
            MonTool = RoIsUnpackingMonitoring(flags, prefix="cTAU", maxCount=maxRoICount_eFex))
        tools += [cTauUnpacker]

    return tools

def createLegacyMuonRoIUnpackers(flags):
    from HLTSeeding.HLTSeedingMonitoring import RoIsUnpackingMonitoring
    muUnpacker = CompFactory.MURoIsUnpackingTool(
        Decisions = mapThresholdToL1DecisionCollection("MU"),
        DecisionsProbe = mapThresholdToL1DecisionCollection("PROBEMU"),
        OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("MU")),
        MonTool = RoIsUnpackingMonitoring(flags, prefix="MU", maxCount=20))

    return [muUnpacker]

def createMuonRoIUnpackers(flags):
    from HLTSeeding.HLTSeedingMonitoring import RoIsUnpackingMonitoring
    muUnpacker = CompFactory.MuonRoIsUnpackingTool(
        Decisions = mapThresholdToL1DecisionCollection("MU"),
        DecisionsProbe = mapThresholdToL1DecisionCollection("PROBEMU"),
        OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("MU")),
        RoIHalfWidthEta = 0.1,
        RoIHalfWidthPhi = 0.1,
        MonTool = RoIsUnpackingMonitoring(flags, prefix="MU", maxCount=20))
    return [muUnpacker]

def createPrescalingTool(flags):
    from HLTSeeding.HLTSeedingMonitoring import PrescalingMonitoring

    prescaler = CompFactory.PrescalingTool(MonTool = PrescalingMonitoring(flags))
    return prescaler

def createKeyWriterTool():
    keyWriter = CompFactory.getComp('TrigConf::KeyWriterTool')('KeyWriterToolOnline')
    keyWriter.ConfKeys = 'TrigConfKeysOnline'
    keyWriter.IncludeL1PrescaleKey = False
    keyWriter.IncludeBunchgroupKey = False
    return keyWriter

def L1TriggerResultMakerCfg(flags):
    acc = ComponentAccumulator()

    # Reset properties to empty by default and fill based on flags below
    l1trMaker = CompFactory.L1TriggerResultMaker(
        MuRoIKey = "",
        eFexEMRoIKey = "",
        eFexTauRoIKey = "",
        jFexFwdElRoIKey = "",
        jFexTauRoIKey = "",
        jFexSRJetRoIKey = "",
        jFexLRJetRoIKey = "",
        gFexSRJetRoIKey = "",
        gFexLRJetRoIKey = "",
        cTauRoIKey = "",
        cjTauLinkKey = "",
        ThresholdPatternTools = [] )

    # Muon RoIs
    if flags.Trigger.L1.doMuon and flags.Trigger.enableL1MuonPhase1:
        l1trMaker.MuRoIKey = "LVL1MuonRoIs"
        from TrigT1MuctpiPhase1.TrigT1MuctpiPhase1Config import TrigThresholdDecisionToolCfg
        l1trMaker.ThresholdPatternTools += [acc.popToolsAndMerge(TrigThresholdDecisionToolCfg(flags))]

    # L1Calo RoIs
    if flags.Trigger.L1.doCalo and flags.Trigger.enableL1CaloPhase1:
        if flags.Trigger.L1.doeFex:
            l1trMaker.eFexEMRoIKey = "L1_eEMRoI"
            l1trMaker.eFexTauRoIKey = "L1_eTauRoI"
            l1trMaker.ThresholdPatternTools += [
                CompFactory.eFexEMRoIThresholdsTool(),
                CompFactory.eFexTauRoIThresholdsTool(),
            ]
        if flags.Trigger.L1.dojFex:
            l1trMaker.jFexFwdElRoIKey = "L1_jFexFwdElRoI"
            l1trMaker.jFexTauRoIKey = "L1_jFexTauRoI"
            l1trMaker.jFexSRJetRoIKey = "L1_jFexSRJetRoI"
            l1trMaker.jFexLRJetRoIKey = "L1_jFexLRJetRoI"
            l1trMaker.ThresholdPatternTools += [
                CompFactory.jFexFwdElRoIThresholdsTool(),
                CompFactory.jFexTauRoIThresholdsTool(),
                CompFactory.jFexSRJetRoIThresholdsTool(),
                CompFactory.jFexLRJetRoIThresholdsTool(),
            ]
        if flags.Trigger.L1.dogFex:
            l1trMaker.gFexSRJetRoIKey = "L1_gFexSRJetRoI"
            l1trMaker.gFexLRJetRoIKey = "L1_gFexLRJetRoI"
            l1trMaker.ThresholdPatternTools += [
                CompFactory.gFexSRJetRoIThresholdsTool(),
                CompFactory.gFexLRJetRoIThresholdsTool(),
            ]
        # Need both eFex and jFex for cTAU
        if flags.Trigger.L1.doeFex and flags.Trigger.L1.dojFex:
            l1trMaker.cTauRoIKey = "L1_cTauRoI"  # Note: WriteHandle
            l1trMaker.cjTauLinkKey = "L1_cTauRoI.jTauLink"  # Note: WriteDecorHandle
            l1trMaker.ThresholdPatternTools += [
                CompFactory.cTauRoIThresholdsTool(),
            ]

    # Placeholder for other L1 xAOD outputs:
    # - CTP result
    # - L1Topo result

    acc.addEventAlgo(l1trMaker, primary=True)
    return acc


def HLTSeedingCfg(flags, seqName = None):
    if seqName:
        from AthenaCommon.CFElements import parOR
        acc = ComponentAccumulator(sequence=parOR(seqName)) # TODO - once rec-ex-common JO are phased out this can also be dropped
    else:
        acc = ComponentAccumulator()

    from HLTSeeding.HLTSeedingMonitoring import CTPUnpackingMonitoring, L1DataConsistencyMonitoring
    decoderAlg = CompFactory.HLTSeeding(
        RoIBResult = "RoIBResult", # Not yet ready to disable, see ATR-29954
        L1TriggerResult = "L1TriggerResult" if flags.Trigger.enableL1MuonPhase1 or flags.Trigger.enableL1CaloPhase1 else "",
        HLTSeedingSummaryKey = "HLTSeedingSummary", # Transient, consumed by DecisionSummaryMakerAlg
        ctpUnpacker = CompFactory.CTPUnpackingTool( ForceEnableAllChains = flags.Trigger.forceEnableAllChains,
                                                    MonTool = CTPUnpackingMonitoring(flags, 512, 400) )
    )

    # Add L1DataConsistencyChecker unless we forceEnableAllChains which always results in missing TOBs
    if not flags.Trigger.forceEnableAllChains:
        def checkConsistency(thrName):
            '''Filter out threshold types for which HLT doesn't read TOBs from L1 readout'''
            return thrName not in ['FSNOSEED','TE','XE','XS'] and not thrName.startswith('PROBE')

        decoderAlg.L1DataConsistencyChecker = CompFactory.L1DataConsistencyChecker(
            ThresholdToDecisionMap = dict([(k,v) for k,v in _mapL1ThresholdToDecisionCollection.items() if checkConsistency(k)]),
            MonTool = L1DataConsistencyMonitoring(flags),
            ErrorOnMissingTOB = flags.Trigger.L1.errorOnMissingTOB )

    #Transient bytestream
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format is Format.POOL:
        transTypeKey = ("TransientBSOutType","StoreGateSvc+TransientBSOutKey")
        decoderAlg.ExtraInputs.add(transTypeKey)

    decoderAlg.RoIBRoIUnpackers += [
        CompFactory.FSRoIsUnpackingTool("FSRoIsUnpackingTool", Decisions=mapThresholdToL1DecisionCollection("FSNOSEED"),
                                        OutputTrigRoIs = recordable(mapThresholdToL1RoICollection("FSNOSEED")) ) ]

    if flags.Trigger.L1.doCalo:
        if flags.Trigger.enableL1CaloPhase1:
            decoderAlg.xAODRoIUnpackers += createCaloRoIUnpackers(flags)
        if flags.Trigger.enableL1CaloLegacy:
            decoderAlg.RoIBRoIUnpackers += createLegacyCaloRoIUnpackers(flags)

    if flags.Trigger.L1.doMuon:
        if flags.Trigger.enableL1MuonPhase1:
            decoderAlg.xAODRoIUnpackers += createMuonRoIUnpackers(flags)
        else:
            decoderAlg.RoIBRoIUnpackers += createLegacyMuonRoIUnpackers(flags)

    decoderAlg.prescaler = createPrescalingTool(flags)
    decoderAlg.KeyWriterTool = createKeyWriterTool()
    decoderAlg.DoCostMonitoring = flags.Trigger.CostMonitoring.doCostMonitoring
    decoderAlg.CostMonitoringChain = flags.Trigger.CostMonitoring.chain
    decoderAlg.RoiZedWidthDefault = flags.Trigger.InDetTracking.RoiZedWidthDefault

    if flags.Input.Format is Format.BS and not flags.Trigger.doLVL1:
        # Add the algorithm decoding ByteStream into xAOD (Run-3 L1) and/or RoIBResult (legacy L1)
        from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
        acc.merge( L1TriggerByteStreamDecoderCfg(flags), sequenceName = seqName )

    # Add the algorithm creating L1TriggerResult which is the input to HLTSeeding (Run-3 L1)
    if flags.Trigger.enableL1MuonPhase1 or flags.Trigger.enableL1CaloPhase1:
        acc.merge( L1TriggerResultMakerCfg(flags), sequenceName = seqName )

    acc.addEventAlgo( decoderAlg, sequenceName = seqName )

    from TrigConfigSvc.TrigConfigSvcCfg import TrigConfigSvcCfg, HLTPrescaleCondAlgCfg
    acc.merge( TrigConfigSvcCfg( flags ) )
    acc.merge( HLTPrescaleCondAlgCfg( flags ) )

    # Configure ROB prefetching from initial RoIs
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
    if ROBPrefetching.InitialRoI in flags.Trigger.ROBPrefetchingOptions:
        allDecisionsSet = set()
        for roiUnpacker in decoderAlg.RoIBRoIUnpackers + decoderAlg.xAODRoIUnpackers:
            dec = str(roiUnpacker.Decisions)
            if dec:
                allDecisionsSet.add(dec)
        allDecisions = sorted(list(allDecisionsSet))
        from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si, ROBPrefetchingAlgCfg_Calo, ROBPrefetchingAlgCfg_Muon
        acc.merge(ROBPrefetchingAlgCfg_Si(flags, "initialRoI", RoILinkName="initialRoI", ROBPrefetchingInputDecisions=allDecisions), sequenceName=seqName)
        acc.merge(ROBPrefetchingAlgCfg_Calo(flags,"initialRoI", RoILinkName="initialRoI", ROBPrefetchingInputDecisions=allDecisions), sequenceName=seqName)
        acc.merge(ROBPrefetchingAlgCfg_Muon(flags,"initialRoI", RoILinkName="initialRoI", ROBPrefetchingInputDecisions=allDecisions), sequenceName=seqName)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Trigger.forceEnableAllChains= True
    flags.Input.Files= ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data17_13TeV.00327265.physics_EnhancedBias.merge.RAW._lb0100._SFO-1._0001.1",]
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.lock()
    acc = HLTSeedingCfg( flags )
    acc.wasMerged()
