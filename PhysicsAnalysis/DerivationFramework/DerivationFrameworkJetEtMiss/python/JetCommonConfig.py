# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Contains the configuration for common jet reconstruction + decorations
# used in analysis DAODs
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def JetCommonCfg(ConfigFlags):
    """Main config for jet reconstruction and decorations"""

    acc = ComponentAccumulator()

    acc.merge(StandardJetsInDerivCfg(ConfigFlags))
    if "McEventCollection#GEN_EVENT" not in ConfigFlags.Input.TypedCollections:
        acc.merge(AddBadBatmanCfg(ConfigFlags))
    acc.merge(AddDistanceInTrainCfg(ConfigFlags))
    acc.merge(AddNeutralEventShapeCfg(ConfigFlags))
    acc.merge(AddEventCleanFlagsCfg(ConfigFlags))
    
    return acc


def StandardJetsInDerivCfg(ConfigFlags):
    """Jet reconstruction needed for PHYS/PHYSLITE"""

    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo_deriv,AntiKt4EMPFlow_deriv,AntiKtVR30Rmax4Rmin02PV0Track
    from JetRecConfig.StandardLargeRJets import AntiKt10UFOCSSKSoftDrop_deriv
    from JetRecConfig.JetRecConfig import JetRecCfg

    acc = ComponentAccumulator()

    jetList = [AntiKt4EMTopo_deriv, AntiKt4EMPFlow_deriv,
               AntiKtVR30Rmax4Rmin02PV0Track,
               AntiKt10UFOCSSKSoftDrop_deriv]

    for jd in jetList:
        acc.merge(JetRecCfg(ConfigFlags,jd))

    return acc

def AddBadBatmanCfg(ConfigFlags):
    """Add bad batman decoration for events with large EMEC-IW noise"""

    acc = ComponentAccumulator()

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    from DerivationFrameworkJetEtMiss.JetToolConfig import BadBatmanToolCfg
    badBatmanTool = acc.getPrimaryAndMerge(BadBatmanToolCfg(ConfigFlags))
    acc.addEventAlgo(CommonAugmentation("BadBatmanAugmentation", AugmentationTools = [badBatmanTool]))

    return acc

def AddDistanceInTrainCfg(ConfigFlags):
    """Add distance in train information to EventInfo"""
    from DerivationFrameworkJetEtMiss.JetToolConfig import DistanceInTrainToolCfg

    acc = ComponentAccumulator()

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    distanceInTrainTool = acc.getPrimaryAndMerge(DistanceInTrainToolCfg(ConfigFlags))
    acc.addEventAlgo(CommonAugmentation("DistanceInTrainAugmentation", AugmentationTools = [distanceInTrainTool]))

    return acc

def AddNeutralEventShapeCfg(ConfigFlags):
    """Special rho definitions for PFlow jets"""
    from JetRecConfig.JetRecConfig import getConstitPJGAlg
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.JetInputConfig import buildEventShapeAlg

    acc = ComponentAccumulator()

    #New rho definition using only neutral PFOs
    acc.addEventAlgo(getConstitPJGAlg(cst.GPFlow, suffix='Neut'))
    acc.addEventAlgo(buildEventShapeAlg(cst.GPFlow, '', suffix = 'Neut' ))

    return acc

def AddJvtDecorationAlgCfg(ConfigFlags, algName = "JvtPassDecorAlg", jetContainer='AntiKt4EMPFlow', **kwargs):
    acc = ComponentAccumulator()
    # Decorate if jet passed JVT criteria
    from JetJvtEfficiency.JetJvtEfficiencyToolConfig import getJvtSelToolCfg

    passJvtTool = acc.popToolsAndMerge(getJvtSelToolCfg(ConfigFlags, "{}Jets".format(jetContainer)))
    passJvtTool.PassFlagName = "DFCommonJets_passJvt"
    kwargs.setdefault("Decorators", [passJvtTool])
    kwargs.setdefault("JetContainer", "{}Jets".format(jetContainer))
    acc.addEventAlgo(CompFactory.JetDecorationAlg(algName, **kwargs), primary = True)
    return acc

