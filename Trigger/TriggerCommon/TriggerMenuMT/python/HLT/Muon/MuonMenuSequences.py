#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from ..Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, InEventRecoCA

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaCommon.CFElements import parOR, seqAND
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
log = logging.getLogger(__name__)

#muon container names (for RoI based sequences)
from .MuonRecoSequences import muonNames
muNames = muonNames().getNames('RoI')
muNamesLRT = muonNames().getNames('LRT')
muNamesFS = muonNames().getNames('FS')
from TrigEDMConfig.TriggerEDM import recordable

#-----------------------------------------------------#
### ************* Step1  ************* ###
#-----------------------------------------------------#
def muFastAlgSequenceCfg(flags, selCAName="", is_probe_leg=False):

    selAccSA = SelectionCA('L2MuFastSel'+selCAName, isProbe=is_probe_leg)
    
    viewName="L2MuFastReco"

    recoSA = InViewRecoCA(name=viewName, isProbe=is_probe_leg)

    ### get muFast reco sequence ###   
    #Clone and replace offline flags so we can set muon trigger specific values
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    from .MuonRecoSequences import muFastRecoSequenceCfg, muonDecodeCfg
    recoSA.mergeReco(muonDecodeCfg(muonflags,RoIs=viewName+'RoIs'))

    extraLoads = []
    from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
    filterInput = [ CFNaming.inputMakerOutName('IM_'+viewName) ]
    for decision in filterInput:
      extraLoads += [( 'xAOD::TrigCompositeContainer', 'StoreGateSvc+%s' % decision )]

    from .MuonRecoSequences  import  isCosmic
    acc = ComponentAccumulator()
    seql2sa = seqAND("L2MuonSASeq{}".format("_probe" if is_probe_leg else ""))
    acc.addSequence(seql2sa)
    muFastRecoSeq = muFastRecoSequenceCfg( muonflags, viewName+'RoIs', doFullScanID= isCosmic(flags), extraLoads=extraLoads )
    sequenceOut = muNames.L2SAName
    acc.merge(muFastRecoSeq, sequenceName=seql2sa.name)

    ##### L2 mutli-track mode #####
    seqFilter = seqAND("L2MuonMTSeq{}".format("_probe" if is_probe_leg else ""))
    acc.addSequence(seqFilter)
    from TrigMuonEF.TrigMuonEFConfig import MuonChainFilterAlgCfg
    MultiTrackChains = getMultiTrackChainNames()
    MultiTrackChainFilter = MuonChainFilterAlgCfg(muonflags, "SAFilterMultiTrackChains", ChainsToFilter = MultiTrackChains, 
                                                  InputDecisions = filterInput,
                                                  L2MuFastContainer = muNames.L2SAName+"l2mtmode", 
                                                  L2MuCombContainer = muNames.L2CBName+"l2mtmode",
                                                  WriteMuFast = True, NotGate = True)

    acc.merge(MultiTrackChainFilter, sequenceName=seqFilter.name)
    muFastl2mtRecoSeq = muFastRecoSequenceCfg( muonflags, viewName+'RoIs', doFullScanID= isCosmic(flags), l2mtmode=True )
    acc.merge(muFastl2mtRecoSeq, sequenceName=seqFilter.name)
    recoSA.mergeReco(acc)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    robPrefetch = ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)

    selAccSA.mergeReco(recoSA, robPrefetchCA=robPrefetch)


    return (selAccSA, sequenceOut)


def muFastCalibAlgSequenceCfg(flags, is_probe_leg=False):

    selAccSA = SelectionCA('L2MuFastCalibSel', isProbe=is_probe_leg)
    
    viewName="L2MuFastCalibReco"

    recoSA = InViewRecoCA(name=viewName, isProbe=is_probe_leg)

    ### get muFast reco sequence ###
    #Clone and replace offline flags so we can set muon trigger specific values
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    from .MuonRecoSequences import muFastRecoSequenceCfg, muonDecodeCfg
    recoSA.mergeReco(muonDecodeCfg(muonflags,RoIs=viewName+'RoIs'))

    from .MuonRecoSequences  import  isCosmic
    muFastRecoSeq = muFastRecoSequenceCfg( muonflags, viewName+'RoIs', doFullScanID= isCosmic(flags), calib=True )
    sequenceOut = muNames.L2SAName+"Calib"
    recoSA.mergeReco(muFastRecoSeq)


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    robPrefetchAlg = ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAccSA.mergeReco(recoSA, robPrefetchCA=robPrefetchAlg)

    return (selAccSA, sequenceOut)


@AccumulatorCache
def muFastSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muFastAlgSequenceCfg(flags, "", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMufastHypoAlgCfg, TrigMufastHypoToolFromDict
    l2saHypo = TrigMufastHypoAlgCfg( flags,
                                     name = 'TrigL2MufastHypoAlg',
                                     MuonL2SAInfoFromMuFastAlg = sequenceOut)

    selAcc.addHypoAlgo(l2saHypo)
    
    l2saSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigMufastHypoToolFromDict, isProbe=is_probe_leg)


    return l2saSequence


