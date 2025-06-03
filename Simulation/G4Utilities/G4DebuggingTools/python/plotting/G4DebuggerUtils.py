import ROOT
import operator
import sys

from G4DebuggerConfig import *

if AtlasStyle:
  print("setting ATLAS Style")
  ROOT.SetAtlasStyle()

def rename(label):
  if label == "FCO":
    return "FC Other"
  elif label == "HCB":
    return "Tile"
  elif label == "Sev":
    return "Services"
  else:
    return label

def fillHisto(h,catDict,i,p,cat):
  if cat in catDict.keys() and p in catDict[cat].keys():
    h.SetBinContent(i,catDict[cat][p])
  else:
    h.SetBinContent(i,0)
  if not cat in translate.keys():
    h.GetXaxis().SetBinLabel(i, cat)
  else:
    h.GetXaxis().SetBinLabel(i, translate[cat])

def setFillLineColor(h,p, hcolors):
  h.SetFillColor(hcolors[p])
  if p not in samecolor:
      if p in linecolor:
        h.SetLineColor(linecolor[p])
      else:
        h.SetLineColor(hcolors[p]+2)
  else:
      h.SetLineColor(hcolors[p])
  if p in styles.keys():
      h.SetFillStyle(styles[p])

def divide(hs1,hs2):
  h1 = hs1.GetStack().Last().Clone()
  h2 = hs2.GetStack().Last().Clone()
  name = h1.GetName()+"ratio"
  nHalf = int(h1.GetNbinsX()/2)
  h = ROOT.TH1F(name,name,nHalf,0,nHalf)
  for i in range(nHalf):
    denum = h1.GetBinContent(1+2*i) if h1.GetBinContent(1+2*i)!=0 else -1
    h.SetBinContent(i+1,h2.GetBinContent(1+2*i+1)/denum)
    binLabel = rename(h1.GetXaxis().GetBinLabel(1+2*i))
    h.GetXaxis().SetBinLabel(1+i, binLabel)
  return h


