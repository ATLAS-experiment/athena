# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os
import numpy as np
import pandas as pd
import subprocess

import ROOT as rr

rr.gROOT.SetStyle('ATLAS')
rr.gROOT.SetBatch(1)

rr.TCandle.SetScaledViolin(False)

#From the atlas-plots repo:
#https://github.com/joeycarter/atlas-plots/tree/master
def atlas_label(
    x=None,
    y=None,
    text="",
    loc="",
    size=None,
    font=None,
    align=None,
    color=None,
    alpha=None,
    angle=None   ):
  """Draws the "official" ATLAS label.

  Guidelines from PubCom:

  An ATLAS label must be present on plots which have been approved by ATLAS:

  - ATLAS: means the plot has been submitted for publication or is in the
    associated approved auxiliary material (i.e. an ATLAS paper, or a
    pre-datataking publication like the CSC notes).
  - ATLAS Preliminary: means the plot has been approved by ATLAS but has not
    appeared in a refereed publication. This is used in particular for CONF
    and PUB notes.
  - ATLAS Work In Progress: used only in student talks at national meetings
    where the student is presenting what is largely her own work. This should
    not be used for internal plots inside ATLAS.
  - ATLAS Internal: for plots shown in internal ATLAS talks (including
    approval talks) or included in draft documents circulated to ATLAS.

  Parameters
  ----------
  x, y : float, optional
      The x- and y-coordinates of the ATLAS label in NDC units [0, 1].

      This x-coordinate is left-aligned and the y-coordinate is top-aligned
      unless the `align` argument is given.

  text : str, optional
      Additional text, e.g. "Internal", "Preliminary", etc. ROOT TLatex syntax
      is supported.

  loc : str, optional
      The location of the ATLAS label.

      The strings 'upper left', 'upper right', 'lower left', 'lower right'
      place the label at the corresponding corner of the axes/figure. This
      option is overridden if the `x` and `y` arguments are provided.

      The default location is 'upper left'.

  size : float, optional
      Text size.

  font : int, optional
      ROOT font code.

  align : int, optional
      ROOT text-alignment code.

  color : int, optional
      ROOT text-color code.

  alpha : float, optional
      Text alpha.

      Not recommended for official ATLAS labels!

  angle : float, optional
      Text angle in degrees.

      Not recommended for official ATLAS labels!

  Returns
  -------
  ROOT.TLatex
      The TLatex object for this label.
  """
  if rr.gROOT.GetStyle("ATLAS") is None:
    print("warning: The 'ATLAS' style has not been set")

  label = rr.TLatex()
  label.SetNDC()

  if size is not None:
    label.SetTextSize(size)

  if font is not None:
    label.SetTextFont(font)

  if align is not None:
    label.SetTextAlign(align)

  if color is not None and alpha is not None:
    label.SetTextColorAlpha(color, alpha)
  elif color is not None:
    label.SetTextColor(color)

  if angle is not None:
    label.SetTextAngle(angle)

  if font is not None:
    label.SetTextFont(font)

  # Decide on label position
  if x is not None and y is not None:
    # Set location manually
    xpos = x
    ypos = y

  elif loc:
    # Set location automatically from 'loc' argument
    if loc == "upper left":
        label.SetTextAlign(13)
        xpos = rr.gPad.GetLeftMargin() + 0.04
        ypos = 1 - rr.gPad.GetTopMargin() - 0.04
    elif loc == "upper right":
        label.SetTextAlign(33)
        xpos = 1 - rr.gPad.GetRightMargin() - 0.04
        ypos = 1 - rr.gPad.GetTopMargin() - 0.04
    elif loc == "lower left":
        label.SetTextAlign(11)
        xpos = rr.gPad.GetLeftMargin() + 0.04
        ypos = rr.gPad.GetBottomMargin() + 0.04
    elif loc == "lower right":
        label.SetTextAlign(31)
        xpos = 1 - rr.gPad.GetRightMargin() - 0.04
        ypos = rr.gPad.GetBottomMargin() + 0.04
    else:
      print(
          "warning: unrecognized location '{}'. "
          "Falling back on 'upper left'".format(loc)
      )
      label.SetTextAlign(13)
      xpos = rr.gPad.GetLeftMargin() + 0.04
      ypos = 1 - rr.gPad.GetTopMargin() - 0.04

  else:
    # User 'upper left' if no position arguments are given
    label.SetTextAlign(13)
    xpos = rr.gPad.GetLeftMargin() + 0.04
    ypos = 1 - rr.gPad.GetTopMargin() - 0.04

  if not text:
    label.DrawLatex(xpos, ypos, "#bf{#it{ATLAS}}")
  else:
    label.DrawLatex(xpos, ypos, "#bf{#it{ATLAS}} " + text)
  return label