@AccumulatorCache
def muFastCalibSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muFastCalibAlgSequenceCfg(flags, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMufastHypoAlgCfg, TrigMufastHypoToolFromDict
    l2saHypo = TrigMufastHypoAlgCfg( flags,
                                     name = 'TrigL2MufastCalibHypoAlg',
                                     MuonL2SAInfoFromMuFastAlg = sequenceOut)

    selAcc.addHypoAlgo(l2saHypo)
    
    l2saSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigMufastHypoToolFromDict, isProbe=is_probe_leg)


    return l2saSequence


@AccumulatorCache
def mul2mtSAOvlpRmSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muFastAlgSequenceCfg(flags, "mt", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMufastHypoAlgCfg, TrigMufastHypoToolFromDict
    l2saHypo = TrigMufastHypoAlgCfg( flags,
                                     name = 'TrigL2mtMufastHypoAlg',
                                     MuonL2SAInfoFromMuFastAlg = muNames.L2SAName+"l2mtmode")

    selAcc.addHypoAlgo(l2saHypo)
    
    l2saSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigMufastHypoToolFromDict, isProbe=is_probe_leg)


    return l2saSequence



#-----------------------------------------------------#
### ************* Step2  ************* ###
#-----------------------------------------------------#

def muCombAlgSequenceCfg(flags, selCAName="", is_probe_leg=False):
    ### set the EVCreator ###
    ### get ID tracking and muComb reco sequences ###
    from .MuonRecoSequences  import muFastRecoSequenceCfg, muCombRecoSequenceCfg, muonIDFastTrackingSequenceCfg, muonIDCosmicTrackingSequenceCfg, isCosmic

    selAccCB = SelectionCA('L2MuCombSel'+selCAName, isProbe=is_probe_leg)
    
    viewName="Cosmic" if isCosmic(flags) else "L2MuCombReco"

    ViewCreatorFetchFromViewROITool=CompFactory.ViewCreatorFetchFromViewROITool
    #temporarily using different view names until L2 SA sequence is migrated to CA
    roiTool         = ViewCreatorFetchFromViewROITool(RoisWriteHandleKey="HLT_Roi_L2SAMuon", InViewRoIs = muNames.L2forIDName)
    requireParentView = True

    recoCB = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView, isProbe=is_probe_leg)
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')

    
    sequenceOut = muNames.L2CBName

    acc = ComponentAccumulator()
    from TrigMuonEF.TrigMuonEFConfig import MuonChainFilterAlgCfg


    from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
    filterInput = [ CFNaming.inputMakerOutName('IM_'+viewName) ]
    extraLoads = []
    for decision in filterInput:
      extraLoads += [( 'xAOD::TrigCompositeContainer' , 'StoreGateSvc+%s' % decision )]

    if isCosmic(flags):
        recoCB.mergeReco(muonIDCosmicTrackingSequenceCfg( flags, viewName+"RoIs" , "cosmics", extraLoads, extraLoads ))
    else:
        recoCB.mergeReco(muonIDFastTrackingSequenceCfg(flags, viewName+"RoIs", "muon", extraLoads, extraLoads ))

    # for nominal muComb
    seql2cb = seqAND("l2muCombFilterSequence{}".format("_probe" if is_probe_leg else ""))
    acc.addSequence(seql2cb)

    #Filter algorithm to run muComb only if non-Bphysics muon chains are active
    muonChainFilter = MuonChainFilterAlgCfg(muonflags, "FilterBphysChains", ChainsToFilter = getBphysChainNames(), 
                                            InputDecisions = filterInput,
                                            L2MuCombContainer = sequenceOut,
                                            WriteMuComb = True, WriteMuFast=False)
    acc.merge(muonChainFilter, sequenceName=seql2cb.name)



    acc.merge(muCombRecoSequenceCfg(flags, viewName+"RoIs", "FTF", l2mtmode=False, l2CBname = sequenceOut ), sequenceName=seql2cb.name)

    # for L2 multi-track SA
    MultiTrackChains = getMultiTrackChainNames()
    MultiTrackChainFilter = MuonChainFilterAlgCfg(muonflags, "CBFilterMultiTrackChains", ChainsToFilter = MultiTrackChains, 
                                                  InputDecisions = filterInput,
                                                  L2MuFastContainer = muNames.L2SAName+"l2mtmode", 
                                                  L2MuCombContainer = muNames.L2CBName+"l2mtmode",
                                                  WriteMuComb = True, NotGate = True)


    seql2cbmt = seqAND("l2mtmuCombFilterSequence{}".format("_probe" if is_probe_leg else ""))
    acc.addSequence(seql2cbmt)
    acc.merge(MultiTrackChainFilter, sequenceName=seql2cbmt.name)


    sequenceOutL2mtCB = muNames.L2CBName+"l2mtmode"
    acc.merge(muCombRecoSequenceCfg(flags, viewName+"RoIs", "FTF", l2mtmode=True, l2CBname = sequenceOutL2mtCB ), sequenceName=seql2cbmt.name)



    # for Inside-out L2SA
    seql2iocb = seqAND("l2muFastIOFilterSequence{}".format("_probe" if is_probe_leg else ""))
    acc.addSequence(seql2iocb)

    from .MuonRecoSequences  import isCosmic
    sequenceOutL2SAIO = muNames.L2SAName+"IOmode"
    insideoutMuonChainFilter = MuonChainFilterAlgCfg("FilterInsideOutMuonChains", ChainsToFilter = getInsideOutMuonChainNames(),
                                                     InputDecisions = filterInput,
                                                     L2MuFastContainer = sequenceOutL2SAIO, L2MuCombContainer = muNames.L2CBName+"IOmode",
                                                     WriteMuFast = True, WriteMuComb = True, NotGate=True)

    acc.merge(insideoutMuonChainFilter, sequenceName=seql2iocb.name)
    acc.merge(muFastRecoSequenceCfg(muonflags, viewName+"RoIs", doFullScanID=isCosmic(flags) , InsideOutMode=True), sequenceName=seql2iocb.name)
    recoCB.mergeReco(acc)


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAccCB.mergeReco(recoCB, robPrefetchCA=robPrefetchAlg)

    return (selAccCB, sequenceOut)