def DecorateHSTP(ConfigFlags):
    """ Determine if the process is Dijet and would therefore need HSTP filtering.

        QCD multijet (dijet) simulations face an ambiguity between HS and pileup jets, 
        since both originate from the same physics process. Combined with JZ sample slicing, 
        large in-time pileup in low-pT slices can cause events to leak into higher kinematic regimes, 
        leading to unphysical normalization in the detector-level jet spectrum. 
        The Hard-Scatter Softer Than Pile-up filter requires the HS jet to have higher pT than all pileup jets, 
        restoring a physical reconstructed jet pT spectrum.
        See: https://atlas-jetetmiss.docs.cern.ch/users/QCD-samples/#hard-scatter-softer-than-pileup-hstp-filter
    """

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    from AthenaCommon.Utils.unixtools import find_datafile
    from AthenaCommon.Logging import logging
    import os
    import re

    log           = logging.getLogger('DecorateHSTP')
    dsid          = GetFileMD(ConfigFlags.Input.Files).get("mc_channel_number", 0)
    mc_campaign   = str(ConfigFlags.Input.MCCampaign)
    pmgxsec_files = []
 
    # First try database search via MC campaign
    mc_match = re.search(r"MC(\d+(?:_14TeV)?)", mc_campaign)
    if mc_match:
      mc_number = mc_match.group(1)
      file_name = find_datafile( f"JetSelectorTools/DijetMCsamples/PMGxsecDB_mc{mc_number}.txt" )
      if file_name and os.path.isfile(file_name):
        pmgxsec_files.append(file_name)
    # If MC campaign is undefined, or mc_number doesnt correespond with a file searching all available databases
    if not pmgxsec_files:
      print("here")
      dijet_samples_dir = os.path.dirname( find_datafile( "JetSelectorTools/DijetMCsamples/PMGxsecDB_mc23.txt" ) )
      pmgxsec_files = [
        os.path.join(dijet_samples_dir, file_name)
        for file_name in os.listdir(dijet_samples_dir)
        if file_name.endswith(".txt")
      ]

    # Search all selected xSecDB files for the DSID
    is_jz_sample = False
    for candidate in pmgxsec_files:
      with open(candidate, 'r', encoding="utf-8") as xsec_file:
        for line in xsec_file:
          fields = line.split()
          if fields and fields[0] == str(dsid):
            is_jz_sample = True
            break
      if is_jz_sample:
        break

    if is_jz_sample:
      log.info("DSID %s is a Dijet (JZ) process. Decorating passHSTPFilter", dsid)
    else:
      log.info("DSID %s is not a Dijet (JZ) process.", dsid)

    return is_jz_sample