extra_label_to_use = "Internal"


def combine_num_events(n1, n2):
  if n1 >= 0 and n2 >= 0:
    return min(n1, n2)
  elif n1 >= 0:
    return n1
  elif n2 >= 0:
    return n2
  else:
    return -1

class Sample:
  def __init__(self, d = {}, global_folder = "", global_num_events = -1):
    self.input_folder = os.path.join(global_folder, d.get("input_folder", ""))
    self.output_folder = d.get("output_folder", "")
    self.name = d.get("name", "NO NAME")
    self.latex_name = d.get("latex", "NO NAME")
    self.root_name = d.get("root", "NO NAME")
    self.colour = d.get("colour", 0)
    self.do_perf_info = d.get("perf", False)
    self.do_matching = d.get("match", False)
    self.do_sync = d.get("sync", False)
    self.use_two_gaussian = d.get("two_gaussian", False)
    self.num_events = combine_num_events(d.get("num_events", -1), global_num_events)
    self.threads_start = 0
    self.threads_end = 0
    self.threads_step = 1
    self.data_list = []
    self.perf_list = []
    self.marker_style = None #Set later based on position inside sample.

class Combination:
  def __init__(self, d = {}, global_folder = "", global_num_events = -1):
    self.name = d.get("name", "NO NAME")
    self.do_plots = d.get("do_plots", False)
    self.do_tables = d.get("do_tables", False)
    self.add_to_global = d.get("add_to_global", True)
    self.samples = []
    
    this_num_events = combine_num_events(d.get("num_events", -1), global_num_events)
    
    for s in d.get("samples", []):
      this_s = Sample(s, global_folder, this_num_events)
      if self.do_plots:
        this_s.do_perf_info = True
      if self.do_tables:
        this_s.do_sync = True
      self.samples += [this_s]

class Threads:
  def __init__(self, d = {}, global_folder = "", global_num_events = -1):
    self.threads_start = d.get("start", 1)
    self.threads_end   = d.get("end", 1)
    self.threads_step  = d.get("step", 1)
    self.samples = []
    
    this_num_events = combine_num_events(d.get("num_events", -1), global_num_events)
    
    for s in d.get("samples", []):
      this_s = Sample(s, global_folder, this_num_events)
      this_s.do_perf_info = False
      this_s.do_sync = False
      this_s.do_matching = False
      this_s.threads_start = self.threads_start
      this_s.threads_end = self.threads_end
      this_s.threads_step = self.threads_step
      self.samples += [this_s]

def get_from_file(sample_info):
  print("Getting {} from {}".format(sample_info.name, sample_info.output_folder))
  
  #List of files to read and the columns
  #to ignore for outlier removal.
  if sample_info.do_sync:
    files_and_columns = (
      ("GlobalTimes.txt", ("Event Number", "Total", "GrowPerfInfo", "SplitPerfInfo", "ClusterDeleter", "PlotterMonitoring")), 
      ("EventDataExporterTimes.txt", ("Event Number", "Clusters")), #Empty cluster collection means clusters here are a no-op...
      ("GPUGrowingTimes.txt", ("Event Number", "Preprocessing")),
      ("PropCalcPostGrowingTimes.txt", ("Event Number", "Preprocessing")),
      ("GPUSplitterTimes.txt", ("Event Number", "Preprocessing")),
      ("GPUClusterSortingTimes.txt", ("Event Number", "Preprocessing")),
      ("GPUTopoMomentsTimes.txt", ("Event Number")),
      ("AthenaClusterImporterTimes.txt", ("Event Number", "Sorting")) #Sorting is a no-op here
      )
  else:
    files_and_columns = (
      ("GlobalTimes.txt", ("Event Number", "Total", "GrowPerfInfo", "SplitPerfInfo", "ClusterDeleter", "PlotterMonitoring")),
    )
    
  
  subfolder_list = [str(i) for i in range(sample_info.threads_start, sample_info.threads_end, sample_info.threads_step)]
  
  if len(subfolder_list) == 0:
    subfolder_list = [None]
  else:
    sample_info.data_list = []
    if sample_info.do_perf_info:
      sample_info.perf_list = []
      
  for extra_folder in subfolder_list:
    all_data = [pd.read_table(os.path.join(sample_info.output_folder, f_c[0]) if extra_folder is None else os.path.join(sample_info.output_folder, extra_folder, f_c[0]), sep=r"\s+") for f_c in files_and_columns]
    for d in all_data:
      d.rename(columns = lambda c: c[c.rfind('.')+1:].replace('_', ' '), inplace=True)
    
    inclusions = np.ones(len(all_data[0]), dtype=bool)
      
    for d, f_c in zip(all_data, files_and_columns):
      for c in d:
        if c in f_c[1]:
          continue
        this_center = np.median(d[c])
        one_edge    = np.min(d[c])
        limit       = this_center + (this_center - one_edge) * 20
        if limit <= this_center:
          limit = np.max(d[c])
        inclusions *= d[c] <= limit
    
    print("{}: Events selected: {}%".format(sample_info.latex_name, inclusions.mean() * 100))
    
    for d in all_data:
      d.drop(d[~inclusions].index, inplace = True)
    
    if extra_folder is None:
      sample_info.data_list = all_data
    else:
      sample_info.data_list += [all_data]
    
    if sample_info.do_perf_info:
      all_perf = [pd.read_table(os.path.join(sample_info.output_folder, f) if extra_folder is None else os.path.join(sample_info.output_folder, extra_folder, f), sep=r"\s+") for f in ("post_grow_info.txt", "post_split_info.txt")]
      for d in all_perf:
        d.drop(d[~inclusions].index, inplace = True)
        d.rename(columns = lambda c: c[c.rfind('.')+1:].replace('_', ' '), inplace=True)
      if extra_folder is None:
        sample_info.perf_list = all_perf
      else:
        sample_info.perf_list += [all_perf]
    