@AccumulatorCache
def muCombSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muCombAlgSequenceCfg(flags, "", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, TrigmuCombHypoToolFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2MuCBHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut)

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigmuCombHypoToolFromDict, isProbe=is_probe_leg)


    return l2cbSequence


@AccumulatorCache
def mul2IOOvlpRmSequence(flags, is_probe_leg=False):


    (selAcc, sequenceOut) = muCombAlgSequenceCfg(flags, "IO", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, Trigl2IOHypoToolwORFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2MuCBIOHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut+"IOmode")

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = Trigl2IOHypoToolwORFromDict, isProbe=is_probe_leg)

    return l2cbSequence


def muCombLRTAlgSequenceCfg(flags, is_probe_leg=False):

    selAcc = SelectionCA('l2muCombLRT', isProbe=is_probe_leg)
    
    viewName="l2muCombLRT"
    ViewCreatorCenteredOnIParticleTool=CompFactory.ViewCreatorCentredOnIParticleROITool
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDConfig = getInDetTrigConfig("muonLRT")
  
    roiTool         = ViewCreatorCenteredOnIParticleTool(RoisWriteHandleKey = recordable("HLT_Roi_L2SAMuon_LRT"), RoIZedWidth=IDConfig.zedHalfWidth, RoIEtaWidth=IDConfig.etaHalfWidth, RoIPhiWidth=IDConfig.phiHalfWidth, UseZedPosition=False)
    requireParentView = True

    recol2cb = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView, isProbe=is_probe_leg)

    ### get ID tracking and muComb reco sequences ###
    from .MuonRecoSequences  import muCombRecoSequenceCfg, muonIDFastTrackingSequenceCfg
    sequenceOut = muNamesLRT.L2CBName
    recol2cb.mergeReco(muCombRecoSequenceCfg(flags, viewName+"RoIs", "FTF_LRT", l2CBname = sequenceOut ))

    extraLoads = []

    recol2cb.mergeReco(muonIDFastTrackingSequenceCfg(flags, viewName+"RoIs" , "muonLRT", extraLoads, doLRT=True ))


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAcc.mergeReco(recol2cb, robPrefetchAlg)


    return (selAcc, sequenceOut)


@AccumulatorCache
def muCombLRTSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muCombLRTAlgSequenceCfg(flags, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, TrigmuCombHypoToolFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2MuCBLRTHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut,
                                     RoILinkName = "l2lrtroi")

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigmuCombHypoToolFromDict, isProbe=is_probe_leg)


    return l2cbSequence


@AccumulatorCache
def muCombOvlpRmSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muCombAlgSequenceCfg(flags, "", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, TrigmuCombHypoToolwORFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2MuCBHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut)

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = TrigmuCombHypoToolwORFromDict, isProbe=is_probe_leg)

    return l2cbSequence
 

