# conversion to XAOD, 
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'


from GeneratorFilters.GeneratorFiltersConf import xAODJetFilter
xAODJetFilter = xAODJetFilter("xAODJetFilter")
filtSeq += xAODJetFilter
# example setup of parameters (to use in JOs)
#filtSeq.xAODJetFilter.JetNumber = 1
#filtSeq.xAODJetFilter.EtaRange = 2.7
#filtSeq.xAODJetFilter.JetType = False # True = cone, False = grid
#filtSeq.xAODJetFilter.GridSizeEta = 2 # Number of (approx 0.06 size) eta cells
#filtSeq.xAODJetFilter.GridSizePhi = 2 # Number of (approx 0.06 size) phi cells
#filtSeq.xAODJetFilter.JetThreshold = 35000.