def get_CPU_time(df_list):
  return (df_list[0]["DefaultGrowing"]   +
          df_list[0]["DefaultSplitting"] +
          df_list[0]["DefaultMoments"]     )

def get_GPU_time(df_list):
  return (df_list[0]["EventDataExporter"]     +
          df_list[0]["GPUGrowing"]            +
          df_list[0]["PropCalcPostGrowing"]   +
          df_list[0]["GPUSplitter"]           +
          df_list[0]["GPUClusterSorting"]     +
          df_list[0]["GPUTopoMoments"]        +
          df_list[0]["AthenaClusterImporter"]   )

def get_GPU_algorithm_time(df_list):
  return (df_list[0]["GPUGrowing"]            +
          df_list[0]["PropCalcPostGrowing"]   +
          df_list[0]["GPUSplitter"]           +
          df_list[0]["GPUClusterSorting"]     +
          df_list[0]["GPUTopoMoments"]          )

def make_simple_row(long_format, row_name, total_getter, getter, samples):
  ret = " \\hline\n"
  if long_format:
    ret += r"\multicolumn{2}{|c||}{" + row_name + r"}"
  else:
    ret += row_name
  
  for idx, s in enumerate(samples):
    total_t = total_getter(s.data_list)
    this_t  = getter(s.data_list)
    frac_t  = 100 * this_t / total_t
    ret += r" & $ " + str(round(this_t.mean(),0)) + r" \pm " + str(round(this_t.std(),0)) + r" $ & "
    if long_format:
      ret += r"\multicolumn{2}{c" + (r"|}" if idx == len(samples) - 1 else r"||}")
      ret += r"{ $ " + str(round(frac_t.mean(),2)) + r" \pm " + str(round(frac_t.std(),2)) + r" \% $ }"
    else:
      ret += r"$ " + str(round(frac_t.mean(),2)) + r" \pm " + str(round(frac_t.std(),2)) + r" \% $"
  
  ret += r"\bigstrut\\" + "\n" + r"  \hline"
  return ret

def make_complex_row(include_total, row_name, total_getter, getter, substeps, samples):
  ret = " \\hline\n"
  num_multi_rows = len(substeps) + include_total
  for idx, step in enumerate(substeps):
    if idx == 0:
      ret += r"  \multicolumn{1}{|c|}{\multirow{" + str(num_multi_rows) + r"}[" + str(2 * num_multi_rows) + r"]{*}{" + row_name + r"}}"
    else:
      ret += r"\cline{2-3}"
      for j in range(0, len(samples)):
        ret += r"\cline{" + str(3 + j * 3) + r"-" + str(4 + j * 3) + r"}"
      ret += r" \multicolumn{1}{|c|}{}"
        
    ret += r" & " + step[0]
    for s in samples:
      total_t      = total_getter(s.data_list)
      step_total_t = getter(s.data_list)
      substep_t    = step[1](s.data_list)
      frac_step    = 100 * step_total_t / total_t
      frac_substep = 100 * substep_t / step_total_t
      ret += r" & $ " + str(round(substep_t.mean(),0)) + r" \pm " + str(round(substep_t.std(),0)) + r" $ & \multicolumn{1}{c|}"
      ret += r"{ $ " +  str(round(frac_substep.mean(),2)) + r" \pm " + str(round(frac_substep.std(),2)) + r" \% $ } & "
      if idx == 0:
        ret += r"\multirow{" + str(num_multi_rows) + r"}[" + str(2 * num_multi_rows) + r"]{*}"
        ret += r"{ $ " + str(round(frac_step.mean(),2)) + r" \pm " + str(round(frac_step.std(),2)) + r" \% $ }"
    ret += r" \bigstrut\\" + "\n"
  
  if include_total:
    ret += r"\cline{2-3}"
    for j in range(0, len(samples)):
      ret += r"\cline{" + str(3 + j * 3) + r"-" + str(4 + j * 3) + r"}"
    ret += r"  \multicolumn{1}{|c|}{} & Total"
    for s in samples:
      step_total_t = getter(s.data_list)
      ret += r" & $ " + str(round(step_total_t.mean(),0)) + r" \pm " + str(round(step_total_t.std(),0)) + r" $ & \multicolumn{1}{c|}{---} & " 
    ret += r" \bigstrut\\" + "\n"
    
  ret += r"  \hline"
  return ret

