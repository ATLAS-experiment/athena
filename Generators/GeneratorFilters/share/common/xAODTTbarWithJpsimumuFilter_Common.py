# common fragment for xAODTTbarWithJpsimumu filter
# conversion to XAOD, 
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODTTbarWithJpsimumuFilter
xAODTTbarWithJpsimumuFilter = xAODTTbarWithJpsimumuFilter("xAODTTbarWithJpsimumuFilter")
filtSeq += xAODTTbarWithJpsimumuFilter

# to modiify cuts put into JOs e.g.:
#filtSeq.xAODTTbarWithJpsimumuFilter.SelectJpsi = True 
#filtSeq.xAODTTbarWithJpsimumuFilter.JpsipTMinCut = 5000. 
#filtSeq.xAODTTbarWithJpsimumuFilter.JpsietaMaxCut = 5. 



