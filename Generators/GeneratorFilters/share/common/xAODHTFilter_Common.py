## HT filter setup for anti-kT R=0.4 truth jets
#include("GeneratorFilters/AntiKt4TruthWZJets.py")
include ("GeneratorFilters/FindJets.py")
CreateJets(prefiltSeq, 0.4,"WZ")

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODHTFilter
if "xAODHTFilter" not in filtSeq:
    filtSeq += xAODHTFilter()

#filtSeq.xAODHTFilter.MinJetPt = 20.*GeV # Min pT to consider jet in HT
#filtSeq.xAODHTFilter.MaxJetEta = 999. # Max eta to consider jet in HT
#filtSeq.xAODHTFilter.MinHT = 200.*GeV # Min HT to keep event
#filtSeq.xAODHTFilter.MaxHT = 1000.*GeV # Max HT to keep event
#filtSeq.xAODHTFilter.TruthJetContainer = "AntiKt4TruthWZJets" # Which jets to use for HT
#filtSeq.xAODHTFilter.UseNeutrinosFromWZTau = False # Include neutrinos from W/Z/tau in the HT
#filtSeq.xAODHTFilter.UseLeptonsFromWZTau = True # Include e/mu from W/Z/tau in the HT