def make_tables(output_long, output_short, include_total, samples):
  long_table = r"\begin{tabular}{|cc|"
  for _ in samples:
    long_table += r"|c|cc|"
  long_table += "}\n" + r"  \hline" + "\n" + r"  \multicolumn{2}{|c||}{\multirow{2}[4]{*}{Step}}"
  for s in samples[:-1]:
    long_table += r" & \multicolumn{3}{c||}{" + s.latex_name + r"}"
  long_table += r" & \multicolumn{3}{c|}{" + samples[-1].latex_name + r"} \bigstrut\\"
  long_table += "\n" + r"  \cline{3-" + str(2 + len(samples) * 3) + r"} \multicolumn{2}{|c||}{}"
  for _ in samples[:-1]:
    long_table += r" & Time  (\SI{}{\micro\second}) & \multicolumn{2}{c||}{Fraction of Total Time}"
  long_table += r" & Time  (\SI{}{\micro\second}) & \multicolumn{2}{c|}{Fraction of Total Time} \bigstrut\\" + "\n" + r"  \hline"
  
  long_table += make_simple_row(True, "CPU to GPU Data Conversion", 
                                lambda d_list: get_GPU_time(d_list),
                                lambda d_list: d_list[1]["Cells"] + d_list[1]["Clusters"],
                                samples)
  
  long_table += make_simple_row(True, "Cell Data Transfer", 
                                lambda d_list: get_GPU_time(d_list),
                                lambda d_list: d_list[1]["Transfer to GPU"],
                                samples)
  
  long_table += make_complex_row(include_total, "Growing",
                                lambda d_list: get_GPU_time(d_list),
                                lambda d_list: d_list[2]["Total"],
                                ( ("Cell Classification",
                                   lambda d_list: d_list[2]["Preprocessing"] + d_list[2]["Signal-to-Noise Ratio"]),
                                  ("Neighbour Pair Creation",
                                   lambda d_list: d_list[2]["Cell Pair Creation"]),
                                  ("Tag Propagation and Finalization",
                                   lambda d_list: d_list[2]["Cluster Growing"]),
                                ),
                                samples)
  
  long_table += make_simple_row(True, "Cluster Properties Calculation", 
                                lambda d_list: get_GPU_time(d_list),
                                lambda d_list: d_list[3]["Total"],
                                samples)
  
  long_table += make_complex_row(include_total, "Splitting",
                                lambda d_list: get_GPU_time(d_list),
                                lambda d_list: d_list[4]["Total"],
                                ( ("Neighbour Pair Creation",
                                   lambda d_list: d_list[4]["Preprocessing"] + d_list[4]["Fill List of Intra-Cluster Neighbours"]),
                                  ("Local Maxima Identification",
                                   lambda d_list: d_list[4]["Find Local Maxima"]),
                                  ("Secondary Maxima Exclusion",
                                   lambda d_list: d_list[4]["Find Secondary Maxima"]),
                                  ("Main Tag Propagation",
                                   lambda d_list: d_list[4]["Splitter Tag Propagation"]),
                                  ("Cluster Finalization",
                                   lambda d_list: d_list[4]["Cell Weighting And Finalization"]),
                                ),
                                samples)
  
  long_table += make_complex_row(include_total, "Sorting",
                                lambda d_list: get_GPU_time(d_list),
                                lambda d_list: d_list[5]["Total"],
                                ( ("Transverse Energy Calculation",
                                   lambda d_list: d_list[5]["Preprocessing"] + d_list[5]["Calculating ET"]),
                                  ("Cluster Sorting",
                                   lambda d_list: d_list[5]["Sorting Clusters"]),
                                  ("Prefix Sum and Finalization",
                                   lambda d_list: d_list[5]["Finalizing Clusters"]),
                                ),
                                samples)
  
  long_table += make_complex_row(include_total, "Moments",
                                lambda d_list: get_GPU_time(d_list),
                                lambda d_list: d_list[6]["Total"],
                                ( ("Zeroth Pass",
                                   lambda d_list: d_list[6]["Isolation Clusters"] + d_list[6]["Isolation Cells"] + d_list[6]["Zeroth Clusters"]),
                                  ("First Pass",
                                   lambda d_list: d_list[6]["First Cells"] + d_list[6]["First Clusters"]),
                                  ("Second Pass",
                                   lambda d_list: d_list[6]["Second Cells"] + d_list[6]["Second Clusters"]),
                                  ("Shower Axis Calculation",
                                   lambda d_list: d_list[6]["Shower Axis"]),
                                  ("Third Pass",
                                   lambda d_list: d_list[6]["Third Cells"] + d_list[6]["Third Clusters"]),
                                  ("Finalization",
                                   lambda d_list: d_list[6]["Finalize Clusters"]),
                                ),
                                samples)
  
  long_table += make_complex_row(include_total, "Importing",
                                lambda d_list: get_GPU_time(d_list),
                                lambda d_list: d_list[7]["Total"],
                                ( ("Initialization",
                                   lambda d_list: d_list[7]["Preprocessing"] + d_list[7]["Number and State"]),
                                  ("Cell Assignment",
                                   lambda d_list: d_list[7]["Link Creation"] + d_list[7]["Cell Processing"] + d_list[7]["Sorting"]),
                                  ("Basic Cluster Properties",
                                   lambda d_list: d_list[7]["Basic Info"]),
                                  ("Cluster Moments",
                                   lambda d_list: d_list[7]["Moments"] + d_list[7]["Cluster Size"]),
                                  ("High-Voltage Moments",
                                   lambda d_list: d_list[7]["HV Moments"]),
                                ),
                                samples)
  
  if include_total:
    long_table += r" \hline" + "\n" + r"  \multicolumn{2}{|c||}{Total} "
    for idx, s in enumerate(samples):
      this_t = get_GPU_time(s.data_list)
      long_table += r" & $ " + str(round(this_t.mean(), 0)) + r" \pm " + str(round(this_t.std(), 0)) + r" $ & "
      long_table += r"\multicolumn{2}{c" + (r"|}" if idx == len(samples) - 1 else r"||}") + r"{---}"
    long_table += r" \bigstrut\\" + "\n" + r"  \hline"
  
  long_table += "\n" + r"\end{tabular}"
  
  with open(output_long, "w") as out_long:
    out_long.write(long_table)
  
  short_table = r"\begin{tabular}{|c|"
  for _ in samples:
    short_table += r"|c|c|"
  short_table += "}\n" + r"  \hline" + "\n" + r"  \multirow{2}[4]{*}{Step}"
  for s in samples[:-1]:
    short_table += r" & \multicolumn{2}{c||}{" + s.latex_name + r"}"
  short_table += r" & \multicolumn{2}{c|}{" + samples[-1].latex_name + r"} \bigstrut\\" + "\n"
  short_table += r"  \cline{2-" + str(1 + len(samples) * 2) + r"} "
  for _ in samples:
    short_table += r" & Time  (\SI{}{\micro\second}) & Fraction of Total Time"
  short_table += r" \bigstrut\\" + "\n" + r"  \hline"
  
  short_table += make_simple_row(False, "CPU to GPU Data Conversion", 
                                 lambda d_list: get_GPU_time(d_list),
                                 lambda d_list: d_list[1]["Cells"] + d_list[1]["Clusters"],
                                 samples)
  
  short_table += make_simple_row(False, "Cell Data Transfer", 
                                 lambda d_list: get_GPU_time(d_list),
                                 lambda d_list: d_list[1]["Transfer to GPU"],
                                 samples)
  
  short_table += make_simple_row(False, "Cluster Growing", 
                                 lambda d_list: get_GPU_time(d_list),
                                 lambda d_list: d_list[2]["Total"],
                                 samples)
  
  short_table += make_simple_row(False, "Cluster Properties Calculation", 
                                 lambda d_list: get_GPU_time(d_list),
                                 lambda d_list: d_list[3]["Total"],
                                 samples)
  
  short_table += make_simple_row(False, "Cluster Splitting", 
                                 lambda d_list: get_GPU_time(d_list),
                                 lambda d_list: d_list[4]["Total"],
                                 samples)
  
  short_table += make_simple_row(False, "Cluster Sorting and Finalization", 
                                 lambda d_list: get_GPU_time(d_list),
                                 lambda d_list: d_list[5]["Total"],
                                 samples)
  
  short_table += make_simple_row(False, "Cluster Moments Calculation", 
                                 lambda d_list: get_GPU_time(d_list),
                                 lambda d_list: d_list[6]["Total"],
                                 samples)
  
  short_table += make_simple_row(False, "GPU to CPU Data Conversion", 
                                 lambda d_list: get_GPU_time(d_list),
                                 lambda d_list: d_list[7]["Total"],
                                 samples)
  
  if include_total:
    short_table += r" \hline" + "\n" + r"  Total "
    for idx, s in enumerate(samples):
      this_t = get_GPU_time(s.data_list)
      short_table += r" & $ " + str(round(this_t.mean(), 0)) + r" \pm " + str(round(this_t.std(), 0)) + r" $ & --- "
    short_table += r" \bigstrut\\" + "\n" + r"  \hline" + "\n" + r"\end{tabular}"
  
  with open(output_short, "w") as out_short:
    out_short.write(short_table)

