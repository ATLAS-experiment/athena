import ROOT
import os, sys

from G4DebuggerConfig import *

class G4Debugger:

  def __init__(self, path, G4Version):


    self.volumeSummary = {}
    self.materialSummary = {}
    self.processSummary = {}

    self.G4Version = G4Version
    self.file = ROOT.TFile(os.path.join(path,"%s.root" % G4Version),"READ")

    print(self.file)

    d = self.file.GetDirectory("volumes")
    for k in d.GetListOfKeys():
      name = k.GetName()
      if name in translate.keys() and name != 'other':
          name = translate[name]
          print(name)
      namePerm = k.GetName()
      if name == "summary":
        continue
      for p in particles:
        h = self.GetHistogram("stepLength",namePerm,p,"volumes")
        if h:
          if not name in self.volumeSummary.keys():
            self.volumeSummary[name] = {}
          if not p in self.volumeSummary[name].keys():
            self.volumeSummary[name][p] = 0
          self.volumeSummary[name][p] += h.Integral(0,h.GetNbinsX()+1)
        if p == "other":
          for op in otherParticles:
            oh = self.GetHistogram("stepLength",namePerm,op,"volumes")
            if oh:
              self.volumeSummary[name]["other"] += oh.Integral(0,oh.GetNbinsX()+1)

    d = self.file.GetDirectory("materials")
    for k in d.GetListOfKeys():
      name = k.GetName()
      if name == "summary":
        continue
      for p in particles:
        h = self.GetHistogram("stepLength",name,p,"materials")
        if h:
          if not name in self.materialSummary.keys():
            self.materialSummary[name] = {}
          if not p in self.materialSummary[name].keys():
            self.materialSummary[name][p] = 0
          self.materialSummary[name][p] += h.Integral(0,h.GetNbinsX()+1)

    d = self.file.GetDirectory("processes")
    for k in d.GetListOfKeys():
      name = k.GetName()
      if name == "summary":
        continue
      for p in particles:
        h = self.GetHistogram("stepLength",name,p,"processes")
        if h:
          if not name in self.processSummary.keys():
            self.processSummary[name] = {}
          if not p in self.processSummary[name].keys():
            self.processSummary[name][p] = 0
          self.processSummary[name][p] += h.Integral(0,h.GetNbinsX()+1)

  def GetHistogram(self,his,cat,p,folder):
    middle = folder[0:3] if not folder == "processes" else "prc"
    hisName = "%s_%s_%s_%s" % (cat, p, middle, (his if his != "2DMaps" else "RZ"))
    h = self.file.Get(os.path.join(folder,cat,his,hisName))
    return h

  def GetHistogramAllATLAS(self,his,cat,p,folder):
    hisName = "%s_%s_%s" % (cat, p, his)
    h = self.file.Get(os.path.join(folder,cat,his,hisName))
    return h