@AccumulatorCache
def mul2mtCBOvlpRmSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muCombAlgSequenceCfg(flags, "mt", is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigmuCombHypoAlgCfg, Trigl2mtCBHypoToolwORFromDict
    l2cbHypo = TrigmuCombHypoAlgCfg( flags,
                                     name = 'TrigL2mtMuCBHypoAlg',
                                     MuonL2CBInfoFromMuCombAlg = sequenceOut+"l2mtmode")

    selAcc.addHypoAlgo(l2cbHypo)
    
    l2cbSequence = MenuSequenceCA(flags, selAcc,
                                  HypoToolGen = Trigl2mtCBHypoToolwORFromDict, isProbe=is_probe_leg)

    return l2cbSequence


######################
###  EFSA step ###
######################

def muEFSAAlgSequenceCfg(flags, is_probe_leg=False):

    selAccMS = SelectionCA('EFMuMSSel_RoI', isProbe=is_probe_leg)
    
    viewName="EFMuMSReco_RoI"
    ViewCreatorFetchFromViewROITool=CompFactory.ViewCreatorFetchFromViewROITool
    #temporarily using different view names until L2 SA sequence is migrated to CA
    roiTool         = ViewCreatorFetchFromViewROITool(RoisWriteHandleKey="HLT_Roi_L2SAMuonForEF", InViewRoIs = "forMS", ViewToFetchFrom = "L2MuFastRecoViews")
    requireParentView = True

    recoMS = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView, isProbe=is_probe_leg)


    #Clone and replace offline flags so we can set muon trigger specific values
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    from .MuonRecoSequences import muEFSARecoSequenceCfg, muonDecodeCfg
    #Run decoding again since we are using updated RoIs
    recoMS.mergeReco(muonDecodeCfg(muonflags,RoIs=viewName+"RoIs"))
    ### get EF reco sequence ###    
    muEFSARecoSequenceAcc, sequenceOut = muEFSARecoSequenceCfg(muonflags, viewName+'RoIs', 'RoI' )
    recoMS.mergeReco(muEFSARecoSequenceAcc)

    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    prefetch=ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)
    selAccMS.mergeReco(recoMS, robPrefetchCA=prefetch)

    return (selAccMS, sequenceOut)


@AccumulatorCache
def muEFSASequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFSAAlgSequenceCfg(flags, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFMSonlyHypoToolFromDict
    efmuMSHypo = TrigMuonEFHypoAlgCfg( flags,
                              name = 'TrigMuonEFMSonlyHypo_RoI',
                              MuonDecisions = sequenceOut,
                              IncludeSAmuons=True)

    selAcc.addHypoAlgo(efmuMSHypo)
    
    efmuMSSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFMSonlyHypoToolFromDict, isProbe=is_probe_leg)


    return efmuMSSequence


######################
###  EFCB seq ###
######################

def muEFCBAlgSequenceCfg(flags, selCAName='', is_probe_leg=False):

    from .MuonRecoSequences import isCosmic
    selAccCB = SelectionCA('EFMuCBSel_RoI'+selCAName, isProbe=is_probe_leg)
    
    viewName="EFMuCBReco_RoI" if not isCosmic(flags) else "CosmicEFCB"
    ViewCreatorTool=CompFactory.ViewCreatorNamedROITool
    #temporarily using different view names until L2 SA sequence is migrated to CA
    roiTool         = ViewCreatorTool(ROILinkName="l2cbroi")

    recoCB = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = True, isProbe=is_probe_leg, mergeUsingFeature=True)

    #By default the EFCB sequence will run both outside-in and
    #(if zero muons are found) inside-out reconstruction
    from TrigMuonEF.TrigMuonEFConfig import MuonFilterAlgCfg, MergeEFMuonsAlgCfg
    from .MuonRecoSequences import muEFCBRecoSequenceCfg, muEFInsideOutRecoSequenceCfg

    acc = ComponentAccumulator()
    seqmerge = seqAND("muonCBInsideOutMergingSequence{}".format("_probe" if is_probe_leg else ""))
    acc.addSequence(seqmerge)

    #outside-in reco sequence
    acc2 = ComponentAccumulator()
    seqreco = parOR("muonEFCBandInsideOutRecoSequence{}".format("_probe" if is_probe_leg else ""))
    acc2.addSequence(seqreco)
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    acc2.merge(muEFCBRecoSequenceCfg(muonflagsCB, viewName+"RoIs", "RoI" ), sequenceName=seqreco.name)
    sequenceOutCB = muNames.EFCBOutInName

    #Algorithm to filter events with no muons
    acc3 = ComponentAccumulator()
    seqfilter = seqAND("muonEFInsideOutSequence{}".format("_probe" if is_probe_leg else ""))
    acc3.addSequence(seqfilter)
    muonFilter = MuonFilterAlgCfg(flags, name="FilterZeroMuons", MuonContainerLocation=sequenceOutCB)
    acc3.merge(muonFilter, sequenceName=seqfilter.name)

    #inside-out reco sequence - runs only if filter is passed
    acc4 = ComponentAccumulator()
    seqio = parOR("efmuInsideOutViewNode_RoI{}".format("_probe" if is_probe_leg else ""))
    acc4.addSequence(seqio)
    acc4.merge(muEFInsideOutRecoSequenceCfg(muonflagsCB, viewName+"RoIs", "RoI"), sequenceName=seqio.name)
    sequenceOutInsideOut = muNames.EFCBInOutName

    acc3.merge(acc4, sequenceName=seqfilter.name)
    acc2.merge(acc3, sequenceName=seqreco.name)
    acc.merge(acc2, sequenceName=seqmerge.name)
    #Merge muon containers from outside-in and inside-out reco
    mergeMuons = MergeEFMuonsAlgCfg(flags, name="MergeEFMuons", MuonCBContainerLocation=sequenceOutCB, 
                                    MuonInsideOutContainerLocation=sequenceOutInsideOut, MuonOutputLocation=muNames.EFCBName)

    acc.merge(mergeMuons, sequenceName=seqmerge.name)
    recoCB.mergeReco(acc)
    selAccCB.mergeReco(recoCB)

    return (selAccCB, muNames.EFCBName)


