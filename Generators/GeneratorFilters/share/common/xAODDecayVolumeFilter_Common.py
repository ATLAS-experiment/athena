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

# to modify cuts put into JOs e.g.:
#filtSeq.xAODDecayVolumeFilter.LLP_PDG = 36
#filtSeq.xAODDecayVolumeFilter.RCutMin = 0.
#filtSeq.xAODDecayVolumeFilter.RCutMax = 400.
#filtSeq.xAODDecayVolumeFilter.zCutMin = 0.
#filtSeq.xAODDecayVolumeFilter.zCutMax = 400.
#filtSeq.xAODDecayVolumeFilter.MaxAbsEta = 2.5
#filtSeq.xAODDecayVolumeFilter.MinPass = 2
