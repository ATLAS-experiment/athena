
include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthGen",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

if not hasattr(filtSeq, "xAODMuDstarFilter"):
   from GeneratorFilters.GeneratorFiltersConf import xAODMuDstarFilter
   filtSeq += xAODMuDstarFilter("xAODMuDstarFilter")

## Default cut params
filtSeq.xAODMuDstarFilter.PtMinMuon =  0.
filtSeq.xAODMuDstarFilter.PtMaxMuon =  1e9
filtSeq.xAODMuDstarFilter.EtaRangeMuon =  10.0
filtSeq.xAODMuDstarFilter.PtMinDstar =  0.
filtSeq.xAODMuDstarFilter.PtMaxDstar =  1e9
filtSeq.xAODMuDstarFilter.EtaRangeDstar =  10.0
filtSeq.xAODMuDstarFilter.RxyMinDstar =  -1e9
filtSeq.xAODMuDstarFilter.PtMinPis =  0.
filtSeq.xAODMuDstarFilter.PtMaxPis =  1e9
filtSeq.xAODMuDstarFilter.EtaRangePis =  10.0
filtSeq.xAODMuDstarFilter.D0Kpi_only = False
filtSeq.xAODMuDstarFilter.PtMinKpi =  0.
filtSeq.xAODMuDstarFilter.PtMaxKpi =  1e9
filtSeq.xAODMuDstarFilter.EtaRangeKpi =  10.0
filtSeq.xAODMuDstarFilter.mKpiMin =  0.
filtSeq.xAODMuDstarFilter.mKpiMax =  1e9
filtSeq.xAODMuDstarFilter.delta_m_Max =  1e9
filtSeq.xAODMuDstarFilter.DstarMu_m_Max =  1e9
