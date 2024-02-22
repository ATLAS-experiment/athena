##
## Make a HeavyFlavorHadronFilter and steer it to select C Hadrons
## with any pT within |eta| < 4
##
include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

if not hasattr(filtSeq, "xAODHeavyFlavorCHadronFilter"):
    from GeneratorFilters.GeneratorFiltersConf import xAODHeavyFlavorHadronFilter
    filtSeq += xAODHeavyFlavorHadronFilter("xAODHeavyFlavorCHadronFilter") 

## Default cut params
filtSeq.xAODHeavyFlavorCHadronFilter.RequestBottom=False
filtSeq.xAODHeavyFlavorCHadronFilter.RequestCharm=True
filtSeq.xAODHeavyFlavorCHadronFilter.Request_cQuark=False
filtSeq.xAODHeavyFlavorCHadronFilter.Request_bQuark=False
filtSeq.xAODHeavyFlavorCHadronFilter.RequestSpecificPDGID=False
filtSeq.xAODHeavyFlavorCHadronFilter.RequireTruthJet=False
filtSeq.xAODHeavyFlavorCHadronFilter.CharmPtMin=0*GeV
filtSeq.xAODHeavyFlavorCHadronFilter.CharmEtaMax=4.0


