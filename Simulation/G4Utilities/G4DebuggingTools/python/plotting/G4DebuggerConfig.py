import ROOT
import os

def merge_two_dicts(x, y):
    z = x.copy()   # start with x's keys and values
    z.update(y)    # modifies z with y's keys and values & returns None
    return z

AtlasStylePath = os.environ['HOME']+'/RootUtils/AtlasStyle'
AtlasStyle = os.path.exists(AtlasStylePath)
if AtlasStyle:
  ROOT.gROOT.LoadMacro(AtlasStylePath+"/AtlasStyle.C")
  ROOT.gROOT.LoadMacro("common/AtlasLabels.C")
  ROOT.gROOT.LoadMacro("common/AtlasUtils.C")
  ROOT.SetAtlasStyle()

particles = ["other","gamma","e-","neutron",]
otherParticles = ["nucleus","proton","e+"]
# particles = ["other","nucleus","proton","neutron","gamma","e-","e+"]

egamma = ["gamma","e-","e+"]
electrons = ["e-","e+"]
neutrons = ["neutron"]
hadrons = ["proton","neutron","nucleus"]

colors = {
 'e-':ROOT.kRed-6,
 'e+':ROOT.kBlack,
 'neutron':ROOT.kBlue-9,
 'gamma':ROOT.kGray,
 'other':ROOT.kGray+1,
 'proton':ROOT.kBlack,
 'nucleus':ROOT.kCyan-4,
 'proton':ROOT.kPink+2,
}

colorsNew = {
 'e-':ROOT.kRed-2,
 'e+':ROOT.kBlack,
 'neutron':ROOT.kBlue-2,
 'gamma':ROOT.kGray+2,
 'other':ROOT.kGray+3,
 'proton':ROOT.kBlack,
 'nucleus':ROOT.kCyan,
 'proton':ROOT.kPink+6,
}

styles = {
'gamma': 3354,
}

samecolor = [
'other',
'gamma',
'e+',
]

linecolor = {
'neutron':ROOT.kBlue,
'e-':ROOT.kRed+2,
}

translate = {
'e-':'electrons',
'e+':'positrons',
'gamma':'photons',
'neutron':'neutrons',
# 'Section':'Beampipe',
'Section':'ID services',
'EMEC':'EM end-cap',
'EMB':'EM barrel',
'FC1':'FCal1',
'FC23':'FCal2/3',
'FCOther':'other',
'HEC':'Had. end-cap',
'LAr':'LAr services',
'Cryo':'LAr services',
'Service':'ID services',
'other':'Other',
}

label = {
'stepLength':'log( step length [mm] )',
'stepEnergyDeposit':'log( energy deposit [MeV] )',
'stepSecondaryKinetic':'log( kinetic energy of the secondary [MeV] )',
'stepEnergyNonIonDeposit':'log( non-ionizing energy deposit [MeV] )',
'stepKineticEnergy':'log( kinetic energy in current step [MeV] )',
'stepPseudorapidity':'#eta in current step',
}

labelAllATLAS = {
'numberOfSteps':'number of steps',
'numberOfStepsPerInitialE':'log( Initial kinetic [MeV] )',
'CumulativeNumberOfStepsPerInitialE':'log( Initial kinetic [MeV] )',
'InitialE':'log( Initial kinetic [MeV] )',
'CumulativeInitialE':'log( Initial kinetic [MeV] )',
'averageNumberOfStepsPerInitialE':'log( Initial kinetic [MeV] )',
}

labelAll = merge_two_dicts(label,labelAllATLAS)