@AccumulatorCache
def muEFCBSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFCBAlgSequenceCfg(flags, '', is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFCombinerHypoToolFromDict
    efmuCBHypo = TrigMuonEFHypoAlgCfg( flags,
                              name = 'TrigMuonEFCombinerHypoAlg',
                              MuonDecisions = sequenceOut,
                              MapToPreviousDecisions=True)

    selAcc.addHypoAlgo(efmuCBHypo)
    
    efmuCBSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFCombinerHypoToolFromDict, isProbe=is_probe_leg)

    return efmuCBSequence


@AccumulatorCache
def muEFCBIDperfSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFCBAlgSequenceCfg(flags, 'idperf', is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFCombinerHypoToolFromDict
    efmuCBHypo = TrigMuonEFHypoAlgCfg( flags,
                                       name = 'TrigMuonEFCombinerIDperfHypoAlg',
                                       IncludeSAmuons=True,
                                       MuonDecisions = sequenceOut,
                                       MapToPreviousDecisions=True)

    selAcc.addHypoAlgo(efmuCBHypo)
    
    efmuCBSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFCombinerHypoToolFromDict, isProbe=is_probe_leg)

    return efmuCBSequence


@AccumulatorCache
def muEFIDtpSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFCBAlgSequenceCfg(flags, 'idtp', is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFIdtpHypoAlgCfg, TrigMuonEFIdtpHypoToolFromDict
    efmuCBHypo = TrigMuonEFIdtpHypoAlgCfg( flags,
                                       name = 'TrigMuonEFIdtpHypoAlg')


    selAcc.addHypoAlgo(efmuCBHypo)
    
    efmuCBSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFIdtpHypoToolFromDict, isProbe=is_probe_leg)

    return efmuCBSequence


def muEFCBLRTAlgSequenceCfg(flags, selCAName='', is_probe_leg=False):

    selAccCB = SelectionCA('EFMuCBLRTSel'+selCAName, isProbe=is_probe_leg)
    
    viewName="EFMuCBLRTReco"
    ViewCreatorTool=CompFactory.ViewCreatorNamedROITool
    roiTool         = ViewCreatorTool(ROILinkName="l2lrtroi")

    recoCB = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = True, mergeUsingFeature=True, isProbe=is_probe_leg)

    from .MuonRecoSequences import muEFCBRecoSequenceCfg

    #outside-in reco sequence
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    recoCB.mergeReco(muEFCBRecoSequenceCfg(muonflagsCB, viewName+"RoIs", "LRT"))
    sequenceOut = muNamesLRT.EFCBName

    selAccCB.mergeReco(recoCB)

    return (selAccCB, sequenceOut)


@AccumulatorCache
def muEFCBLRTSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFCBLRTAlgSequenceCfg(flags, '', is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFCombinerHypoToolFromDict
    efmuCBLRTHypo = TrigMuonEFHypoAlgCfg( flags,
                                          name = 'TrigMuonEFCombinerHypoAlgLRT',
                                          MuonDecisions = sequenceOut,
                                          MapToPreviousDecisions=True)


    selAcc.addHypoAlgo(efmuCBLRTHypo)
    
    efmuCBSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFCombinerHypoToolFromDict, isProbe=is_probe_leg)

    return efmuCBSequence


@AccumulatorCache
def muEFCBLRTIDperfSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFCBLRTAlgSequenceCfg(flags, 'idperf', is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFCombinerHypoToolFromDict
    efmuCBLRTHypo = TrigMuonEFHypoAlgCfg( flags,
                                          name = 'TrigMuonEFCombinerHypoAlgLRTIDPerf',
                                          IncludeSAmuons=True,
                                          MuonDecisions = sequenceOut,
                                          MapToPreviousDecisions=True)


    selAcc.addHypoAlgo(efmuCBLRTHypo)
    
    efmuCBSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFCombinerHypoToolFromDict, isProbe=is_probe_leg)

    return efmuCBSequence


######################
### EF SA full scan ###
######################

