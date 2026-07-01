# common fragment for xAODDecayVolume filter
# conversion to XAOD, 
# creation of slimmed container containing truth events
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODDecayVolumeFilter
xAODDecayVolumeFilter = xAODDecayVolumeFilter("xAODDecayVolumeFilter")  
filtSeq += xAODDecayVolumeFilter
