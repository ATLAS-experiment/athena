# common fragment for xAODMultiLepton filter
# conversion to XAOD,
# creation of slimmed container containing electrons, muons and taus
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthElectrons",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

createxAODSlimmedContainer("TruthMuons",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

createxAODSlimmedContainer("TruthTaus",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODMultiLeptonFilter
xAODMultiLeptonFilter = xAODMultiLeptonFilter("xAODMultiLeptonFilter")
filtSeq += xAODMultiLeptonFilter

# to modiify cuts put into JOs e.g.:
#filtSeq.xAODMultiLeptonFilter.countElectrons = True
#filtSeq.xAODMultiLeptonFilter.countMuons     = True
#filtSeq.xAODMultiLeptonFilter.countTaus      = True
#filtSeq.xAODMultiLeptonFilter.PtCut  = 10000.0
#filtSeq.xAODMultiLeptonFilter.EtaCut = 2.5
#filtSeq.xAODMultiLeptonFilter.TauPtCut  = 15000.0  # per-flavour override, negative inherits PtCut
#filtSeq.xAODMultiLeptonFilter.NLeptons = 4

