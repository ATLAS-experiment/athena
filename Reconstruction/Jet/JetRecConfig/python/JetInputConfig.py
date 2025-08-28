# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
# JetInputConfig: A helper module providing function to setup algorithms
# in charge of preparing input sources to jets (ex: EventDensity algo, track
# or truth selection,...)
#
# Author: P-A Delsart                                              #
"""
from AthenaConfiguration.ComponentFactory import CompFactory

# we can't add the imports here, because some modules may not be available
# in all releases (Ex: AthGeneration, AnalysisBase...) so we delay the imports
# inside the functions

def _buildJetAlgForInput(suffix, tools ):
    jetalg = CompFactory.JetAlgorithm("jetalg_"+suffix,
                                      Tools = tools,
    )
    return jetalg

def buildJetTrackUsedInFitDeco( parentjetdef, inputspec ):
    from InDetUsedInFitTrackDecoratorTool.UsedInVertexFitTrackDecoratorConfig import (
        getUsedInVertexFitTrackDecoratorAlg)
    trkProperties = parentjetdef._contextDic
    
    return getUsedInVertexFitTrackDecoratorAlg(trackCont=trkProperties["Tracks"],
                                               vtxCont= trkProperties["Vertices"])

    
def buildJetInputTruth(parentjetdef, truthmod):
    truthmod = truthmod or ""
    from ParticleJetTools.ParticleJetToolsConfig import getCopyTruthJetParticles
    return _buildJetAlgForInput("truthpartcopy_"+truthmod,
                                tools = [ getCopyTruthJetParticles(truthmod, parentjetdef._cflags) ]
    )

def buildJetInputTruthGEN(parentjetdef, truthmod):
    """  Build truth constituents as in EVTGEN jobs in the r21 config. 
    IMPORTANT : this is expected to be temporary, only to reproduce the EVTGEN r21 config with the new config. The definitions should be harmonized with reco-level at some point and this function removed.
    The source for r21 EVTGEN config was in GeneratorFilters/share/common/GenerateTruthJets.py
    """
    truthmod = truthmod or ""

    # recopy config from GeneratorFilters/share/common/GenerateTruthJets.py
    truthClassifier = CompFactory.MCTruthClassifier("JetMCTruthClassifier") 

    if truthmod == "":
        truthpartcopy = CompFactory.CopyTruthJetParticles("truthpartcopy",
                                                                   OutputName="JetInputTruthParticlesGEN",
                                                                   MCTruthClassifier=truthClassifier)
    elif truthmod=="NoWZ":
 
        truthpartcopy = CompFactory.CopyTruthJetParticles("truthpartcopywz",
                                                                     OutputName="JetInputTruthParticlesGENNoWZ",
                                                                     MCTruthClassifier=truthClassifier,
                                                                     IncludePromptLeptons=False)
        
    return _buildJetAlgForInput("truthpartcopy_"+truthmod,
                                tools = [ truthpartcopy ]
    )

def buildLabelledTruth(parentjetdef, truthmod):
    from ParticleJetTools.ParticleJetToolsConfig import getCopyTruthLabelParticles
    tool = getCopyTruthLabelParticles(truthmod)
    return _buildJetAlgForInput("truthlabelcopy_"+truthmod,
                                tools = [ tool ]
    )

def buildPV0TrackSel(parentjetdef, spec):
    from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import getTTVAToolForReco
    trkOptions = parentjetdef._contextDic
    tvaTool = getTTVAToolForReco("trackjetTVAtool", 
                                 HardScatterLinkDeco = "",
                                 WorkingPoint = "Nonprompt_All_MaxWeight",
                                 TrackContName = trkOptions['JetTracksQualityCuts']
                                 )
    alg = CompFactory.PV0TrackSelectionAlg("pv0tracksel_trackjet", 
                                           InputTrackContainer = trkOptions['JetTracksQualityCuts'],
                                           VertexContainer = trkOptions['Vertices'],
                                           OutputTrackContainer = "PV0"+trkOptions['JetTracks'],
                                           TVATool = tvaTool,
                                           )
    return alg


def buildPFlowSel(parentjetdef, spec):
    return  CompFactory.JetPFlowSelectionAlg( "pflowselalg",
                                              electronIDToExclude = "LHMedium",
                                              ChargedPFlowInputContainer  = "JetETMissChargedParticleFlowObjects",
                                              NeutralPFlowInputContainer  = "JetETMissNeutralParticleFlowObjects",
                                              ChargedPFlowOutputContainer = "GlobalChargedParticleFlowObjects",
                                              NeutralPFlowOutputContainer = "GlobalNeutralParticleFlowObjects"
                                             )

#This is to be used to seed tau jets which exclude electrons.
#Therefore it is an inclusive selection, which means we have
#to change the default settings and include all muons.
#Then we also exclude charged + neutral FE linked to electrons
#passing the relevant egamma PID WP.
def buildPFlowSel_tauSeedEleRM(parentjetdef,spec):
    return  CompFactory.JetPFlowSelectionAlg( "pflowselalg_tauSeedEleRM",
                                              electronIDToExclude = "LHMedium",
                                              ElectronInputContainer="Electrons",
                                              excludeNeutralElectronFE=True,
                                              muonIDToInclude = "Loose",
                                              excludeChargedMuonFE=False,
                                              includeChargedMuonFE=True,
                                              includeNeutralMuonFE=True,
                                              ChargedPFlowInputContainer  = "JetETMissChargedParticleFlowObjects",
                                              NeutralPFlowInputContainer  = "JetETMissNeutralParticleFlowObjects",
                                              ChargedPFlowOutputContainer = "GlobalChargedParticleFlowObjects_tauSeedEleRM",
                                              NeutralPFlowOutputContainer = "GlobalNeutralParticleFlowObjects_tauSeedEleRM"
                                             )

def buildPFlowSel_noElectrons(parentjetdef,spec):
    return  CompFactory.JetPFlowSelectionAlg( "pflowselalg_noElectrons",
                                              electronIDToExclude = "LHMedium",
                                              ElectronInputContainer="Electrons",
                                              excludeNeutralElectronFE=True,
                                              ChargedPFlowInputContainer  = "JetETMissChargedParticleFlowObjects",
                                              NeutralPFlowInputContainer  = "JetETMissNeutralParticleFlowObjects",
                                              ChargedPFlowOutputContainer = "GlobalChargedParticleFlowObjects_noElectrons",
                                              NeutralPFlowOutputContainer = "GlobalNeutralParticleFlowObjects_noElectrons"
                                             )

def buildPFlowSel_noMuons(parentjetdef,spec):
    return  CompFactory.JetPFlowSelectionAlg( "pflowselalg_noMuons",
                                              electronIDToExclude = "LHMedium",                                              
                                              muonIDToExclude = "Medium",
                                              excludeNeutralMuonFE=True,
                                              ChargedPFlowInputContainer  = "JetETMissChargedParticleFlowObjects",
                                              NeutralPFlowInputContainer  = "JetETMissNeutralParticleFlowObjects",
                                              ChargedPFlowOutputContainer = "GlobalChargedParticleFlowObjects_noMuons",
                                              NeutralPFlowOutputContainer = "GlobalNeutralParticleFlowObjects_noMuons"
                                             )

def buildPFlowSel_noLeptons(parentjetdef,spec):
    return  CompFactory.JetPFlowSelectionAlg( "pflowselalg_noLeptons",
                                              electronIDToExclude = "LHMedium",
                                              ElectronInputContainer="Electrons",
                                              excludeNeutralElectronFE=True,
                                              muonIDToExclude = "Medium",
                                              excludeNeutralMuonFE=True,
                                              ChargedPFlowInputContainer  = "JetETMissChargedParticleFlowObjects",
                                              NeutralPFlowInputContainer  = "JetETMissNeutralParticleFlowObjects",
                                              ChargedPFlowOutputContainer = "GlobalChargedParticleFlowObjects_noLeptons",
                                              NeutralPFlowOutputContainer = "GlobalNeutralParticleFlowObjects_noLeptons"
                                             )

########################################################################

def buildEventShapeAlg(jetOrConstitdef, inputspec, voronoiRf = 0.9, radius = 0.4, suffix = None ):
    """Function producing an EventShapeAlg to calculate
     median energy density for pileup correction"""

    from .JetRecConfig import getPJContName
    from EventShapeTools.EventDensityConfig import configEventDensityTool, getEventShapeName

    
    pjContName = getPJContName(jetOrConstitdef,suffix)
    nameprefix = inputspec or ""
    rhotool = configEventDensityTool(
        f"EventDensity_{nameprefix}Kt4{pjContName}",
        jetOrConstitdef,
        InputContainer = pjContName, 
        OutputContainer = getEventShapeName(jetOrConstitdef, nameprefix=nameprefix, suffix=suffix, radius=radius),
        JetRadius = radius,
        VoronoiRfact = voronoiRf,
        )
    
    eventshapealg = CompFactory.EventDensityAthAlg(
        f"EventDensity_{nameprefix}Kt4{pjContName}Alg",
        EventDensityTool = rhotool )

    return eventshapealg