marker_styles = [20, 21, 22, 29, 33, 34, 41, 43, 45, 47]

def plot_one_combined(plotname, x_getter, y_getter, x_label, y_label, samples, log_X = False, log_Y = False):
  cvs = rr.TCanvas(plotname, "", 3400, 2000)
  
  mg = rr.TMultiGraph(plotname + "_mg", ";" + x_label + ";" + y_label)
  
  for idx, s in enumerate(reversed(samples)):
    graph = rr.TGraph()
    graph.SetTitle(s.root_name + ";" + x_label + ";" + y_label)
    
    xs = x_getter(s.perf_list)
    ys = y_getter(s.data_list)
    
    for x, y in zip(xs, ys):
      graph.AddPoint(x, y)
    
    graph.SetMarkerColor(s.colour)
    graph.SetMarkerSize(2.0)
    
    graph.SetMarkerStyle(marker_styles[idx % len(marker_styles)] if s.marker_style is None else s.marker_style)
    
    mg.Add(graph)
  
  mg.Draw("AP")
  
  if log_X:
    cvs.SetLogx()
  
  if log_Y:
    cvs.SetLogy()
  
  cvs.Update()
  
  rr.gPad.BuildLegend(0.75, 0.75, 0.95, 0.95, "")
  
  atlas_label(text = extra_label_to_use, loc = "upper left")
  
  cvs.SaveAs(plotname + ".png")

