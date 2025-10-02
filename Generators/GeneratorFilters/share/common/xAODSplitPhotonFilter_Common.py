# common fragment for xAODSplitPhoton filter
# conversion to xAOD, 
# creation of slimmed container 
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

if not hasattr(filtSeq, "xAODSplitPhotonFilter"):
  from GeneratorFilters.GeneratorFiltersConf import xAODSplitPhotonFilter
  xAODSplitPhotonFilter = xAODSplitPhotonFilter("xAODSplitPhotonFilter")  
  filtSeq += xAODSplitPhotonFilter

# to modiify cuts put into JOs sth. like:
#filtSeq.xAODSplitPhotonFilter.Ptcut = 1.0
#filtSeq.xAODSplitPhotonFilter.EtaCut = 2.5
#filtSeq.xAODSplitPhotonFilter.NPhotons = 1
#filtSeq.xAODSplitPhotonFilter.AcceptedSplit = [11]