class summaryDefs:
  rsplit = 0.4
  pad1TopMarin = 0.08
  pad1BotMarin = 0.04
  pad2TopMarin = 0.03
  pad2BotMarin = 0.6
  LeftMargin = 0.15
  # yaxis = "Relative steps [%]"
  yaxis = "Steps / Event"

  def getCanvas(self, uniqueName):
    canv = ROOT.TCanvas(uniqueName,uniqueName,700,600)
    pad1 = ROOT.TPad("pad1"+uniqueName,"top pad"+uniqueName,0.,self.rsplit,1.,1.)
    pad1.SetTopMargin(self.pad1TopMarin)
    pad1.SetBottomMargin(self.pad1BotMarin)
    pad1.SetLeftMargin(self.LeftMargin)
    pad1.Draw()
    pad2 = ROOT.TPad("pad2"+uniqueName,"bottom pad"+uniqueName,0,0,1,self.rsplit)
    pad2.SetTopMargin(self.pad2TopMarin)
    pad2.SetBottomMargin(self.pad2BotMarin)
    pad2.SetLeftMargin(self.LeftMargin)
    pad2.SetTicky()
    pad2.SetTickx()
    pad2.Draw()
    return canv, pad1, pad2

  def configureUpperPad(self,hs1):
    hs1.GetXaxis().SetLabelSize(0)
    hs1.GetYaxis().SetLabelSize(0.075)
    hs1.GetYaxis().SetTitleSize(0.082)
    hs1.GetYaxis().SetTitleOffset(0.7)
    hs1.GetYaxis().SetTitle(self.yaxis)
    ROOT.gPad.RedrawAxis()

  def configureLowerPad(self,h2,ratiodn,ratioup,xaxis,yaxisr):
    h2.SetFillColor(0)
    h2.SetFillStyle(0)
    h2.GetXaxis().SetTitle(xaxis)
    h2.GetYaxis().SetTitle(yaxisr)
    h2.GetYaxis().SetRangeUser(1.0 - ratiodn, 1.0 + ratioup)
    h2.SetLineColor(ROOT.kBlack)
    h2.GetXaxis().LabelsOption("v")
    h2.GetXaxis().SetLabelSize(0.15)
    h2.GetXaxis().SetLabelOffset(0.02)
    h2.GetXaxis().SetTitleOffset(2.2)
    h2.GetXaxis().SetTitleSize(0.14)
    h2.GetYaxis().SetLabelSize(0.11)
    h2.GetYaxis().SetNdivisions(3)
    h2.GetYaxis().SetTitleOffset(0.5)
    h2.GetYaxis().SetTitleSize(0.12)
    ROOT.gPad.RedrawAxis()

class histogramDefs:
  rsplit = 0.35
  pad1TopMarin = 0.07
  pad1BotMarin = 0.04
  pad2TopMarin = 0.03
  pad2BotMarin = 0.4
  LeftMargin = 0.15
  yaxis = "Steps"

  def getCanvas(self, uniqueName, nologx = False, nology = False):
    canv = ROOT.TCanvas(uniqueName,uniqueName,800,600)
    pad1 = ROOT.TPad("pad1"+uniqueName,"top pad"+uniqueName,0.,self.rsplit,1.,1.)
    pad1.SetTopMargin(self.pad1TopMarin)
    pad1.SetBottomMargin(self.pad1BotMarin)
    pad1.SetLeftMargin(self.LeftMargin)
    if not nologx:
      pad1.SetLogx()
    if not nology:
      pad1.SetLogy()
    pad1.Draw()
    pad2 = ROOT.TPad("pad2"+uniqueName,"bottom pad"+uniqueName,0,0,1,self.rsplit)
    pad2.SetTopMargin(self.pad2TopMarin)
    pad2.SetBottomMargin(self.pad2BotMarin)
    pad2.SetLeftMargin(self.LeftMargin)
    pad2.SetTicky()
    pad2.SetTickx()
    if not nologx:
      pad2.SetLogx()
    pad2.SetGridy()
    pad2.Draw()
    return canv, pad1, pad2

  def configureUpperPad(self,hs1, yaxis=""):
    hs1.GetXaxis().SetLabelSize(0)
    hs1.GetYaxis().SetLabelSize(0.06)
    hs1.GetYaxis().SetTitleSize(0.06)
    hs1.GetYaxis().SetTitleOffset(0.95)
    hs1.GetYaxis().SetTitle(self.yaxis if yaxis=="" else yaxis)
    ROOT.gPad.RedrawAxis()

  def configureLowerPad(self,h2,ratio,xaxis,yaxisr):
    h2.GetXaxis().SetTitle(xaxis)
    h2.GetYaxis().SetTitle(yaxisr)
    h2.GetXaxis().SetTitleSize(0.13)
    h2.GetXaxis().SetTitleOffset(0.9)
    h2.GetXaxis().SetLabelSize(0.10)
    h2.GetYaxis().SetTitleSize(0.12)
    h2.GetYaxis().SetTitleOffset(0.5)
    h2.GetYaxis().SetLabelSize(0.10)
    h2.GetYaxis().SetLabelOffset(0.015)
    h2.GetYaxis().SetRangeUser(0.5,1.5)
    h2.GetYaxis().SetNdivisions(6)
    h2.SetMinimum(1.0 - ratio)
    h2.SetMaximum(1.0 + ratio)
    ROOT.gPad.RedrawAxis()




summary = summaryDefs()
histograms = histogramDefs()

