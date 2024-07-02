#====================================================================
# JETM9.py 
# reductionConf flag JETM9 in Reco_tf.py   
#====================================================================

from DerivationFrameworkCore.DerivationFrameworkMaster import *
from DerivationFrameworkInDet.InDetCommon import *
from DerivationFrameworkJetEtMiss.JetCommon import *
from DerivationFrameworkJetEtMiss.ExtendedJetCommon import *
#from DerivationFrameworkJetEtMiss.METCommon import *

#====================================================================
# SKIMMING TOOL 
#====================================================================
from DerivationFrameworkJetEtMiss import TriggerLists
triggers = TriggerLists.jetTrig()

# NOTE: need to be able to OR isSimulated as an OR with the trigger
#orstr =' || '
#trigger = '('+orstr.join(triggers)+')'
expression = 'EventInfo.eventTypeBitmask==1'


from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__TriggerSkimmingTool
JETM9TrigSkimmingTool = DerivationFramework__TriggerSkimmingTool(   name                    = "JETM9TrigSkimmingTool1",
                                                                TriggerListOR          = triggers )
ToolSvc += JETM9TrigSkimmingTool

from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__xAODStringSkimmingTool
JETM9OfflineSkimmingTool = DerivationFramework__xAODStringSkimmingTool(name = "JETM9OfflineSkimmingTool1",
                                                                    expression = expression)
ToolSvc += JETM9OfflineSkimmingTool

# OR of the above two selections
from DerivationFrameworkTools.DerivationFrameworkToolsConf import DerivationFramework__FilterCombinationOR
JETM9ORTool = DerivationFramework__FilterCombinationOR(name="JETM9ORTool", FilterList=[JETM9TrigSkimmingTool,JETM9OfflineSkimmingTool] )
ToolSvc+=JETM9ORTool

#=======================================
# CREATE PRIVATE SEQUENCE
#=======================================

jetm9Seq = CfgMgr.AthSequencer("JETM9Sequence")
DerivationFrameworkJob += jetm9Seq
#jetm9Seq = DerivationFrameworkJob

#=======================================
# RESTORE AOD-REDUCED JET COLLECTIONS
#=======================================
reducedJetList = ["AntiKt2PV0TrackJets",
                  "AntiKt4PV0TrackJets",
                  "AntiKt4TruthJets"]
replaceAODReducedJets(reducedJetList,jetm9Seq,"JETM9")

OutputJets["JETM9"] = ["AntiKt4EMTopoJets","AntiKt4EMPFlowJets","AntiKt4TruthJets"]

#=======================================
# Track thinning
#=======================================

thinningTools = []

# Inner detector group recommendations for indet tracks in analysis
JETM9_thinning = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__TrackParticleThinning
JETM9TrackParticleThinningTool = DerivationFramework__TrackParticleThinning(name                    = "JETM9TrackParticleThinningTool",
                                                                            ThinningService         = "JETM9ThinningSvc",
                                                                            SelectionString         = JETM9_thinning,
                                                                            InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                            ApplyAnd                = False)

ToolSvc += JETM9TrackParticleThinningTool
thinningTools.append(JETM9TrackParticleThinningTool)

# Include inner detector tracks associated with muons
from DerivationFrameworkInDet.DerivationFrameworkInDetConf import DerivationFramework__MuonTrackParticleThinning
JETM9MuonTPThinningTool = DerivationFramework__MuonTrackParticleThinning(name                    = "JETM9MuonTPThinningTool",
                                                                         ThinningService         = "JETM9ThinningSvc",
                                                                         MuonKey                 = "Muons",
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles",
                                                                         ApplyAnd = False)
ToolSvc += JETM9MuonTPThinningTool
thinningTools.append(JETM9MuonTPThinningTool)

from DerivationFrameworkCore.DerivationFrameworkCoreConf import DerivationFramework__DerivationKernel
jetm9Seq += CfgMgr.DerivationFramework__DerivationKernel(       name = "JETM9Kernel",
                                                                SkimmingTools = [JETM9ORTool],
                                                                ThinningTools = thinningTools)

#====================================================================
# SET UP STREAM   
#====================================================================
streamName = derivationFlags.WriteDAOD_JETM9Stream.StreamName
fileName   = buildFileName( derivationFlags.WriteDAOD_JETM9Stream )
JETM9Stream = MSMgr.NewPoolRootStream( streamName, fileName )
JETM9Stream.AcceptAlgs(["JETM9Kernel"])
# for thinning
from AthenaServices.Configurables import ThinningSvc, createThinningSvc
augStream = MSMgr.GetStream( streamName )
evtStream = augStream.GetEventStream()
svcMgr += createThinningSvc( svcName="JETM9ThinningSvc", outStreams=[evtStream] )

#====================================================================
# Jets for R-scan 
#====================================================================
for radius in [0.2, 0.3, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]:
    if jetFlags.useTruth:
        addRscanJetsHighThresholds("AntiKt",radius,"Truth",jetm9Seq,"JETM9")
    addRscanJetsHighThresholds("AntiKt",radius,"EMPFlow",jetm9Seq,"JETM9")

#====================================================================
# Add the containers to the output stream - slimming done here
#====================================================================
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
JETM9SlimmingHelper = SlimmingHelper("JETM9SlimmingHelper")
JETM9SlimmingHelper.SmartCollections = ["PrimaryVertices",
                                        "AntiKt4EMTopoJets",
                                        "AntiKt2EMPFlowJets","AntiKt3EMPFlowJets","AntiKt4EMPFlowJets",
                                        "AntiKt5EMPFlowJets","AntiKt6EMPFlowJets","AntiKt7EMPFlowJets",
                                        "AntiKt8EMPFlowJets","AntiKt9EMPFlowJets","AntiKt10EMPFlowJets"]

JETM9SlimmingHelper.AllVariables = ["TruthEvents","MuonSegments","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape"]
JETM9SlimmingHelper.ExtraVariables = ["TruthVertices.barcode.z"]

# Trigger content
JETM9SlimmingHelper.IncludeJetTriggerContent = True

SmartListJets = ["AntiKt2EMPFlowJets","AntiKt3EMPFlowJets","AntiKt4EMPFlowJets",
                 "AntiKt5EMPFlowJets","AntiKt6EMPFlowJets","AntiKt7EMPFlowJets",
                 "AntiKt8EMPFlowJets","AntiKt9EMPFlowJets","AntiKt10EMPFlowJets"]

addJetOutputs(JETM9SlimmingHelper,["JETM9"],SmartListJets)
JETM9SlimmingHelper.AppendContentToStream(JETM9Stream)
JETM9Stream.RemoveItem("xAOD::TrigNavigation#*")
JETM9Stream.RemoveItem("xAOD::TrigNavigationAuxInfo#*")
