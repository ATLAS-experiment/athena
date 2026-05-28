# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# HION5.py  


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND

def HION5SkimmingToolCfg(flags):
    """Configure the example skimming tool"""
    acc = ComponentAccumulator()
    
    # added: DF-prefixed jet containers to ExtraData
    JetColl = flags.HeavyIon.HIJetPrefix
    
    ExtraData  = []
    ExtraData += ['xAOD::MuonContainer/Muons']
    ExtraData += ['xAOD::ElectronContainer/Electrons']
    ExtraData += ['xAOD::PhotonContainer/Photons']
    ExtraData += ['xAOD::TrackParticleContainer/InDetTrackParticles']
    ExtraData += ['xAOD::JetContainer/'+JetColl+'AntiKt2HIJets']
    ExtraData += ['xAOD::JetContainer/'+JetColl+'AntiKt4HIJets']
    
    acc.addSequence( seqAND("HION5Sequence") )
    acc.getSequence("HION5Sequence").ExtraDataForDynamicConsumers = ExtraData
    acc.getSequence("HION5Sequence").ProcessDynamicDataDependencies = True
    filterList = []
    
    req_electrons = 'count( ( Electrons.pt > 15*GeV ) && ( abs(Electrons.eta) < 2.5) )>0'
    req_muons     = 'count( Muons.DFCommonMuonPassPreselection && (Muons.pt > 15*GeV) && ( abs(Muons.eta) < 2.7))>0'
    req_photons = 'count( Photons.DFCommonPhotonsIsEMLoose && (Photons.pt > 30*GeV) ) > 0'
    req_total = '(' + req_electrons + ' || ' + req_muons + ' || ' + req_photons + ')'
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    HION5StringSkimmingTool = acc.addPublicTool(acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(flags, name = "HION5StringSkimmingTool",
                                  expression = req_total)))
    filterList += [HION5StringSkimmingTool]
    
    from DerivationFrameworkHI import ListTriggers
    triggers = ListTriggers.HION5SkimmingTriggers()
    HION5TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
        name = "HION5TriggerSkimmingTool", TriggerListOR = triggers)
    acc.addPublicTool(HION5TriggerSkimmingTool)
    filterList += [HION5TriggerSkimmingTool]

    HION5SkimmingTool  = CompFactory.DerivationFramework.FilterCombinationAND(
        name="HION5SkimmingTool",  FilterList=filterList)
    acc.addPublicTool(HION5SkimmingTool, primary = True)
    return acc

def HION5Thinning(flags):    
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg,JetTrackParticleThinningCfg
    acc = ComponentAccumulator()

    # added: DF-prefixed jet collection names
    JetColl = flags.HeavyIon.HIJetPrefix

    # find collision type
    from CoolConvUtilities.ParticleTypeUtil import getTypeForRun
    info=getTypeForRun(flags.Input.RunNumbers[0])
    isOxygenOxygenCollision = False
    if (info.getBeam1Type() == 8) or (info.getBeam2Type() == 8):
        isOxygenOxygenCollision = True

    pTCut = 0.9
    if isOxygenOxygenCollision:
        pTCut = 0.5

    track_thinning_expression  = f"InDetTrackParticles.pt > {pTCut}*GeV"
    TrackParticleThinningTool  = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
         flags,
         name                    = "PHYSTrackParticleThinningTool",
         StreamName              = "StreamDAOD_HION5", 
         SelectionString         = track_thinning_expression,
         InDetTrackParticlesKey  = "InDetTrackParticles"))

    AntiKt2HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt2HIJetsThinningTool",
         StreamName              = "StreamDAOD_HION5",
         JetKey                  = JetColl+"AntiKt2HIJets",
         SelectionString         = JetColl+"AntiKt2HIJets.pt > 15*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    AntiKt4HIJetsThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
         flags,
         name                    = "AntiKt4HIJetsThinningTool",
         StreamName              = "StreamDAOD_HION5",
         JetKey                  = JetColl+"AntiKt4HIJets",
         SelectionString         = JetColl+"AntiKt4HIJets.pt > 15*GeV",
         InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    acc.addPublicTool(TrackParticleThinningTool,primary = True)
    acc.addPublicTool(AntiKt2HIJetsThinningTool)
    acc.addPublicTool(AntiKt4HIJetsThinningTool)
    
    return acc