def muEFSAFSAlgSequenceCfg(flags):

    selAccMS = SelectionCA('EFMuMSSel_FS')
    
    viewName="EFMuMSReco_FS"
    ViewCreatorFSROITool=CompFactory.ViewCreatorFSROITool
    roiTool         = ViewCreatorFSROITool(RoisWriteHandleKey="MuonFS_RoIs")
    requireParentView = False
                                                         
    recoMS = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView)


    #Clone and replace offline flags so we can set muon trigger specific values
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    from .MuonRecoSequences import muEFSARecoSequenceCfg, muonDecodeCfg
    recoMS.mergeReco(muonDecodeCfg(muonflags,RoIs=recoMS.name+"RoIs"))
    ### get EF reco sequence ###    
    muEFSARecoSequenceAcc, sequenceOut = muEFSARecoSequenceCfg(muonflags, recoMS.name+'RoIs', 'FS' )
    recoMS.mergeReco(muEFSARecoSequenceAcc)

    selAccMS.mergeReco(recoMS)

    return (selAccMS, sequenceOut)


@AccumulatorCache
def muEFSAFSSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFSAFSAlgSequenceCfg(flags)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFMSonlyHypoToolFromName

    efmuMSHypo = TrigMuonEFHypoAlgCfg( flags,
                              name = 'TrigMuonEFMSonlyHypo_FS',
                              MuonDecisions = sequenceOut,
                              IncludeSAmuons=True)

    selAcc.addHypoAlgo(efmuMSHypo)
    
    efmuMSSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFMSonlyHypoToolFromName, isProbe=is_probe_leg)


    return efmuMSSequence


######################
### EF CB full scan ###
######################

def muEFCBFSAlgSequenceCfg(flags, is_probe_leg=False):

    selAccCB = SelectionCA('EFMuCBSel_FS', isProbe=is_probe_leg)
    
    viewName="EFMuCBReco_FS"
    #temporarily using different view names until L2 SA sequence is migrated to CA
    roiTool         = CompFactory.ViewCreatorCentredOnIParticleROITool(RoisWriteHandleKey = "MuonCandidates_FS_ROIs")

    recoCB = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = True, isProbe=is_probe_leg, mergeUsingFeature=True, 
                          PlaceMuonInView=True, InViewMuons = "InViewMuons", InViewMuonCandidates = "MuonCandidates_FS")


    from TrigMuonEF.TrigMuonEFConfig import MuonFilterAlgCfg, MergeEFMuonsAlgCfg
    from .MuonRecoSequences import muEFCBRecoSequenceCfg, muEFInsideOutRecoSequenceCfg
    #outside-in reco sequence
    acc = ComponentAccumulator()
    seqmerge = seqAND("muonCBInsideOutMergingSequenceEFCBFS{}".format("_probe" if is_probe_leg else ""))
    acc.addSequence(seqmerge)

    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    acc2 = ComponentAccumulator()
    seqreco = parOR("muonEFCBFSInsideOutRecoSequence{}".format("_probe" if is_probe_leg else ""))
    acc2.addSequence(seqreco)

    acc2.merge(muEFCBRecoSequenceCfg(muonflagsCB, viewName+"RoIs", "FS" ), sequenceName=seqreco.name)
    sequenceOutCB = muNamesFS.EFCBOutInName

    #Alg fitltering for no muon events
    muonFilter =  MuonFilterAlgCfg(flags, name="FilterZeroMuonsEFCBFS", MuonContainerLocation = sequenceOutCB)

    acc3 = ComponentAccumulator()
    seqfilt = seqAND("muonEFCBFSInsideOutSequence{}".format("_probe" if is_probe_leg else ""))
    acc3.addSequence(seqfilt)
    acc3.merge(muonFilter, sequenceName=seqfilt.name)


    #If filter passed
    acc4 = ComponentAccumulator()
    seqio = parOR("efmuInsideOutViewNode_FS{}".format("_probe" if is_probe_leg else ""))
    acc4.addSequence(seqio)

    muonEFInsideOutRecoAlgSequence = muEFInsideOutRecoSequenceCfg(muonflagsCB, viewName+"RoIs", "FS" )
    acc4.merge(muonEFInsideOutRecoAlgSequence, sequenceName=seqio.name)
    acc3.merge(acc4, sequenceName=seqfilt.name)
    acc2.merge(acc3, sequenceName=seqreco.name)
    acc.merge(acc2, sequenceName=seqmerge.name)
    sequenceOutInsideOut = muNamesFS.EFCBInOutName

    #Merge muon containers from O-I and I-O reco
    mergeMuons = MergeEFMuonsAlgCfg(flags, name="MergeEFCBFSMuons", MuonCBContainerLocation = sequenceOutCB, 
                                    MuonInsideOutContainerLocation = sequenceOutInsideOut, MuonOutputLocation = muNamesFS.EFCBName)
    acc.merge(mergeMuons, sequenceName=seqmerge.name)
    recoCB.mergeReco(acc)

    sequenceOut = muNamesFS.EFCBName
    selAccCB.mergeReco(recoCB)

    return (selAccCB, sequenceOut)


