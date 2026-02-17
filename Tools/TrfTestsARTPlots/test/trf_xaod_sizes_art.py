#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import os, glob, sys
import argparse
import pandas as pd
import matplotlib.pyplot as plt

import plot_style

def readfile(filename):

  # Read line into lines array
  with open(filename) as f:
    lines = f.readlines()

  # Set some variable to start values
  iline = 0
  domaindict1 = {}
  domaindict2 = {}
  domaindict3 = {}

  filetype = ""
  testName = 'test'

  # Loop through all lines
  for line in lines:

    # Determine filetype to parse
    if line.startswith("		ART job name:"):
      testName = os.path.splitext(line.strip().split()[3])[0]
    # Determine filetype to parse
    if line.startswith("============ checkxAOD tmp.AOD"):
      filetype="AOD"
    if line.startswith("============ checkxAOD AOD"):
      filetype="AOD"
    if line.startswith("============ checkxAOD myAOD"):
      filetype="AOD"
    if "1000events_mc21_ttbar.AOD.pool.root" in line:
      filetype="AOD"
    if line.startswith("============ checkxAOD DAOD_PHYS.art.pool.root"):
      filetype="PHYS"
    if line.startswith("============ checkxAOD DAOD_PHYSLITE.art.pool.root"):
      filetype="PHYSLITE"

    # Parse CSV lines with domain sizes
    if line.startswith("CSV"):
      domainlist = lines[iline+1].strip().split(",")
      domainsize = lines[iline+2].strip().split(",")
      domaindict = dict(zip(domainlist, domainsize))

      if filetype == "AOD":
        domaindict1 = domaindict
      if filetype == "PHYS":
        domaindict2 = domaindict 
      if filetype == "PHYSLITE":
        domaindict3 = domaindict 

    iline = iline + 1

  return testName, domaindict1, domaindict2, domaindict3

def plot_pieChart(dateStamp, testName, filetype, labels, values):

  def formatter(x):
    return f"{total*x/100:.1f}"

  # Define a DataFrame
  df = pd.DataFrame({'Domain': labels,
                            'Size': values })
  total = sum(df["Size"])

  #the top 10
  df2 = df[:10].copy()

  #others
  new_row = pd.DataFrame(data = {
    'Domain' : ['others'],
    'Size' : [df['Size'][10:].sum()]
  })

  #combining top 10 with others
  df2 = pd.concat([df2, new_row])

  # Title of the plot
  title = "%s, %s, %s, %.1f kB/event" %(dateStamp, testName, filetype, total)

  # Plotting the pie chart for above dataframe
  plot = df2.groupby(['Domain']).sum().plot(kind='pie', y='Size', autopct=formatter, legend=False, title=title, ylabel='' )
  fig = plot.get_figure()

  fileName = "pie_%s_%s.png" %(testName,filetype)
  fig.savefig(fileName)

  return


def plot_barvChart(dateStamp, labels, values):
  # Define a DataFrame
  df = pd.DataFrame({'Domain': labels,
                            'Size': values })
  df.sort_values('Domain',inplace=True)

  # Title of the plot
  title = "%s, Total size/event [kB]" %(dateStamp)

  # Plotting the pie chart for above dataframe
  plot = df.plot.bar(x='Domain', y='Size', legend=False, title=title, ylabel='Size/event [kB]', xlabel='')
  for container in plot.containers:
    plot.bar_label(container, fmt='%.1f')

  plt.xticks(rotation=45, ha='right', rotation_mode='anchor')
  plt.gcf().subplots_adjust(bottom=0.25)

  fig = plot.get_figure()
  fileName = "barv_sum.png" 
  fig.savefig(fileName)

  return


def plot_barhChart(dateStamp, testName, filetype, labels, values):

  # Define a DataFrame
  df = pd.DataFrame({'Domain': labels,
                            'Size': values })

  df.sort_values('Size',inplace=True)
  total = sum(df["Size"])

  # Title of the plot
  title = "%s, %s, %s, %.1f kB/event" %(dateStamp, testName, filetype, total)

  # Plotting the horizontal bar chart for above dataframe
  plot = df.plot.barh(x='Domain', y='Size', legend=False, title=title, ylabel='', xlabel='size/event [kB]')
  for container in plot.containers:
    plot.bar_label(container, fmt='%.1f')

  plt.gcf().subplots_adjust(left=0.25)

  fig = plot.get_figure()
  fileName = "barh_%s_%s.png" %(testName,filetype)
  fig.savefig(fileName)

  return


def parseFiles(dateStamp, logfile):

  payloadFiles = glob.glob(logfile)
  payloadFiles.sort(key=os.path.getmtime)

  totalDict = {}

  # Loop over the payload.stdout files
  for payload in payloadFiles:
    # Parse the PanDA payload.txt logfile of TrfTestART test 
    testName, dict1, dict2, dict3 = readfile(payload)
    print (testName, dict1, dict2, dict3)

    # Plot the AOD, PHYS and PHYSLITE pie charts
    filetypes = [ 'AOD', 'PHYS', 'PHYSLITE']
    alldicts = [ dict1, dict2, dict3 ]
    for k in range(0,3):
      i = -1
      labels = []
      values = []
      for x,y in alldicts[k].items():
        # Remove Total entry
        i = i+1
        if i == 0:
          continue
        labels.append(x)
        values.append(float(y))
      if alldicts[k]:
        plot_pieChart(dateStamp, testName, filetypes[k], labels, values)
        plot_barhChart(dateStamp, testName, filetypes[k], labels, values)
        totallabel = "%s_%s" %(testName,filetypes[k])
        totalDict[totallabel] = alldicts[k]['Total']

  # plot bar chart with Total format size
  labels = []
  values = []
  for x,y in totalDict.items():
    labels.append(x)
    values.append(float(y))
  plot_barvChart(dateStamp, labels, values)


  return

def main():

  parser = argparse.ArgumentParser(description="Extracts payload.stdout files of TrfTestsARTPlots logfile archives stored on EOS")
  parser.add_argument("--date", type=str, help="date stamp of test, e.g. 2022-11-14T2101", default="2022-11-14T2101", action="store")
  parser.add_argument("--arch", type=str, help="architecture of the test, x86_64-centos7-gcc11-opt", default="x86_64-centos7-gcc11-opt", action="store")
  parser.add_argument("--logfile", type=str, help="Logfile to parse information, e.g. payload.stdout ", default="payload.stdout", action="store")

  args = parser.parse_args()

  if len(sys.argv) < 2:
    parser.print_help()
    sys.exit(1)

  # Setup ATLAS style
  plot_style.set_atlas()

  parseFiles(args.date, args.logfile)

if __name__ == "__main__":
  main()