def handle_combination(samples, do_tables, do_plots, comb_name):
  for s in samples:
    get_from_file(s)
  
  if do_tables:
    make_tables(comb_name + "_long.tex", comb_name + "_short.tex", True, samples)
  
  if do_plots:
    plot_one_combined(comb_name + "_perf_split_cluster_number",
                      lambda p: p[1]["Number Clusters"],
                      lambda d: get_GPU_time(d) * 0.001,
                      "# of Clusters",
                      "GPU Execution Time [ms]",
                      samples)
                      #Easy to extend this...

def plot_one_global_per_sample(plotname, getter, y_label, fill_colour, samples):
  cvs = rr.TCanvas(plotname, "", 3400, 2000)
  
  cvs.cd()
  
  times = [getter(s.data_list) for s in samples]
  
  limits = [999999999, 0]
  
  for t_sample in times:
    limits[0] = min(limits[0], np.min(t_sample))
    limits[1] = max(limits[1], np.max(t_sample))
  
  violin = rr.TH2F(plotname + "_v", ";Sample;" + y_label, len(samples), 0, len(samples), 100, limits[0] * 0.5, limits[1] * 1.25)
  
  for idx, t_sample in enumerate(times):
    for t in t_sample:
      violin.Fill(idx, t)
  
  for idx, s in enumerate(samples):
    violin.GetXaxis().SetBinLabel(idx + 1, s.root_name)
    
  violin.SetFillColor(fill_colour)
  
  config_string = "VIOLIN(03002100)"
  
  violin.Draw(config_string)
  cvs.Update()
  
  atlas_label(text = extra_label_to_use, loc = "upper left")
  
  cvs.SaveAs(plotname + ".png")
  cvs.Clear()

def do_global_plots(samples):
  colour = 27
  
  plot_one_global_per_sample("global_CPU_time",
                             lambda d: get_CPU_time(d) * 0.001, 
                             "CPU Total Execution Times [ms]",
                             colour, samples)
  plot_one_global_per_sample("global_GPU_time",
                             lambda d: get_GPU_time(d) * 0.001, 
                             "GPU Total Execution Times [ms]",
                             colour, samples)
  plot_one_global_per_sample("global_speedup",
                             lambda d: get_CPU_time(d)/get_GPU_time(d), 
                             "Speed-Up",
                             colour, samples)
  plot_one_global_per_sample("global_algorithm_time",
                             lambda d: get_GPU_algorithm_time(d) * 0.001, 
                             "GPU Algorithm Times [ms]",
                             colour, samples)
                             
  plot_one_combined("global_perf_split_cluster_number",
                    lambda p: p[1]["Number Clusters"],
                    lambda d: get_GPU_time(d) * 0.001,
                    "# of Clusters",
                    "GPU Execution Time [ms]",
                    samples)

