# common fragment for xAODDecaysFinalState filter
# conversion to xAOD, 
# creation of slimmed container containing truth events
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODDecaysFinalStateFilter
xAODDecaysFinalStateFilter = xAODDecaysFinalStateFilter("xAODDecaysFinalStateFilter")  
filtSeq += xAODDecaysFinalStateFilter

# to modify cuts put into JOs e.g.:
#filtSeq.xAODDecaysFinalStateFilter.PDGAllowedParents = [ 23, 24, -24 ]
#filtSeq.xAODDecaysFinalStateFilter.NQuarks = 2
#filtSeq.xAODDecaysFinalStateFilter.NElectrons = -1
#filtSeq.xAODDecaysFinalStateFilter.NMuons = -1
#filtSeq.xAODDecaysFinalStateFilter.NTaus = -1
#filtSeq.xAODDecaysFinalStateFilter.NChargedLeptons = 2
#filtSeq.xAODDecaysFinalStateFilter.NNeutrinos = -1
#filtSeq.xAODDecaysFinalStateFilter.NPhotons = -1

#filtSeq.xAODDecaysFinalStateFilter.MinNQuarks = 0
#filtSeq.xAODDecaysFinalStateFilter.MinNElectrons = 0
#filtSeq.xAODDecaysFinalStateFilter.MinNMuons = 0
#filtSeq.xAODDecaysFinalStateFilter.MinNTaus = 0
#filtSeq.xAODDecaysFinalStateFilter.MinNChargedLeptons = 0
#filtSeq.xAODDecaysFinalStateFilter.MinNNeutrinos = 0
#filtSeq.xAODDecaysFinalStateFilter.MinNPhotons = 0




