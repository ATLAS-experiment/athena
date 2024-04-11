# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Done as direct AddItems
CompulsoryContent = [
# Next two lines removed because this is now done in smart slimming. Leaving for documentary purposes.
#'xAOD::EventInfo#*',
#'xAOD::EventAuxInfo#*',
'xAOD::TrigDecision#*',
'xAOD::TrigDecisionAuxInfo#*',
'xAOD::TrigConfKeys#*',
'xAOD::BunchConfKey#*']

CompulsoryTriggerNavigation = [
'xAOD::TrigNavigation#*',
'xAOD::TrigNavigationAuxInfo#*',
# Run 3 navigation:
'xAOD::TrigCompositeContainer#HLTNav_Summary',
'xAOD::TrigCompositeAuxContainer#HLTNav_SummaryAux.',
'xAOD::TrigCompositeContainer#HLTNav_OnlineSlimmed',
'xAOD::TrigCompositeAuxContainer#HLTNav_OnlineSlimmedAux.',
'xAOD::TrigCompositeContainer#HLTNav_ESDSlimmed',
'xAOD::TrigCompositeAuxContainer#HLTNav_ESDSlimmedAux.',
'xAOD::TrigCompositeContainer#HLTNav_AODSlimmed',
'xAOD::TrigCompositeAuxContainer#HLTNav_AODSlimmedAux.',
'xAOD::TrigCompositeContainer#HLTNav_DAODSlimmed',
'xAOD::TrigCompositeAuxContainer#HLTNav_DAODSlimmedAux.',
'xAOD::TrigCompositeContainer#HLTNav_R2ToR3Summary',
'xAOD::TrigCompositeAuxContainer#HLTNav_R2ToR3SummaryAux.',
]

# This accounts for AOD content that must be kept but which is partially expressed
# as dynamic variables and which is therefore not covered by the above.
# Currently not needed as EventInfo is moved in to smart slimming, but leaving in place should it be needed for some other container.
CompulsoryDynamicContent = [
#'EventInfo'
]