def plot_all_combinations(combinations_to_do):
  flattened_samples = []
  
  for comb in combinations_to_do:
    handle_combination(comb.samples, comb.do_tables, comb.do_plots, comb.name)
    if comb.add_to_global:
      for idx in range(len(comb.samples)):
        comb.samples[idx].colour = comb.samples[-1].colour
        comb.samples[idx].marker_style = marker_styles[idx % len(marker_styles)]
      flattened_samples += comb.samples
  
  do_global_plots(flattened_samples)

def plot_one_per_threads(plotname, getter, y_label, samples):
  cvs = rr.TCanvas(plotname, "", 3400, 2000)
  
  times = [[getter(d_l, num_threads) for d_l, num_threads in zip(s.data_list, range(s.threads_start, s.threads_end, s.threads_step))] for s in samples]
  
  print(len(times), len(times[0]))
  
  limits = [999999999, 0]
  
  config_string = "VIOLIN(03002100)"
  
  for ts_sample in times:
    for t_thread in ts_sample:
      limits[0] = min(limits[0], np.min(t_thread))
      limits[1] = max(limits[1], np.max(t_thread))
  
  print(limits[0], limits[1], len(times))
  
  stack = rr.THStack(plotname + "_stack", ";# of CPU Threads;" + y_label)
  
  for (ts_sample, s) in zip(times, samples):
    violin = rr.TH2F(plotname + "_" + s.name + "_v", s.root_name + ";# of CPU Threads;" + y_label, len(ts_sample), 0, len(ts_sample), 250, limits[0] * 0.1, limits[1] * 1.25)
    for bin_idx, ts in enumerate(ts_sample):
      for t in ts:
        violin.Fill(bin_idx, t)
    
    violin.SetFillColor(s.colour)
    
    for bin_idx, num_threads in enumerate(range(s.threads_start, s.threads_end, s.threads_step)):
      violin.GetXaxis().SetBinLabel(bin_idx + 1, str(num_threads))
    
    stack.Add(violin)
  
  stack.Draw(config_string)
  cvs.Update()
  
  rr.gPad.BuildLegend(0.75, 0.75, 0.95, 0.95, "")
  
  atlas_label(text = extra_label_to_use, loc = "upper left")
  
  
  cvs.SaveAs(plotname + ".png")
  cvs.Clear()

def plot_for_threads(thread_spec):
  for s in thread_spec.samples:
    get_from_file(s)
  
  plot_one_per_threads("threads_CPU_time",
                       lambda d, n: get_CPU_time(d) * 0.001 / n, 
                       "CPU Total Execution Times [ms]",
                       thread_spec.samples)
  plot_one_per_threads("threads_GPU_time",
                       lambda d, n: get_GPU_time(d) * 0.001 / n, 
                       "GPU Total Execution Times [ms]",
                       thread_spec.samples)
  plot_one_per_threads("threads_speedup",
                       lambda d, n: get_CPU_time(d)/get_GPU_time(d), 
                       "Speed-Up",
                       thread_spec.samples)
  plot_one_per_threads("threads_algorithm_time",
                       lambda d, n: get_GPU_algorithm_time(d) * 0.001 / n, 
                       "GPU Algorithm Times [ms]",
                       thread_spec.samples)
    
def plot_all_threads(threads_to_do):
  for thread_spec in threads_to_do:
    plot_for_threads(thread_spec)

def load_configurations(files, skip_threads = False, global_folder_override = None, max_events = -1):
  import json
  
  combinations_to_do = []
  threads_to_do = []
  
  for file in files:
    with open(file, 'r') as f:
      desc = json.load(f)
  
    global_folder = desc.get("global_folder", "") if global_folder_override is None else global_folder_override
    
    global_num_events = combine_num_events(desc.get("num_events", -1), max_events)
  
    for c in desc.get("combinations", []):
      combinations_to_do += [Combination(c, global_folder, global_num_events)]
      
    if not skip_threads:
      threads_to_do += [Threads(desc.get("threads", {}), global_folder, global_num_events)]
  
  return combinations_to_do, threads_to_do
  
def plot_from_files(files, skip_threads = False):
  combinations, threads = load_configurations(files, skip_threads)
  
  plot_all_combinations(combinations)
  
  if not skip_threads:
    plot_all_threads(threads)

USE_SUBPROCESS = True
  
