# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

########################################################################
#                                                                      #
# ParticleJetToolsConfig: A helper module for configuring tools for    #
# truth jet reconstruction and classification                          #
# Author: TJ Khoo                                                      #
#                                                                      #
########################################################################

from AthenaCommon import Logging
jrtlog = Logging.logging.getLogger('ParticleJetToolsConfig')

from AthenaConfiguration.ComponentFactory import CompFactory
# workaround for missing JetRecConfig in AthAnalysis
try:
    from JetRecConfig.JetRecConfig import isAnalysisRelease
except ModuleNotFoundError:
    def isAnalysisRelease():
        return True

# Putting MCTruthClassifier here as we needn't stick jet configs in really foreign packages
def getMCTruthClassifier():
    # Assume mc15 value
    truthclassif = CompFactory.MCTruthClassifier(
        "JetMCTruthClassifier"
        )
    if not isAnalysisRelease() :
        truthclassif.xAODTruthLinkVector= ""
    # Config neessary only for Athena releases
    import os
    if "AtlasProject" in os.environ.keys():
        if os.environ["AtlasProject"] in ["Athena","AthDerivation"]:
            truthclassif.ParticleCaloExtensionTool=""
    return truthclassif

# Generates truth particle containers for truth labeling
truthpartoptions = {
    "Partons":{"ToolType":CompFactory.CopyTruthPartons,"ptmin":5000},
    "BosonTop":{"ToolType":CompFactory.CopyBosonTopLabelTruthParticles,"ptmin":100000},
    "FlavourLabel":{"ToolType":CompFactory.CopyFlavorLabelTruthParticles,"ptmin":5000},
}
def getCopyTruthLabelParticles(truthtype):
    toolProperties = {}
    if truthtype == "Partons":
        truthcategory = "Partons"
    elif truthtype in ["WBosons", "ZBosons", "HBosons", "TQuarksFinal"]:
        truthcategory = "BosonTop"
        toolProperties['ParticleType'] = truthtype
    else:
        truthcategory = "FlavourLabel"
        toolProperties['ParticleType'] = truthtype

    tooltype = truthpartoptions[truthcategory]["ToolType"]
    toolProperties.update( PtMin = truthpartoptions[truthcategory]["ptmin"],
                           OutputName = "TruthLabel"+truthtype)
    ctp = tooltype("truthpartcopy_"+truthtype,
                   **toolProperties
                   )
    return ctp

