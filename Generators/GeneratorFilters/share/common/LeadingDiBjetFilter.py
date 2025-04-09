## Example configuration for LeadingDiBjetFilter setting up defaults

include("GeneratorFilters/AntiKt4TruthJets_pileup.py")
include("GeneratorFilters/AntiKt6TruthJets_pileup.py")
include("GeneratorFilters/JetFilter_Fragment.py")

if not hasattr( filtSeq, "LeadingDiBjetFilter" ):
    from GeneratorFilters.GeneratorFiltersConf import LeadingDiBjetFilter
    filtSeq += LeadingDiBjetFilter()
    pass

from AthenaCommon.SystemOfUnits import GeV
"""
LeadingDiBjetFilter = filtSeq.LeadingDiBjetFilter
LeadingDiBjetFilter.LeadJetPtMin = 0  
LeadingDiBjetFilter.LeadJetPtMax = 50000 *GeV
LeadingDiBjetFilter.BottomPtMin = 5.0 *GeV
LeadingDiBjetFilter.BottomEtaMax = 3.0
LeadingDiBjetFilter.JetPtMin = 15.0 *GeV
LeadingDiBjetFilter.JetEtaMax = 2.7
LeadingDiBjetFilter.DeltaRFromTruth = 0.3
LeadingDiBjetFilter.TruthContainerName = "AntiKt4TruthJets"
LeadingDiBjetFilter.LightJetSuppressionFactor = 10
LeadingDiBjetFilter.AcceptSomeLightEvents = False
"""
