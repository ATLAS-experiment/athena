# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from JetRecConfig.StandardJetConstits import stdConstitDic as cst
from JetRecConfig.JetRecConfig import registerAsInputConstit
from .JetDefinition import  JetDefinition


# *********************************************************
# Ghost-associated particles for the standard small R jets 
# *********************************************************
standardghosts =  ["Track","MuonSegment","Truth","Tower"]


flavourghosts = [ "BHadronsInitial", "BHadronsFinal", "BQuarksFinal",
                  "CHadronsInitial", "CHadronsFinal", "CQuarksFinal",
                  "TausFinal",
                  "WBosons", "ZBosons", "HBosons", "TQuarksFinal",
                  "Partons",]





# *********************************************************
# Modifiers for the standard small R jets 
# *********************************************************
# (use tuples rather than lists to prevent accidental modification)
calibmods = (
    "ConstitFourMom",   "CaloEnergies",
    "Calib:T0:mc",
    "Sort",
    )

calibmods_lowCut = (
    "ConstitFourMom", "CaloEnergies",
    "Calib:T0:mc:JetArea_Residual",
    "Sort",
)

calibmods_noCut = (
    "ConstitFourMom", "CaloEnergies", "Sort"
)

standardmods = (
    "Width",
    "CaloQuality", "TrackMoments","TrackSumMoments",
    "JVF", "JVT", "Charge",
)

standardmods_ufo = (
    "Width",
    "CaloQualityFE", "TrackMoments","TrackSumMoments",
    "JVF", "JVT", "Charge",
)

clustermods      = ("ECPSFrac","ClusterMoments",) 
truthmods        = ("PartonTruthLabel","TruthPartonDR","JetDeltaRLabel:5000")
pflowmods        = ()


# ********************************************************
# Standard track jet definition
# ********************************************************
AntiKtVR30Rmax4Rmin02PV0Track = JetDefinition("AntiKt", 0.4, cst.PV0Track,
                                              modifiers = ("Sort","JetDeltaRLabel:4500","JetGhostLabel","vr"),
                                              ptmin=4000,
                                              VRMinR = 0.02,
                                              VRMassSc = 30000,
                                              lock = True)

# These jets are used as ghost, so they also need to be defined as constituents : 
registerAsInputConstit(AntiKtVR30Rmax4Rmin02PV0Track)


AntiKt4PV0Track = JetDefinition("AntiKt", 0.4, cst.PV0Track,
                                modifiers = ("Sort",)+truthmods,
                                ptmin=2000,
                                  lock = True)


# *********************************************************
# Standard small R reco jet definitions
# *********************************************************

AntiKt4EMPFlow = JetDefinition("AntiKt",0.4,cst.GPFlow,
                               ghostdefs = standardghosts+flavourghosts,
                               modifiers = calibmods+truthmods+standardmods+("Filter_calibThreshold:10000","JetGhostLabel","LArHVCorr"),
                               lock = True
)




AntiKt4LCTopo = JetDefinition("AntiKt",0.4,cst.LCTopoOrigin,
                              ghostdefs = standardghosts+flavourghosts, 
                              modifiers = calibmods+("Filter_ifnotESD:15000","OriginSetPV","LArHVCorr")+standardmods+clustermods,
                              lock = True,
)



AntiKt4EMTopo = JetDefinition("AntiKt",0.4,cst.EMTopoOrigin,
                              ghostdefs = standardghosts+["TrackLRT"]+flavourghosts,
                              modifiers = calibmods+truthmods+standardmods+clustermods+("Filter_calibThreshold:15000","LArHVCorr",),
                              lock = True,
)


# *********************************************************
# EMPFlow CSSK jets  (no jet calibration available yet,
# thus applying only low pT filter)
# *********************************************************
AntiKt4EMPFlowCSSK = JetDefinition("AntiKt",0.4,cst.GPFlowCSSK,
                                   ghostdefs = standardghosts+flavourghosts,
                                   modifiers = ("ConstitFourMom","CaloEnergies","Sort","Filter:1","JetPtAssociation","LArHVCorr")+truthmods+standardmods,
                                   ptmin = 2000,
                                   lock = True
)

# *********************************************************
# UFO CSSK jets
# *********************************************************
AntiKt4UFOCSSK = JetDefinition("AntiKt",0.4,cst.UFOCSSK,
                               ghostdefs = standardghosts+flavourghosts,
                               modifiers = calibmods+("Filter_calibThreshold:10000","EMScaleMom","JetPtAssociation","CaloEnergiesClus","JetGhostLabel")+truthmods+standardmods_ufo,
                               lock = True
)

# *********************************************************
# Low and no pT cut containers used in JETMX derivations
# *********************************************************
AntiKt4UFOCSSKNoPtCut = JetDefinition("AntiKt",0.4,cst.UFOCSSK,
                                      infix = "NoPtCut",
                                      ghostdefs = standardghosts+flavourghosts,
                                      modifiers = calibmods_noCut+("Filter:1","EMScaleMom","JetPtAssociation","CaloEnergiesClus",)+truthmods+standardmods_ufo,
                                      ptmin = 1,
                                      lock = True
)

