# common fragment for xAODDiPhoton filter
# conversion to XAOD, 
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODDiPhotonFilter
xAODDiPhotonFilter = xAODDiPhotonFilter("xAODDiPhotonFilter")  
filtSeq += xAODDiPhotonFilter

# to modiify cuts put into JOs sth. like:
#filtSeq.xAODDiPhotonFilter.PtCut1st = 20000.0
#filtSeq.xAODDiPhotonFilter.PtCut2nd = 15000.
#filtSeq.xAODDiPhotonFilter.PtCutOthers = 15000.
#filtSeq.xAODDiPhotonFilter.EtaCut1st = 2.5
#filtSeq.xAODDiPhotonFilter.EtaCut2nd = 2.5 
#filtSeq.xAODDiPhotonFilter.EtaCutOthers = 2.5
#filtSeq.xAODDiPhotonFilter.DeltaRCutFrom = -1.    
#filtSeq.xAODDiPhotonFilter.DeltaRCutTo = -1.    
#filtSeq.xAODDiPhotonFilter.MassCutFrom = -1.    
#filtSeq.xAODDiPhotonFilter.MassCutTo = -1.    
#filtSeq.xAODDiPhotonFilter.DiPhotonPtMin = -1.    
#filtSeq.xAODDiPhotonFilter.DiPhotonPtMax = -1.    
#filtSeq.xAODDiPhotonFilter.Use1st2ndPhotons = false