def AddEventCleanFlagsCfg(ConfigFlags, workingPoints = ['Loose', 'Tight', 'LooseLLP']):
    """Add event cleaning flags"""

    acc = ComponentAccumulator()
    acc.merge(AddJvtDecorationAlgCfg(ConfigFlags, algName="JvtPassDecorAlg_EMTopo", jetContainer='AntiKt4EMTopo'))
    acc.merge(AddJvtDecorationAlgCfg(ConfigFlags, algName="JvtPassDecorAlg", jetContainer='AntiKt4EMPFlow'))

    from DerivationFrameworkTau.TauCommonConfig import AddTauAugmentationCfg
    acc.merge(AddTauAugmentationCfg(ConfigFlags, wp="GNTauLoose"))
    acc.addSequence(CompFactory.AthSequencer('EventCleanSeq', Sequential=True))

    # Overlap for EMTopo
    from AssociationUtils.AssociationUtilsConfig import OverlapRemovalToolCfg
    inputLabel_legacy = 'selected_eventClean_EMTopo'
    outputLabel_legacy = 'DFCommonJets_passOR_EMTopo'
    bJetLabel = '' #default
    tauLabel = 'DFTauGNTauLoose'
    orTool_legacy = acc.popToolsAndMerge(OverlapRemovalToolCfg(ConfigFlags,inputLabel=inputLabel_legacy,outputLabel=outputLabel_legacy,bJetLabel=bJetLabel))
    algOR_legacy = CompFactory.OverlapRemovalGenUseAlg('OverlapRemovalGenUseAlg_EMTopo',
                                                JetKey="AntiKt4EMTopoJets",
                                                SelectionLabel=inputLabel_legacy,
                                                OverlapLabel=outputLabel_legacy,
                                                OverlapRemovalTool=orTool_legacy,
                                                TauLabel=tauLabel,
                                                BJetLabel=bJetLabel
                                                )
    acc.addEventAlgo(algOR_legacy)

    # Overlap for EMPFlow
    inputLabel = 'selected_eventClean_EMPFlow'
    outputLabel = 'DFCommonJets_passOR_EMPFlow'
    orTool = acc.popToolsAndMerge(OverlapRemovalToolCfg(ConfigFlags,inputLabel=inputLabel,outputLabel=outputLabel,bJetLabel=bJetLabel))
    algOR = CompFactory.OverlapRemovalGenUseAlg('OverlapRemovalGenUseAlg',
                                                SelectionLabel=inputLabel,
                                                OverlapLabel=outputLabel,
                                                OverlapRemovalTool=orTool,
                                                TauLabel=tauLabel,
                                                BJetLabel=bJetLabel)
    acc.addEventAlgo(algOR)

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    from DerivationFrameworkMuons.MuonsToolsConfig import MuonJetDrToolCfg
    muonJetDrTool = acc.getPrimaryAndMerge(MuonJetDrToolCfg(ConfigFlags, "MuonJetDrTool"))
    acc.addEventAlgo(CommonAugmentation("DFCommonMuonsKernel2", AugmentationTools = [muonJetDrTool]))

    from JetSelectorTools.JetSelectorToolsConfig import EventCleaningToolCfg,JetCleaningToolCfg
    
    supportedWPs = ['Loose', 'Tight', 'LooseLLP', 'VeryLooseLLP', 'SuperLooseLLP', 'HSTP']
    prefix = "DFCommonJets_"
    evt_lvl_suppWPs_PFlow = ['LooseBad', 'TightBad']

    # Add support for passHSTPFilter event flag
    if DecorateHSTP(ConfigFlags):
        workingPoints.append('HSTP')
    
    for wp in workingPoints:
        if wp not in supportedWPs:
            continue
            
        cleaningLevel = wp + 'Bad'
        # LLP WPs have a slightly different name format
        if 'LLP' in wp:
            cleaningLevel = wp.replace('LLP', 'BadLLP')
        
        # Add support for TightBad event flag as well
        doEvent_PFlow=False
        for evt_swp in evt_lvl_suppWPs_PFlow:
            if evt_swp == cleaningLevel: 
                doEvent_PFlow=True
                break

        doEvent_EMTopo=False
        if 'Loose' in cleaningLevel: 
            doEvent_EMTopo=True

        ## for EMTopo (Legacy), also support for LLPs
        if doEvent_EMTopo:
            jetCleaningTool_legacy = acc.popToolsAndMerge(JetCleaningToolCfg(
                    ConfigFlags, 'JetCleaningTool_'+cleaningLevel+'_EMTopo',
                    'AntiKt4EMTopoJets', cleaningLevel, False))
            acc.addPublicTool(jetCleaningTool_legacy)
            ecTool_legacy = acc.popToolsAndMerge(EventCleaningToolCfg(
                    ConfigFlags,'EventCleaningTool_'+wp+'_EMTopo', cleaningLevel))
            ecTool_legacy.JetCleanPrefix = prefix
            ecTool_legacy.OrDecorator = "passOR_EMTopo"
            ecTool_legacy.JetContainer = "AntiKt4EMTopoJets"
            ecTool_legacy.JetCleaningTool = jetCleaningTool_legacy
            acc.addPublicTool(ecTool_legacy)
    
            eventCleanAlg_legacy = CompFactory.EventCleaningTestAlg('EventCleaningTestAlg_'+wp+'_EMTopo',
                                                             EventCleaningTool=ecTool_legacy,
                                                             JetCollectionName="AntiKt4EMTopoJets",
                                                             EventCleanPrefix=prefix,
                                                             CleaningLevel=cleaningLevel,
                                                             doEvent=True) # Only store event-level flags for Loose and LooseLLP
            acc.addEventAlgo(eventCleanAlg_legacy)

        ## For PFlow
        if doEvent_PFlow:
            jetCleaningTool = acc.popToolsAndMerge(JetCleaningToolCfg(
                    ConfigFlags, 'JetCleaningTool_'+cleaningLevel,
                    'AntiKt4EMPFlowJets', cleaningLevel, False))
            acc.addPublicTool(jetCleaningTool)
    
            ecTool = acc.popToolsAndMerge(EventCleaningToolCfg(ConfigFlags,'EventCleaningTool_' + wp, cleaningLevel))
            ecTool.JetCleanPrefix = prefix
            ecTool.OrDecorator = "passOR_EMPFlow"
            ecTool.JetContainer = "AntiKt4EMPFlowJets"
            ecTool.JetCleaningTool = jetCleaningTool
            acc.addPublicTool(ecTool)
    
            eventCleanAlg = CompFactory.EventCleaningTestAlg('EventCleaningTestAlg_'+wp,
                                                             EventCleaningTool=ecTool,
                                                             JetCollectionName="AntiKt4EMPFlowJets",
                                                             EventCleanPrefix=prefix,
                                                             CleaningLevel=cleaningLevel,
                                                             doEvent=True) # for PFlow we use Loose and Tight
            acc.addEventAlgo(eventCleanAlg)

        ## for passHSTPFilter
        if 'HSTP' in wp:

            # Decorates the decision of the Hard-Scatter Softer Than Pile-up filter, relevent ONLY for Dijet samples.
            # see: https://atlas-jetetmiss.docs.cern.ch/users/QCD-samples/#hard-scatter-softer-than-pileup-hstp-filter
            # We are forced to instatiate a jetcleaning tool to make an eventcleaning tool. This does not get used.
            ecTool = acc.popToolsAndMerge(EventCleaningToolCfg(ConfigFlags,'EventCleaningTool_' + wp, cleaningLevel))
            ecTool.JetContainer  = "AntiKt4EMPFlowJets"  # Must provide a JetContainer name.  The HSTP is a truth level only filter and wont use this.
            ecTool.DoDecorations = False
            acc.addPublicTool(ecTool)

            eventCleanAlg = CompFactory.EventCleaningTestAlg('EventCleaningTestAlg_'+wp,
                                                             EventCleaningTool=ecTool,
                                                             doEvent=False, 
                                                             doHSTPFiltering=True)
            acc.addEventAlgo(eventCleanAlg) 

    return acc