@AccumulatorCache
def muEFCBFSSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFCBFSAlgSequenceCfg(flags, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg, TrigMuonEFCombinerHypoToolFromName
    efmuCBHypo = TrigMuonEFHypoAlgCfg( flags,
                              name = 'TrigMuonEFFSCombinerHypoAlg',
                              MuonDecisions = sequenceOut)

    selAcc.addHypoAlgo(efmuCBHypo)
    
    efmuCBSequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonEFCombinerHypoToolFromName, isProbe=is_probe_leg)

    return efmuCBSequence


def efLateMuRoIAlgSequenceCfg(flags):

    selAcc = SelectionCA('EFLateMuSel')
    
    viewName="EFLateMuRoIReco"
    viewcreator         = CompFactory.ViewCreatorInitialROITool
    roiTool = viewcreator()
    requireParentView = True
                                                         
    recoLateMu = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView)

    from .MuonRecoSequences import efLateMuRoISequenceCfg

    #Get Late Muon RoIs
    efLateMuRoIAcc, sequenceOut = efLateMuRoISequenceCfg(flags)
    recoLateMu.mergeReco(efLateMuRoIAcc)

    selAcc.mergeReco(recoLateMu)
    return (selAcc, sequenceOut)


@AccumulatorCache
def efLateMuRoISequence(flags):

    (selAcc, sequenceOut) = efLateMuRoIAlgSequenceCfg(flags)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonLateMuRoIHypoAlgCfg, TrigMuonLateMuRoIHypoToolFromDict
    latemuHypo = TrigMuonLateMuRoIHypoAlgCfg( flags,
                              name = 'TrigMuonLateMuRoIHypoAlg',
                              LateRoIs = sequenceOut)

    selAcc.addHypoAlgo(latemuHypo)
    
    latemuRoISequence = MenuSequenceCA(flags, selAcc,
                                    HypoToolGen = TrigMuonLateMuRoIHypoToolFromDict)


    return latemuRoISequence


def efLateMuAlgSequenceCfg(flags):

    from .MuonRecoSequences import muEFInsideOutRecoSequenceCfg, muonDecodeCfg, muonIDFastTrackingSequenceCfg
    selAcc = SelectionCA('EFLateMuAlg')
    
    viewName="EFLateMuReco"
    viewcreator         = CompFactory.ViewCreatorNamedROITool
    roiTool = viewcreator(ROILinkName="feature")
    requireParentView = True
                                                         
    recoLateMu = InViewRecoCA(name=viewName, RoITool = roiTool, RequireParentView = requireParentView, mergeUsingFeature=True)


    #Clone and replace offline flags so we can set muon trigger specific values
    muonflagsCB = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon').cloneAndReplace('MuonCombined', 'Trigger.Offline.Combined.MuonCombined')
    muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
    #decode data in these RoIs
    recoLateMu.mergeReco(muonDecodeCfg(muonflags,RoIs=recoLateMu.name+"RoIs"))
    #ID fast tracking
    recoLateMu.mergeReco(muonIDFastTrackingSequenceCfg(flags, recoLateMu.name+"RoIs","muonLate" ))
    #inside-out reco sequence
    recoLateMu.mergeReco(muEFInsideOutRecoSequenceCfg(muonflagsCB, recoLateMu.name+"RoIs", "LateMu"))
    sequenceOut = muNames.EFCBInOutName+'_Late'


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Muon
    robPrefetchAlg = ROBPrefetchingAlgCfg_Muon(flags, nameSuffix=viewName)
    selAcc.mergeReco(recoLateMu, robPrefetchCA = robPrefetchAlg)

    return (selAcc, sequenceOut)


@AccumulatorCache
def efLateMuSequence(flags):

    (selAcc, sequenceOut) = efLateMuAlgSequenceCfg(flags)

    # setup EFCB hypo
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFHypoAlgCfg
    trigMuonEFLateHypo = TrigMuonEFHypoAlgCfg( "TrigMuonEFCombinerLateMuHypoAlg", MuonDecisions = sequenceOut )

    selAcc.addHypoAlgo(trigMuonEFLateHypo)
    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFCombinerHypoToolFromDict

    return MenuSequenceCA(flags, selAcc,
                          HypoToolGen = TrigMuonEFCombinerHypoToolFromDict )