AntiKt4EMPFlowCSSKNoPtCut = JetDefinition("AntiKt",0.4,cst.GPFlowCSSK,
                                          infix = "NoPtCut",
                                          ghostdefs = standardghosts+flavourghosts,
                                          modifiers = ("ConstitFourMom","CaloEnergies","Sort","Filter:1","JetPtAssociation")+truthmods+standardmods,
                                          ptmin = 1,
                                          lock = True
)

AntiKt4EMPFlowNoPtCut = JetDefinition("AntiKt",0.4,cst.GPFlow,
                                      infix = "NoPtCut",
                                      ghostdefs = standardghosts+flavourghosts,
                                      modifiers = calibmods_lowCut+("Filter:1",)+truthmods+standardmods+("JetPtAssociation","CaloEnergiesClus"),
                                      ptmin = 1,
                                      lock = True
)

AntiKt4EMPFlowByVertex = JetDefinition("AntiKt", 0.4, cst.GPFlowByVtx,
                                        ghostdefs = standardghosts+flavourghosts,
                                        modifiers = calibmods_lowCut+("Filter:1",)+truthmods+standardmods+("JetPtAssociation","CaloEnergiesClus"),
                                        ptmin = 7000,
                                        lock = True,
                                        byVertex = True
)

AntiKt4EMTopoNoPtCut = JetDefinition("AntiKt",0.4,cst.EMTopoOrigin,
                                     infix = "NoPtCut",
                                     ghostdefs = standardghosts+flavourghosts,
                                     modifiers = calibmods_lowCut+("Filter:1",)+truthmods+standardmods+clustermods+("JetPtAssociation",),
                                     ptmin = 1,
                                     lock = True
)

AntiKt4UFOCSSKLowPt = JetDefinition("AntiKt",0.4,cst.UFOCSSK,
                                    infix = "LowPt",
                                    ghostdefs = standardghosts+flavourghosts,
                                    modifiers = calibmods_noCut+("Filter:7000","EMScaleMom","JetPtAssociation","CaloEnergiesClus",)+truthmods+standardmods_ufo,
                                    ptmin = 1,
                                    lock = True
)

AntiKt4EMPFlowLowPt = JetDefinition("AntiKt",0.4,cst.GPFlow,
                                    infix = "LowPt",
                                    ghostdefs = standardghosts+flavourghosts,
                                    modifiers = calibmods_lowCut+("Filter:2000",)+truthmods+standardmods+("JetPtAssociation",),
                                    ptmin = 2000,
                                    lock = True
)

AntiKt4EMTopoLowPt = JetDefinition("AntiKt",0.4,cst.EMTopoOrigin,
                                   infix = "LowPt",
                                   ghostdefs = standardghosts+flavourghosts,
                                   modifiers = calibmods_lowCut+("Filter:2000",)+truthmods+standardmods+clustermods+("JetPtAssociation",),
                                   ptmin = 2000,
                                   lock = True
)

# *********************************************************
# Standard small R truth jet definitions
# *********************************************************

AntiKt4Truth = JetDefinition("AntiKt",0.4, cst.Truth,
                             ghostdefs = flavourghosts,
                             modifiers = ("Sort", "Width")+truthmods,
                             lock = True,
)

AntiKt2Truth = JetDefinition("AntiKt",0.2, cst.Truth,
                             ghostdefs = flavourghosts,
                             modifiers = ("Sort", "Width")+truthmods,
                             lock = True,
)

AntiKt4TruthWZ = JetDefinition("AntiKt",0.4, cst.TruthWZ,
                               ghostdefs = flavourghosts,
                               modifiers = ("Sort", "Width")+truthmods,
                               lock = True,
)

AntiKt4TruthDressedWZ = JetDefinition("AntiKt",0.4, cst.TruthDressedWZ,
                                      ghostdefs = flavourghosts,
                                      modifiers = ("Sort", "Width")+truthmods,
                                      lock = True,
)

# These jets are used as input for the JetPileupLabel modifier, so they also need to be defined as constituents: 
registerAsInputConstit(AntiKt4TruthDressedWZ)

AntiKtVRTruthCharged = JetDefinition("AntiKt",0.4, cst.TruthCharged,
                                     ghostdefs = flavourghosts,
                                     modifiers = ("Sort",)+truthmods,
                                     VRMinR = 0.02,
                                     VRMassSc = 30000,
                                     lock = True
)

AntiKt4TruthGEN = JetDefinition("AntiKt",0.4, cst.TruthGEN,
                                ptmin = 5000, 
                                ghostdefs = [],
                                modifiers = ("Sort", )+truthmods,
                                ghostarea = 0.,
                                lock = True,
)
AntiKt4TruthGENWZ = AntiKt4TruthGEN.clone(inputdef=cst.TruthGENWZ)

AntiKt6TruthGEN   = AntiKt4TruthGEN.clone(radius=0.6)
AntiKt6TruthGENWZ = AntiKt4TruthGENWZ.clone(radius=0.6)


def StandardSmallRJetCfg(flags):
    """Top-level function to schedule the smallR jets in standard reconstruction """
    from JetRecConfig.JetRecConfig import JetRecCfg

    standarSmallRList = [
        AntiKt4EMPFlow,
        AntiKt4LCTopo,
        AntiKt4Truth,
        ]

    compacc = JetRecCfg( flags, standarSmallRList[0], )
    for jetdef in standarSmallRList[1:]:
        compacc.merge( JetRecCfg( flags, jetdef) )

    return compacc
        
