if not hasattr(filtSeq, "MuDstarFilter"):
    from GeneratorFilters.GeneratorFiltersConf import MuDstarFilter
    filtSeq += MuDstarFilter("MuDstarFilter")

## Default cut params
filtSeq.MuDstarFilter.PtMinMuon =  0.
filtSeq.MuDstarFilter.PtMaxMuon =  1e9
filtSeq.MuDstarFilter.EtaRangeMuon =  10.0
filtSeq.MuDstarFilter.PtMinDstar =  0.
filtSeq.MuDstarFilter.PtMaxDstar =  1e9
filtSeq.MuDstarFilter.EtaRangeDstar =  10.0
filtSeq.MuDstarFilter.RxyMinDstar =  -1e9
filtSeq.MuDstarFilter.PtMinPis =  0.
filtSeq.MuDstarFilter.PtMaxPis =  1e9
filtSeq.MuDstarFilter.EtaRangePis =  10.0
filtSeq.MuDstarFilter.D0Kpi_only = False
filtSeq.MuDstarFilter.PtMinKpi =  0.
filtSeq.MuDstarFilter.PtMaxKpi =  1e9
filtSeq.MuDstarFilter.EtaRangeKpi =  10.0
filtSeq.MuDstarFilter.mKpiMin =  0.
filtSeq.MuDstarFilter.mKpiMax =  1e9
filtSeq.MuDstarFilter.delta_m_Max =  1e9
filtSeq.MuDstarFilter.DstarMu_m_Max =  1e9
