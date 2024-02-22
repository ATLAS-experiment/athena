##
## Make a HeavyFlavorHadronFilter and steer it to select B Hadrons
## with any pT within |eta| < 4
##

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'
     
if not hasattr(filtSeq, "xAODHeavyFlavorBHadronFilter"):
    from GeneratorFilters.GeneratorFiltersConf import xAODHeavyFlavorHadronFilter
    filtSeq += xAODHeavyFlavorHadronFilter("xAODHeavyFlavorBHadronFilter") 

## Default cut params
filtSeq.xAODHeavyFlavorBHadronFilter.RequestBottom=True
filtSeq.xAODHeavyFlavorBHadronFilter.RequestCharm=False
filtSeq.xAODHeavyFlavorBHadronFilter.Request_cQuark=False
filtSeq.xAODHeavyFlavorBHadronFilter.Request_bQuark=False
filtSeq.xAODHeavyFlavorBHadronFilter.RequestSpecificPDGID=False
filtSeq.xAODHeavyFlavorBHadronFilter.RequireTruthJet=False
filtSeq.xAODHeavyFlavorBHadronFilter.BottomPtMin=0*GeV
filtSeq.xAODHeavyFlavorBHadronFilter.BottomEtaMax=4.0