def plotSummaryRatio(dict1, dict2, xaxis, v1, v2, directory="", name =""):
  NCats = len(dict1)
  integral1 = sum([sum(dict1[cat].values()) for cat in dict1])
  integral2 = sum([sum(dict2[cat].values()) for cat in dict2])
  print(NCats," ",integral1," ",integral2)
  sorted1 = sorted(dict1.items(), key=lambda kv: sum(kv[1].values()), reverse=True)
  uniqueName = name + sorted1[0][0]
  hmap1 = {}
  hmap2 = {}

  nevents = 1000.

  leg = ROOT.TLegend(0.7,0.89-0.07*len(particles),0.92,0.89)
  leg.SetNColumns(1)
  leg.SetBorderSize(0)
  leg.SetFillColor(0)
  leg.SetFillStyle(0)
  leg.SetTextSize(0.06)

  for p in particles:
    h1 = ROOT.TH1F(uniqueName+"1"+p,uniqueName+"1"+p,2*len(sorted1),0,2*len(sorted1))
    h2 = ROOT.TH1F(uniqueName+"2"+p,uniqueName+"2"+p,2*len(sorted1),0,2*len(sorted1))
    i_adjusted = 0
    for i,a in enumerate(sorted1):
      cat = a[0]
      if cat == "other":
        continue
      fillHisto(h1,dict1,2*i_adjusted+1,p,cat)
      fillHisto(h2,dict2,2*i_adjusted+1+1,p,cat)
      i_adjusted += 1
    if not xaxis == "Processes":
      fillHisto(h1,dict1,2*len(sorted1)-1,p,"other")
      fillHisto(h2,dict2,2*len(sorted1),p,"other")      
    hmap1[p] = h1
    hmap2[p] = h2

  hs1 = ROOT.THStack()
  hs2 = ROOT.THStack()
  for p in particles:
    hmap1[p].Scale(1./nevents)
    hmap2[p].Scale(1./nevents)
    hs1.Add(hmap1[p])
    hs2.Add(hmap2[p])
    setFillLineColor(hmap1[p],p, colors)
    setFillLineColor(hmap2[p],p, colorsNew)
  for p in reversed(particles):
    if p in translate.keys() and p != 'other':
      leg.AddEntry(hmap1[p],"#font[42]{%s}"%translate[p],"f")
    else:
      leg.AddEntry(hmap1[p],"#font[42]{%s}"%p,"f")

  canv, pad1, pad2 = summary.getCanvas(uniqueName)

  h2 = divide(hs1,hs2)

  pad1.cd()

  hs1.SetMaximum(hs1.GetStack().Last().GetBinContent(1)*1.2)
  hs1.Draw("hist")
  hs2.Draw("hist same")
  if xaxis == "Processes":
    hs1.GetXaxis().SetRangeUser(0,40)
    hs2.GetXaxis().SetRangeUser(0,40)

  hs1.GetYaxis().SetTitleOffset(0.7)
  leg.Draw()
  if AtlasStyle:
    ROOT.ATLASLabel(0.3,0.84,"Simulation Preliminary",1,0.06)
    ROOT.myText(0.3,0.77,1,"#sqrt{s}=13 TeV, 10k t#bar{t} events",0.06)
    ROOT.myText(0.3,0.70,1,"NRR (E = 1 MeV, w = 10)",0.06)
    ROOT.myText(0.3,0.63,1,"#oplus EM Range Cuts",0.06)

  summary.configureUpperPad(hs1)

  pad1.Update()
  ymax = ROOT.gPad.GetUymax()*0.55
  ymin = ROOT.gPad.GetUymin()
  l1 = ROOT.TLine(6,ymin,6,ymax)
  l1.SetLineStyle(2)
  l1.Draw()
  l2 = ROOT.TLine(7,ymin,7,ymax)
  l2.SetLineStyle(2)
  l2.Draw()
  l3 = ROOT.TLine(8,ymin,8,ymax)
  l3.SetLineStyle(2)
  l3.Draw()
  ROOT.myTextAbs(6.1, 0.8*ymax, 1, "Non optimized", 0.055)
  ROOT.myTextAbs(7.1, 0.6*ymax, 1, "Optimized", 0.055)


  pad2.cd()
  pad2.SetGridy()
  h2.Draw("hist")
  if xaxis == "Processes":
    h2.GetXaxis().SetRangeUser(0,20)
  summary.configureLowerPad(h2, 0.5, 0.1, xaxis, v2+" / "+v1)

  canv.Print(os.path.join(directory,("%s.pdf" % xaxis)))
  canv.Print(os.path.join(directory,("%s.png" % xaxis)))