######################
### efMuiso step ###
######################
def muEFIsoAlgSequenceCfg(flags, doMSiso=False, is_probe_leg=False):
    name = ""
    if doMSiso:
        name = "MS"


    selAccIso = SelectionCA('EFMuIso'+name, isProbe=is_probe_leg)
    
    viewName="EFMuIsoReco"+name
    if doMSiso:
        roisWriteHandleKey = "Roi_MuonIsoMS"
    else:
        roisWriteHandleKey = recordable("HLT_Roi_MuonIso")

    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDConfig = getInDetTrigConfig("muonIso")

    roiTool         = CompFactory.ViewCreatorCentredOnIParticleROITool(RoisWriteHandleKey = roisWriteHandleKey, RoIEtaWidth=IDConfig.etaHalfWidth,
                                                                       RoIPhiWidth=IDConfig.phiHalfWidth,RoIZedWidth=IDConfig.zedHalfWidth)

    recoIso = InViewRecoCA(name=viewName, RoITool = roiTool, isProbe=is_probe_leg, mergeUsingFeature=True,
                          PlaceMuonInView=True, InViewMuons = "IsoViewMuons"+name, InViewMuonCandidates = "IsoMuonCandidates"+name)


    ### get EF reco sequence ###
    from .MuonRecoSequences  import efmuisoRecoSequenceCfg
    sequenceOut = muNames.EFIsoMuonName+name
    recoIso.mergeReco(efmuisoRecoSequenceCfg( flags, viewName+"RoIs", "IsoViewMuons"+name, doMSiso ))


    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    robPrefetchAlg = ROBPrefetchingAlgCfg_Si(flags, nameSuffix=viewName+'_probe' if is_probe_leg else viewName)

    selAccIso.mergeReco(recoIso, robPrefetchCA = robPrefetchAlg)

    return (selAccIso, sequenceOut)


@AccumulatorCache
def muEFIsoSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFIsoAlgSequenceCfg(flags, False, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFTrackIsolationHypoAlgCfg, TrigMuonEFTrackIsolationHypoToolFromDict
    efmuisoHypo = TrigMuonEFTrackIsolationHypoAlgCfg( flags,
                                                      name = 'EFMuisoHypoAlg',
                                                      EFMuonsName = sequenceOut)

    selAcc.addHypoAlgo(efmuisoHypo)
    
    efmuisoSequence = MenuSequenceCA(flags, selAcc,
                                     HypoToolGen = TrigMuonEFTrackIsolationHypoToolFromDict, isProbe=is_probe_leg)

    return efmuisoSequence


@AccumulatorCache
def muEFMSIsoSequence(flags, is_probe_leg=False):

    (selAcc, sequenceOut) = muEFIsoAlgSequenceCfg(flags, True, is_probe_leg)

    from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFTrackIsolationHypoAlgCfg, TrigMuonEFTrackIsolationHypoToolFromDict
    efmuisoHypo = TrigMuonEFTrackIsolationHypoAlgCfg( flags,
                                                      name = 'EFMuMSisoHypoAlg',
                                                      EFMuonsName = sequenceOut)

    selAcc.addHypoAlgo(efmuisoHypo)
    
    efmuisoSequence = MenuSequenceCA(flags, selAcc,
                                     HypoToolGen = TrigMuonEFTrackIsolationHypoToolFromDict, isProbe=is_probe_leg)

    return efmuisoSequence


####################################################
##  Muon RoI Cluster Trigger for MS LLP Searches  ##
####################################################

def muRoiClusterSequence(flags):

    from TrigLongLivedParticles.TrigLongLivedParticlesConfig import MuonClusterConfig
    from TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoConfig import MuonClusterHypoAlgConfig, TrigLongLivedParticlesHypoToolFromDict

    selAcc = SelectionCA('muRoIClusterSel')
    
    viewName="MuRoIClusReco"
    viewcreator         = CompFactory.ViewCreatorInitialROITool
    roiTool = viewcreator()
                                                         
    recoRoICluster = InEventRecoCA(name=viewName, RoITool = roiTool, mergeUsingFeature = False, RoIs = 'HLT_muVtxCluster_RoIs')
    recoRoICluster.mergeReco(MuonClusterConfig(flags, 'muvtxMuonCluster'))
    selAcc.mergeReco(recoRoICluster)


    hypoAlg = MuonClusterHypoAlgConfig( flags,
                              name = 'MuRoiClusterHypoAlg')

    selAcc.addHypoAlgo(hypoAlg)
    
    muRoIClusterSequence = MenuSequenceCA(flags, selAcc,
                                          HypoToolGen = TrigLongLivedParticlesHypoToolFromDict)

    return muRoIClusterSequence



##############################
### Get Bphysics triggers to #
### filter chains where we   #
### don't want to run muComb #
##############################

def getBphysChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()  # get menu singleton
    chains = [chain.name for chain in menu.chainsInMenu['Bphysics']]
    return chains

############################################################
### Get muon triggers except L2 inside-out trigger
### to filter chains where we don't want to run L2SA IO mode
############################################################

def getInsideOutMuonChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()  # get menu singleton
    chains = [chain.name for chain in menu.chainsInMenu['Muon'] if "l2io" in chain.name]
    chains += [chain.name for chain in menu.chainsInMenu['Bphysics'] if not any(key in chain.name for key in ['noL2Comb','l2mt'])]
    return chains

############################################################
### Get muon triggers except L2 multi-track trigger
### to filter chains where we don't want to run L2SA multi-track mode
############################################################

def getMultiTrackChainNames():
    from ..Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT()  # get menu singleton
    chains = [chain.name for chain in menu.chainsInMenu['Muon'] if "l2mt" in chain.name]
    chains += [chain.name for chain in menu.chainsInMenu['Bphysics'] if "l2mt" in chain.name]
    return chains