# Generates input truth particle containers for truth jets
def getCopyTruthJetParticles(modspec, cflags):
    truthclassif = getMCTruthClassifier()

    truthpartcopy = CompFactory.CopyTruthJetParticles(
        "truthpartcopy"+modspec,
        OutputName="JetInputTruthParticles"+modspec,
        MCTruthClassifier=truthclassif)
    if modspec=="NoWZ":
        truthpartcopy.IncludePromptLeptons=False
        truthpartcopy.IncludePromptPhotons=False
        truthpartcopy.IncludeMuons=True
        truthpartcopy.IncludeNeutrinos=True
    if modspec=="DressedWZ":
        truthpartcopy.IncludePromptLeptons=False
        truthpartcopy.IncludePromptPhotons=True
        truthpartcopy.IncludeMuons=True
        truthpartcopy.IncludeNeutrinos=True
        truthpartcopy.DressingDecorationNames=['TruthParticles.dressedPhoton_e','TruthParticles.dressedPhoton_mu']
        ### Declare the dependency on the photon dressing. Needed to run the tool with avalanche scheduler
        truthpartcopy.ExtraInputs = {( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+TruthParticles.dressedPhoton_e' ),
                                     ( 'xAOD::TruthParticleContainer' , 'StoreGateSvc+TruthParticles.dressedPhoton_mu' )}
    if modspec=="Charged":
        truthpartcopy.ChargedParticlesOnly=True
    return truthpartcopy


def _getCommonLabelNames(prefix):
    """Internal unlity to name labels

    Returns a dictionary to configure labeling tools. Takes one
    argument which is prefixed to each label.
    """
    return dict(
        LabelName=f"{prefix}TruthLabelID",
        DoubleLabelName=f"{prefix}ExtendedTruthLabelID",
        LabelPtName=f"{prefix}TruthLabelPt",
        LabelLxyName=f"{prefix}TruthLabelLxy",
        LabelDRName=f"{prefix}TruthLabelDR",
        LabelPdgIdName=f"{prefix}TruthLabelPdgId",
        LabelPositionDPhiName=f"{prefix}TruthLabelPositionDPhi",
        LabelPositionDEtaName=f"{prefix}TruthLabelPositionDEta",
        LabelBarcodeName=f"{prefix}TruthLabelBarcode",
        ChildLxyName=f"{prefix}TruthLabelChildLxy",
        ChildPtName=f"{prefix}TruthLabelChildPt",
        ChildPdgIdName=f"{prefix}TruthLabelChildPdgId",
        ChildPositionDPhiName=f"{prefix}TruthLabelChildPositionDPhi",
        ChildPositionDEtaName=f"{prefix}TruthLabelChildPositionDEta",
    )


def getJetDeltaRFlavorLabelTool(name='jetdrlabeler', jet_pt_min=5000, collection="Final", dr_max=0.3):
    """Get the standard flavor tagging delta-R labeling tool

    Uses cone matching to B, C and tau truth particles.
    """
    prefix_to_name = "HadronConeExcl"
    if collection != "Final":
        prefix_to_name += collection
        name+=collection

    return CompFactory.ParticleJetDeltaRLabelTool(
        name,
        **_getCommonLabelNames(prefix_to_name),
        BLabelName = "ConeExclBHadrons"+collection,
        CLabelName = "ConeExclCHadrons"+collection,
        TauLabelName = "ConeExclTausFinal",
        BParticleCollection = "TruthLabelBHadrons"+collection,
        CParticleCollection = "TruthLabelCHadrons"+collection,
        TauParticleCollection = "TruthLabelTausFinal",
        PartPtMin = 5000.,
        DRMax = dr_max,
        MatchMode = "MinDR",
        JetPtMin = jet_pt_min,
        )


def getJetDeltaRLabelTool(jetdef, modspec):
    """returns a ParticleJetDeltaRLabelTool
    Cone matching for B, C and tau truth for all but track jets.

    This function is meant to be used as callback from JetRecConfig where
    it is called as func(jetdef, modspec). Hence the jetdef argument even if not used in this case.
    """
    jetptmin = float(modspec)
    name = "jetdrlabeler_jetpt{0}GeV".format(int(jetptmin/1000))
    return getJetDeltaRFlavorLabelTool(name, jetptmin)

def getJetDeltaRInitialLabelTool(jetdef, modspec):
    """returns a ParticleJetDeltaRLabelTool
    Cone matching for B, C and tau truth for all but track jets.

    This function is meant to be used as callback from JetRecConfig where
    it is called as func(jetdef, modspec). Hence the jetdef argument even if not used in this case.
    """
    jetptmin = float(modspec)
    name = "jetdrlabeler_jetpt{0}GeV".format(int(jetptmin/1000))
    return getJetDeltaRFlavorLabelTool(name, jetptmin, collection = "Initial")


def getJetGhostFlavorLabelTool(name="jetghostlabeler", collection="Final"):

    prefix_to_name = "HadronGhost"
    if collection != "Final":
        prefix_to_name += collection
        name+=collection
    return CompFactory.ParticleJetGhostLabelTool(
        name,
        **_getCommonLabelNames(prefix_to_name),
        GhostBName = "GhostBHadrons"+collection,
        GhostCName = "GhostCHadrons"+collection,
        GhostTauName = "GhostTausFinal",
        PartPtMin = 5000.0
    )

def getJetGhostInitialLabelTool(jetdef, modspec):
    """get ghost-based flavor tagging labeling

    This is a wrapper for JetRecConfig where it's called as
    func(jetdef, modspec)
    """
    return getJetGhostFlavorLabelTool(modspec,collection = "Initial")

def getJetGhostLabelTool(jetdef, modspec):
    """get ghost-based flavor tagging labeling

    This is a wrapper for JetRecConfig where it's called as
    func(jetdef, modspec)
    """
    return getJetGhostFlavorLabelTool(modspec)


def getJetTruthLabelTool(jetdef, modspec):

    isTruthJet = 'Truth' in jetdef.fullname()

    if not isinstance(modspec, str):
        raise ValueError("JetTruthLabelingTool can only be scheduled with str as modspec")
    else:
        truthLabel = str(modspec)

    jetTruthLabelTool = CompFactory.JetTruthLabelingTool('truthlabeler_{0}'.format(truthLabel),
                                                         RecoJetContainer = jetdef.fullname(),
                                                         IsTruthJetCollection = isTruthJet,
                                                         TruthLabelName = truthLabel)

    return jetTruthLabelTool

def getJetTruthLabelToolPrereqs(jetdef, modspec):
    return ["input:AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets"] if modspec == "R10WZTruthLabel_R22v1" and jetdef._cflags.Input.isMC else []

def getJetPileupLabelTool(jetdef, modspec):

    jetPileupLabelTool = CompFactory.JetPileupLabelingTool('pileuplabeler',
                                                           RecoJetContainer = jetdef.fullname(),
                                                           TruthJetContainer= "AntiKt4TruthDressedWZJets")

    return jetPileupLabelTool