def run_in_folder(these_args, folder, do_logs = False, timeout_seconds = -1, repeat_tries = -1):
  os.makedirs(folder, exist_ok = True)

  if USE_SUBPROCESS:
    
    retry_count = 0
    
    if do_logs:  
      with open(os.path.join(folder, "log.txt"), "ab") as out_file, open(os.path.join(folder, "err.txt"), "ab") as err_file:
        while retry_count <= repeat_tries or repeat_tries < 0:
          try:
            ret = subprocess.call(these_args,
                                  cwd = folder,
                                  stdout = out_file,
                                  stderr = err_file,
                                  timeout = timeout_seconds if timeout_seconds > 0 else None)
            if ret < 0:
              print("Exited with signal {}. Retrying.".format(-ret))
              retry_count += 1
              continue
          except Exception:
            print("Timed out and/or unspecified error. Retrying.")
            retry_count += 1
            continue
          break
    else:
        while retry_count <= repeat_tries or repeat_tries < 0:
          try:
            ret = subprocess.call(these_args,
                                  cwd = folder,
                                  stdout = subprocess.DEVNULL,
                                  stderr = subprocess.DEVNULL,
                                  timeout = timeout_seconds if timeout_seconds > 0 else None)
            if ret < 0:
              print("Exited with signal {}. Retrying.".format(-ret))
              retry_count += 1
              continue
          except Exception:
            print("Timed out and/or unspecified error. Retrying.")
            retry_count += 1
            continue
          break
  else:
    to_run = "cd " + str(folder) + "; " + these_args + (" | tee log.txt" if do_logs else "")
    os.system(to_run + " | grep AthenaHiveEventLoopMgr")
    

def run_sample(sample, scripts_folder = "", do_logs = False, timeout_seconds = -1, repeat_tries = -1):
  import shlex, glob

  thread_range = range(sample.threads_start, sample.threads_end, sample.threads_step)

  command = "python "
  
  if sample.do_matching:
    command += os.path.join("" if len(thread_range) == 0 else "..", "..", scripts_folder, "CaloRecGPU_growsplitmoments_test.py")
  else:
    command += os.path.join("" if len(thread_range) == 0 else "..", "..", scripts_folder, "CaloRecGPU_measure_times.py")
  
  command += " -t"
  
  if not sample.use_two_gaussian:
    command += " -ndgn"
  
  if sample.do_sync:
    command += " -s"
  
  if sample.do_perf_info:
    command += " -pi"
    
  if sample.num_events is not None:
    command += " -events " + str(sample.num_events)
  
  command += " -f"

  if USE_SUBPROCESS:
    real_args = shlex.split(command)
  
    real_args += glob.glob(os.path.join(sample.input_folder, "*"))
  else:
    real_args = command + " " + str(os.path.join(sample.input_folder, "*"))
  
  if len(thread_range) == 0:
    print("Doing {} from {}".format(sample.name, sample.input_folder))
    run_in_folder(real_args, sample.output_folder, do_logs, timeout_seconds, repeat_tries)
  else:
    for i in thread_range:
      print("Doing {}: {}".format(sample.name, i))
      if USE_SUBPROCESS:
        real_real_args = real_args + ["-threads", str(i)]
      else:
        real_real_args = real_args + " -threads " + str(i)
      run_in_folder(real_real_args, os.path.join(sample.output_folder, str(i)), do_logs, timeout_seconds, repeat_tries)


def execute_from_files(files,
                       scripts_folder = "",
                       max_events = -1,
                       skip_threads = False,
                       do_logs = False,
                       timeout_seconds = -1,
                       repeat_tries = -1):
  combinations, threads = load_configurations(files, max_events = max_events)
  
  for c in combinations:
    for s in c.samples:
      run_sample(s, scripts_folder, do_logs, timeout_seconds, repeat_tries)

  if not skip_threads:
    for t in threads:
      for s in t.samples:
        run_sample(s, scripts_folder, do_logs, timeout_seconds, repeat_tries)

if __name__=="__main__":
  import argparse
  parser = argparse.ArgumentParser()
  parser.add_argument('-scripts', '--scripts_folder', type = str, default="")
  parser.add_argument('-run', '--run_files', action = 'extend', nargs = '*')
  parser.add_argument('-plot', '--plot_files', action = 'extend', nargs = '*')
  parser.add_argument('-events', '--max_events', type = int, default = -1)
  parser.add_argument('-log', '--do_logs', action = 'store_true')
  parser.add_argument('-timeout', '--timeout_seconds', type = int, default = 3600)
  parser.add_argument('-retry', '--repeat_tries', type = int, default = 3)
  parser.add_argument('-no_threads', '--skip_threads', action = 'store_true')
  
  args = parser.parse_args()
  
  if args.run_files is not None and len(args.run_files) > 0:
    execute_from_files(args.run_files,
                       args.scripts_folder,
                       args.max_events,
                       args.skip_threads,
                       args.do_logs,
                       args.timeout_seconds,
                       args.repeat_tries)
  
  if args.plot_files is not None and len(args.plot_files) > 0:
    plot_from_files(args.plot_files,
                    args.skip_threads)
  
  