def addJetsToSlimmingTool(slimhelper,contentlist,smartlist=[]):
    for item in contentlist:
        if item not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary.update({item:'xAOD::JetContainer',
                                                  item+"Aux":'xAOD::JetAuxContainer'})
        if item in smartlist:
            slimhelper.SmartCollections.append(item)
        else:
            slimhelper.AllVariables.append(item)


##################################################################
# Helper to add origin corrected clusters to output
##################################################################
def addOriginCorrectedClustersToSlimmingTool(slimhelper,writeLC=False,writeEM=False):

    slimhelper.ExtraVariables.append('CaloCalTopoClusters.calE.calEta.calPhi.calM')

    if writeLC:
        if "LCOriginTopoClusters" not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary.update({"LCOriginTopoClusters":'xAOD::CaloClusterContainer',
                                                  "LCOriginTopoClustersAux":'xAOD::ShallowAuxContainer'})
            slimhelper.ExtraVariables.append('LCOriginTopoClusters.calEta.calPhi.originalObjectLink')

    if writeEM:
        if "EMOriginTopoClusters" not in slimhelper.AppendToDictionary:
            slimhelper.AppendToDictionary.update({"EMOriginTopoClusters":'xAOD::CaloClusterContainer',
                                                  "EMOriginTopoClustersAux":'xAOD::ShallowAuxContainer'})
            slimhelper.ExtraVariables.append('EMOriginTopoClusters.calE.calEta.calPhi.originalObjectLink')
