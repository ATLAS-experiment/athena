# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import ROOT

ROOT.gROOT.SetBatch(True)
 
def check():
  file = ROOT.TFile("expert-monitoring.root")
  unmatched_grow = file.Get("HybridClusterProcessor/PlotterMonitoring/growing_num_unmatched_clusters")
  unmatched_split = file.Get("HybridClusterProcessor/PlotterMonitoring/splitting_num_unmatched_clusters") 
  diff_grow = file.Get("HybridClusterProcessor/PlotterMonitoring/growing_cluster_diff_cells_zoom_3")
  diff_split = file.Get("HybridClusterProcessor/PlotterMonitoring/splitting_cluster_diff_cells_zoom_3")
  if unmatched_grow.GetMean() >= 0.1 or unmatched_split.GetMean() >= 0.1 or diff_grow.GetMean() >= 0.1 or diff_split.GetMean() >= 0.1:
    return 1
  else:
    return 0