def plotHistogramRatio(h1,h2,v1,v2,lab,ALL,hisType,directory):

  leg = ROOT.TLegend(0.18,0.74-0.05*len(h1),0.4,0.74)
  leg.SetNColumns(2)
  leg.SetBorderSize(0)
  leg.SetFillColor(0)
  leg.SetFillStyle(0)
  leg.SetTextSize(0.045)

  hs = ROOT.THStack()
  hsR = ROOT.THStack()

  underflow1 = 0
  underflow2 = 0
  all1 = 0
  all2 = 0

  uniqueName = ""

  for p in particles:
    if not p in h1.keys(): continue
    if not p in h2.keys(): continue
    uniqueName = h1[p].GetName()
    hs.Add(h2[p])
    hs.Add(h1[p])
    h1[p].SetLineColor(colors[p]+2)
    h2[p].SetLineColor(colorsNew[p])
    underflow1 += h1[p].GetBinContent(0)
    underflow2 += h2[p].GetBinContent(0)
    all1 += h1[p].GetEntries()
    all2 += h2[p].GetEntries()
    # print(h1[p]," ",all1)
    hR = h2[p].Clone()
    hR.Divide(h1[p])
    # hR.Smooth()
    hR.SetMarkerSize(0)
    hR.SetMarkerStyle(0)
    hR.SetBinContent(0,1)
    hsR.Add(hR)
    leg.AddEntry(h1[p],"#font[42]{%s}"%p,"l")
    leg.AddEntry(h2[p],"#font[42]{%s}"%p,"l")

  if not uniqueName:
    return

  nologx = True
  if hisType in ["numberOfSteps"]:
    nologx = False
  nology = False
  if hisType in ["CumulativeInitialE","CumulativeNumberOfStepsPerInitialE"]:
    nology = True
  # if hisType in ["averageNumberOfStepsPerInitialE"]:
  #   nology = True
  canv, pad1, pad2 = histograms.getCanvas(uniqueName+"canv", nologx, nology)

  frac = 100*all1/ALL
  frac1 = 100*underflow1/all1
  frac2 = 100*underflow2/all2

  pad1.cd()
  hs.Draw("nostack hist")
  if not nology:
    hs.SetMaximum(hs.GetStack().Last().GetMaximum()*1e3)
  if AtlasStyle:
    ROOT.ATLASLabel(0.18,0.87,"Simulation Internal",1)
    ROOT.myText(0.18,0.81,1,"#sqrt{s}=13 TeV, %s (%2.1f%s)" % (lab,frac,"%"))
    # ROOT.myText(0.18,0.75,1,"particle: %s" % p)
    underTxt = "under: %1.2E (%2.2f%s) %1.2E (%2.2f%s)" % (underflow1, frac1, "%", underflow2, frac2, "%")
    ROOT.myText(0.51,0.81,1,underTxt)
    ROOT.myText(0.58,0.87,1,"%s                  %s" % (v1,v2))
  if hisType in ["numberOfSteps"]:
    shiftx = 0.5
    legTemp = leg.Clone()
    legTemp.SetX1(legTemp.GetX1()+shiftx)
    legTemp.SetX2(legTemp.GetX2()+shiftx)
    legTemp.Draw()
    ROOT.myText(0.18+shiftx,0.75,1,"%s:   %s:" % (v1,v2))
  else:
    leg.Draw()
    ROOT.myText(0.18,0.75,1,"%s:   %s:" % (v1,v2))
  yaxis = ""
  if hisType in ["numberOfSteps","InitialE"]:
    yaxis = "Tracks"
  elif hisType in ["CumulativeInitialE"]:
    yaxis = "Cumulative tracks"
  elif hisType in ["CumulativeNumberOfStepsPerInitialE"]:
    yaxis = "Cumulative steps"
  elif hisType == "averageNumberOfStepsPerInitialE":
    yaxis = "Avg. number of steps in track"
  histograms.configureUpperPad(hs, yaxis)


  pad2.cd()
  hsR.Draw("nostack hist")
  histograms.configureLowerPad(hsR, 0.5 if "Leng" in hisType else 1.0, labelAll[hisType], v2+"/"+v1)

  canv.Print(os.path.join(directory,hisType+".pdf"))
  canv.Print(os.path.join(directory,hisType+".png"))

def plot2D(h,G4Version,v,ALL_STEPS,yaxis,directory):
  if not h:
    return
  uniqueName = h.GetName()+G4Version+v
  canv = ROOT.TCanvas(uniqueName+"c",uniqueName+"c",800,600)
  canv.SetRightMargin(0.20)
  canv.SetTopMargin(0.07)
  if not yaxis:
    canv.SetLogz()
  nx = h.GetNbinsX()
  ny = h.GetNbinsY()
  integral = h.Integral(0,nx+1,0,ny+1)
  h.Draw("COLZ")
  h.GetXaxis().SetTitle("z [mm]")
  h.GetYaxis().SetTitle("r [mm]")
  h.GetZaxis().SetTitle("Steps" if not yaxis else yaxis)
  h.GetZaxis().SetTitleOffset(1.2)
  h.GetXaxis().SetNdivisions(505)
  if AtlasStyle:
    ROOT.ATLASLabel(0.18,0.87,"Simulation Internal",1)
    ROOT.myText(0.18,0.81,1,"#sqrt{s}=13 TeV, G4.%s" % G4Version)
    ROOT.myText(0.18,0.75,1,v + (" (%2.1f%s)" % (100*integral/ALL_STEPS,"%")) if ALL_STEPS else "")
  ROOT.gPad.RedrawAxis()
  if not yaxis:
    canv.Print(os.path.join(directory,"2DMapGeant4.%s.pdf" % G4Version))
    canv.Print(os.path.join(directory,"2DMapGeant4.%s.png" % G4Version))
  else:
    canv.Print(os.path.join(directory,"2DMapDiff.pdf"))
    canv.Print(os.path.join(directory,"2DMapDiff.png"))