def HION5KernelCfg(flags, name="HION5Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for HION5"""
    acc = ComponentAccumulator()

    # added: DF-prefixed jet collection
    JetColl = flags.HeavyIon.HIJetPrefix
    JetKey = JetColl + 'AntiKt4HIJets'

    # Schedule extra jets collections
    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track
    from JetRecConfig.JetRecConfig import JetRecCfg

    jetList = [AntiKt4PV0Track]
    for jd in jetList:
        acc.merge(JetRecCfg(flags, jd))

    # Common augmentations: reuse the same common MC/reco setup as other HION derivations
    # (same pattern as HION15, which imports PhysAugmentationsHION7Cfg from HION7)
    from DerivationFrameworkHI.HION7 import PhysAugmentationsHION7Cfg
    acc.merge(PhysAugmentationsHION7Cfg(flags))

    # jet cleaning
    # Decorate if jet passes OR and save decoration DFCommonJets_passOR
    # Use modified OR that does not check overlaps with taus
    from AssociationUtils.AssociationUtilsConfig import OverlapRemovalToolCfg

    outputLabel = "DFCommonJets_passOR_HI"
    bJetLabel = ""  # default
    tauLabel = ""  # workaround for missing taus
    tauKey = ""  # workaround for missing taus
    orTool = acc.popToolsAndMerge(
        OverlapRemovalToolCfg(
            flags, outputLabel=outputLabel, bJetLabel=bJetLabel, doTaus=False
        )
    )
    algOR = CompFactory.OverlapRemovalGenUseAlg(
        "OverlapRemovalGenUseAlg",
        OverlapLabel=outputLabel,
        OverlapRemovalTool=orTool,
        JetKey = JetKey,
        TauKey=tauKey,
        TauLabel=tauLabel,
        BJetLabel=bJetLabel,
    )
    acc.addEventAlgo(algOR)

    # skimming
    skimmingTool = acc.getPrimaryAndMerge(HION5SkimmingToolCfg(flags))
    
    # Thinning
    thinningTool= acc.getPrimaryAndMerge(HION5Thinning(flags))

    # setup the kernel
    acc.addEventAlgo(
        CompFactory.DerivationFramework.DerivationKernel(
            name,
            SkimmingTools = [skimmingTool],
            ThinningTools = [thinningTool],
            AugmentationTools = [],
            ))

    return acc

def HION5Cfg(flags):
    acc = ComponentAccumulator()


    # HIJetRec configuration with DF-prefixed jet collection names
    JetColl = flags.HeavyIon.HIJetPrefix
    from HIJetRec.HIJetRecConfigCA import HIJetRecCfg
    acc.merge(HIJetRecCfg(flags))

    # B-Tagging for HI jets — must be here after HIJetRecCfg so DFAntiKt4HIJets exists
    if flags.HeavyIon.doHIBTagging:
        from BTagging.FlavorTaggingConfig import FlavorTaggingCfg
        acc.merge(FlavorTaggingCfg(flags, JetColl+"AntiKt4HIJets"))
        from BTagging.TrackLeptonConfig import TrackLeptonDecorationCfg
        acc.merge(TrackLeptonDecorationCfg(flags))

    from DerivationFrameworkEGamma.PhotonsCPDetailedContent import PhotonsCPDetailedContent
    from DerivationFrameworkEGamma.ElectronsCPDetailedContent import ExtraElectronShowerShapes,ExtraElectronGSFVar
    
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    HION5TriggerListsHelper = TriggerListsHelper(flags)
        
    acc.merge(HION5KernelCfg(flags, name="HION5Kernel", StreamName="StreamDAOD_HION5", TriggerListsHelper = HION5TriggerListsHelper,))
    
    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkHI import ListSlimming
    
    HION5SlimmingHelper = SlimmingHelper("HION5SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HION5SlimmingHelper.AppendToDictionary = {'EventInfo':'xAOD::EventInfo','EventInfoAux':'xAOD::EventAuxInfo',
                                               'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                               'MET_Truth':'xAOD::MissingETContainer','MET_TruthAux':'xAOD::MissingETAuxContainer',
                                               'TruthLHEParticles':'xAOD::TruthParticleContainer', 'TruthLHEParticlesAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthElectrons':'xAOD::TruthParticleContainer','TruthElectronsAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthMuons':'xAOD::TruthParticleContainer','TruthMuonsAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthPhotons':'xAOD::TruthParticleContainer','TruthPhotonsAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthTaus':'xAOD::TruthParticleContainer','TruthTausAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthNeutrinos':'xAOD::TruthParticleContainer','TruthNeutrinosAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthBSM':'xAOD::TruthParticleContainer','TruthBSMAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthBoson':'xAOD::TruthParticleContainer','TruthBosonAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthBottom':'xAOD::TruthParticleContainer','TruthBottomAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthTop':'xAOD::TruthParticleContainer','TruthTopAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthForwardProtons':'xAOD::TruthParticleContainer','TruthForwardProtonsAux':'xAOD::TruthParticleAuxContainer',
                                               'BornLeptons':'xAOD::TruthParticleContainer','BornLeptonsAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                               'TruthBSMWithDecayParticles':'xAOD::TruthParticleContainer','TruthBSMWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                               'TruthBSMWithDecayVertices':'xAOD::TruthVertexContainer','TruthBSMWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                               'AntiKt4TruthDressedWZJets':'xAOD::JetContainer','AntiKt4TruthDressedWZJetsAux':'xAOD::JetAuxContainer',
                                               'AntiKt10TruthSoftDropBeta100Zcut10Jets':'xAOD::JetContainer','AntiKt10TruthSoftDropBeta100Zcut10JetsAux':'xAOD::JetAuxContainer',
                                               'MET_Track1000':'xAOD::MissingETContainer', 'MET_Track1000Aux':'xAOD::MissingETAuxContainer',
                                               'MET_Track2000':'xAOD::MissingETContainer', 'MET_Track2000Aux':'xAOD::MissingETAuxContainer',
                                               'MET_Track3000':'xAOD::MissingETContainer', 'MET_Track3000Aux':'xAOD::MissingETAuxContainer',
                                               'MET_Track4000':'xAOD::MissingETContainer', 'MET_Track4000Aux':'xAOD::MissingETAuxContainer',
                                               'MET_Track5000':'xAOD::MissingETContainer', 'MET_Track5000Aux':'xAOD::MissingETAuxContainer',
                                            }
    # Build track MET with ptCut in MeV and HItight Tracks
    from DerivationFrameworkHI.TrackMET_config import Cfg_METTrack
    met_ptCutList = [1000,2000,3000,4000,5000]

    for ptCut in met_ptCutList:
        acc.merge(Cfg_METTrack(flags, ptCut))

    AllVariables  = []    
    AllVariables += ListSlimming.HION5AllVariables(flags.Input.RunNumbers[0])
    AllVariables += ListSlimming.HION5ExtraContainersTrigger()

    # B-Tagging slimming content
    from DerivationFrameworkFlavourTag import FtagBaseContent

    if flags.Input.isMC:
        # MC truth augmentation (charm, HF, standard truth nav links, PV) is handled by
        # PhysAugmentationsHION7Cfg called in HION5KernelCfg — same as HION7/HION15.
        AllVariables += ListSlimming.HION5AllTruthVariables()
        if flags.HeavyIon.doHIBTagging:
            # Add b-tagging truth slimming content
            FtagBaseContent.add_truth_to_slimming_helper(HION5SlimmingHelper)
    
    #Variables from FTAG
    if flags.HeavyIon.doHIBTagging:
        AllVariables += ListSlimming.HION7AllVarFromFTAG1()

    HION5SlimmingHelper.SmartCollections = ListSlimming.HION5SmartCollections()
    HION5SlimmingHelper.ExtraVariables   = ListSlimming.HION5ExtraVariables()
    HION5SlimmingHelper.ExtraVariables   += PhotonsCPDetailedContent
    HION5SlimmingHelper.ExtraVariables   += ExtraElectronShowerShapes
    HION5SlimmingHelper.ExtraVariables   += ExtraElectronGSFVar
    
    #Common Augmentation for FTAG and ExtraVariables
    if flags.HeavyIon.doHIBTagging:
        from DerivationFrameworkFlavourTag.FtagBaseContent import add_common_augmentation
        add_common_augmentation(flags, acc, HION5SlimmingHelper, JetColl+"AntiKt4HIJets")
        # Update AppendToDictionary
        extra_AppendToDictionary = {}
        FtagBaseContent.update_append_to_dictionary_in_slimming_helper(flags, HION5SlimmingHelper, extra_AppendToDictionary)
        # Add ExtraVariables from B-tagging
        HION5SlimmingHelper.ExtraVariables += ListSlimming.HION7ExtraVarForBtag(JetColl+"AntiKt4HIJets")
        FtagBaseContent.add_extra_variables_to_slimming_helper(flags, HION5SlimmingHelper)
    
    HION5SlimmingHelper.AllVariables     = AllVariables

    # Add egamma trigger objects
    HION5SlimmingHelper.IncludeEGammaTriggerContent = True
    # Add muon trigger objects
    HION5SlimmingHelper.IncludeMuonTriggerContent = True
    
    HION5ItemList = HION5SlimmingHelper.GetItemList()

    #DF-prefixed jet containers with b-tagging info
    HIJetRemovedBranches = ListSlimming.makeHIJetRemovedBranchList()
    jet_var_str = '.-'.join([''] + HIJetRemovedBranches)
    
    jetRlist = flags.HeavyIon.Jet.RValues  # Default [0.2, 0.4]
    for jetR in jetRlist:
        output = ["xAOD::JetContainer#"+JetColl+"AntiKt"+str(jetR)+"HIJets",
                  "xAOD::JetAuxContainer#"+JetColl+"AntiKt"+str(jetR)+"HIJetsAux.-PseudoJet"+jet_var_str]
        HION5ItemList += output

    acc.merge(OutputStreamCfg(flags, "DAOD_HION5", ItemList=HION5ItemList, AcceptAlgs=["HION5Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HION5", AcceptAlgs=["HION5Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc
