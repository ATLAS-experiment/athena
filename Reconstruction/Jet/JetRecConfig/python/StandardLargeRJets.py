# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from JetRecConfig.StandardJetConstits import stdConstitDic as cst
from .JetDefinition import  JetDefinition
from .JetGrooming import  JetTrimming, JetSoftDrop
from JetRecConfig.JetRecConfig import registerAsInputConstit

# needed to ensure the smallR VR jets are defined
import JetRecConfig.StandardSmallRJets # noqa: F401

# *********************************************************
# Ghost-associated particles for the standard large R jets 
# *********************************************************
standardghosts =  ["Track","MuonSegment","Truth"]


flavourghosts = [ "BHadronsFinal","CHadronsFinal",
                  "WBosons", "ZBosons", "HBosons", "TQuarksFinal",
                  "Partons"]





# *********************************************************
# Modifiers for the standard large R jets 
# *********************************************************
# (use tuples rather than lists to prevent accidental modification)
standardrecomods = (
    "Sort",
    "Width",
    "ConstitFourMom"
)

ufo_dnn_moments = ("CaloEnergiesLargeR","ConstitFrac","groomMRatio")

clustermods      = ("ECPSFrac","ClusterMoments",) 
truthmods        = ("PartonTruthLabel",)
pflowmods        = ()

truthlabels = ("JetTaggingTruthLabel:R10TruthLabel_R21Consolidated","JetTaggingTruthLabel:R10TruthLabel_R21Precision","JetTaggingTruthLabel:R10TruthLabel_R21Precision_2022v1","JetTaggingTruthLabel:R10TruthLabel_R22v1")
truthlabels_SD = ("JetTaggingTruthLabel:R10TruthLabel_R21Precision","JetTaggingTruthLabel:R10TruthLabel_R21Precision_2022v1","JetTaggingTruthLabel:R10TruthLabel_R22v1", "JetTaggingTruthLabel:R10WZTruthLabel_R22v1")

substrmods = ("nsubjettiness", "nsubjettinessR", "ktsplitter",
              "ecorr", "ecorrR", "qw",
              # ... others ?
)

#Variables used for trimmed large-R jets
lctopo_trimmed_mods = ("planarflow","angularity","comshapes","ktdr","TrackSumMoments","softdropobs")
ufo_softdrop_mods = ("planarflow","angularity","comshapes","ktdr","ecorrgeneral","ecorrgeneralratios","softdropobs")

# *********************************************************
# Standard large R jets definitions
# *********************************************************


AntiKt10LCTopo = JetDefinition("AntiKt",1.0,cst.LCTopoOrigin,
                               ghostdefs = standardghosts+flavourghosts+["AntiKtVR30Rmax4Rmin02PV0TrackJets"] ,
                               modifiers = ("Sort", "Filter:50000","TrackMoments","JetGhostLabel"),
                               standardRecoMode = True,
                               ghostarea = 0.,
                               lock = True
)

#Remove VR track jets from ghosts for core reco
AntiKt10LCTopo_noVR = AntiKt10LCTopo.clone(
    ghostdefs = standardghosts+flavourghosts
)

AntiKt10LCTopo_withmoms = AntiKt10LCTopo.clone(
    modifiers = ("Sort", "Filter:50000", "Width", "TrackMoments", "TrackSumMoments","JetDeltaRLabel:5000")+clustermods+truthmods,
    # NOT all moments from old AntiKt10LCTopo config here yet. 
)

AntiKt10LCTopoTrimmed = JetTrimming(AntiKt10LCTopo,
                                    #A filter of 1 GeV is applied before the calibration to remove jets with zero constituents after trimming
                                    modifiers = ("Filter:1000","Calib:CombinedMass:mc","Filter:100000",)+standardrecomods+substrmods+lctopo_trimmed_mods+truthlabels,
                                    PtFrac = 0.05, RClus = 0.2,                                    
                                    )

AntiKt10LCTopoTrimmed_trigger = JetTrimming(AntiKt10LCTopo_noVR,
                                            #A filter of 1 GeV is applied before the calibration to remove jets with zero constituents after trimming
                                            modifiers = ("Filter:1000","Calib:CombinedMass:mc","Filter:50000","Sort","ConstitFourMom"),
                                            PtFrac = 0.05, RClus = 0.2,
)

AntiKt10LCTopoSoftDrop = JetSoftDrop(AntiKt10LCTopo,
                                     modifiers = standardrecomods+substrmods,
                                     Beta = 1., ZCut= 0.1,
                                     )


AntiKt10UFOCHS = JetDefinition("AntiKt",1.0,cst.UFO,
                               standardRecoMode = True)


AntiKt10UFOCSSK = JetDefinition("AntiKt",1.0,cst.UFOCSSK,
                                ghostdefs = standardghosts+flavourghosts+["AntiKtVR30Rmax4Rmin02PV0TrackJets"] ,
                                modifiers = ("Sort", "Filter:50000","TrackMoments","JetGhostLabel","PartonTruthLabel"),
                                standardRecoMode = True,
                                ghostarea = 0.,                                
                                )

AntiKt10UFOCSSK_noElectrons = JetDefinition("AntiKt",1.0,cst.UFOCSSK_noElectrons,
                                            ghostdefs = standardghosts+flavourghosts+["AntiKtVR30Rmax4Rmin02PV0TrackJets"],
                                            modifiers = ("Sort", "Filter:50000","TrackMoments","JetGhostLabel","PartonTruthLabel"),
                                            standardRecoMode = True,
                                            ghostarea = 0.,                                            
                                            )

AntiKt10UFOCSSK_noMuons = JetDefinition("AntiKt",1.0,cst.UFOCSSK_noMuons,
                                        ghostdefs = standardghosts+flavourghosts+["AntiKtVR30Rmax4Rmin02PV0TrackJets"],
                                        modifiers = ("Sort", "Filter:50000","TrackMoments","JetGhostLabel","PartonTruthLabel"),
                                        standardRecoMode = True,
                                        ghostarea = 0.,                                        
                                        )

AntiKt10UFOCSSK_noLeptons = JetDefinition("AntiKt",1.0,cst.UFOCSSK_noLeptons,
                                          ghostdefs = standardghosts+flavourghosts+["AntiKtVR30Rmax4Rmin02PV0TrackJets"],
                                          modifiers = ("Sort", "Filter:50000","TrackMoments","JetGhostLabel","PartonTruthLabel"),
                                          standardRecoMode = True,
                                          ghostarea = 0.,                                          
                                          )

AntiKt10UFOCSSKSoftDrop = JetSoftDrop(AntiKt10UFOCSSK,
                                      #A filter of 1 GeV is applied before the calibration to remove jets with zero constituents after the soft drop alg is applied
                                      modifiers = ("Filter:1000","Calib:SoftDrop:mc","Filter:100000","JetGhostLabel")+standardrecomods+substrmods+ufo_softdrop_mods+truthlabels_SD+ufo_dnn_moments,
                                      Beta = 1., ZCut= 0.1,
                                      )

AntiKt10UFOCSSKSoftDrop_noElectrons = JetSoftDrop(AntiKt10UFOCSSK_noElectrons,
                                                  #A filter of 1 GeV is applied before the calibration to remove jets with zero constituents after the soft drop alg is applied
                                                  modifiers = ("Filter:1000","Calib:SoftDrop:mc","Filter:100000","Sort","JetGhostLabel")+standardrecomods+substrmods+ufo_softdrop_mods+truthlabels_SD+ufo_dnn_moments,
                                                  Beta = 1., ZCut= 0.1,
                                                  )

AntiKt10UFOCSSKSoftDrop_noMuons = JetSoftDrop(AntiKt10UFOCSSK_noMuons,
                                              #A filter of 1 GeV is applied before the calibration to remove jets with zero constituents after the soft drop alg is applied
                                              modifiers = ("Filter:1000","Calib:SoftDrop:mc","Filter:100000","Sort","JetGhostLabel")+standardrecomods+substrmods+ufo_softdrop_mods+truthlabels_SD+ufo_dnn_moments,
                                              Beta = 1., ZCut= 0.1,
                                              )

AntiKt10UFOCSSKSoftDrop_noLeptons = JetSoftDrop(AntiKt10UFOCSSK_noLeptons,
                                                #A filter of 1 GeV is applied before the calibration to remove jets with zero constituents after the soft drop alg is applied
                                                modifiers = ("Filter:1000","Calib:SoftDrop:mc","Filter:100000","Sort","JetGhostLabel")+standardrecomods+substrmods+ufo_softdrop_mods+truthlabels_SD+ufo_dnn_moments,
                                                Beta = 1., ZCut= 0.1,
                                                )

AntiKt10UFOCSSKSoftDrop_trigger = JetSoftDrop(AntiKt10UFOCSSK,
                                              #A filter of 1 GeV is applied before the calibration to remove jets with zero constituents after the soft drop alg is applied
                                              modifiers = ("Filter:1000","Calib:SoftDrop:mc","Filter:50000","Sort","ConstitFourMom"),
                                              Beta = 1., ZCut= 0.1,
                                              )



AntiKt10Truth = JetDefinition("AntiKt",1.0,cst.Truth,
                               ghostdefs = flavourghosts , 
                               modifiers = ("Sort", "Filter:50000","ktsplitter","JetGhostLabel"),
                               standardRecoMode = True,
                               ghostarea = 0.,                               
                               lock = True
)

AntiKt10TruthTrimmed = JetTrimming(AntiKt10Truth,
                                   modifiers = ("Sort","JetGhostLabel")+substrmods+truthmods,
                                   PtFrac = 0.05, RClus = 0.2,                                    
                                   )

AntiKt10TruthSoftDrop = JetSoftDrop(AntiKt10Truth,
                                    modifiers = ("Sort","JetGhostLabel")+substrmods+truthmods,
                                    Beta = 1., ZCut= 0.1,
                                    )


AntiKt10TruthWZ = JetDefinition("AntiKt",1.0, cst.TruthWZ,
                                ghostdefs = flavourghosts,
                                modifiers = ("Sort", "Filter:50000","ktsplitter","JetGhostLabel"),
                                standardRecoMode = True,
                                ghostarea = 0.,                                
                                lock = True,
)

AntiKt10TruthWZSoftDrop = JetSoftDrop(AntiKt10TruthWZ,
                                      modifiers = ("Sort","JetGhostLabel")+substrmods+truthmods,
                                      Beta = 1., ZCut= 0.1,
)

AntiKt10TruthDressedWZ = JetDefinition("AntiKt",1.0,cst.TruthDressedWZ,
                                       ghostdefs = flavourghosts ,
                                       modifiers = ("Sort", "Filter:50000","ktsplitter","JetGhostLabel"),
                                       standardRecoMode = True,
                                       lock = True
)

AntiKt10TruthDressedWZSoftDrop = JetSoftDrop(AntiKt10TruthDressedWZ,
                                             modifiers = ("Sort","JetGhostLabel")+substrmods+truthmods,
                                             Beta = 1., ZCut= 0.1,
)

# These jets may be used as input for the JetTruthLabelling, so they also need to be defined as constituents:
registerAsInputConstit(AntiKt10TruthDressedWZSoftDrop)

AntiKt10TruthGEN = JetDefinition("AntiKt",1.0, cst.TruthGEN,
                                ptmin = 5000, 
                                ghostdefs = [],
                                modifiers = ("Sort", )+truthmods,
                                ghostarea = 0.,
                                lock = True,
)
AntiKt10TruthGENWZ = AntiKt10TruthGEN.clone(inputdef=cst.TruthGENWZ)






        
