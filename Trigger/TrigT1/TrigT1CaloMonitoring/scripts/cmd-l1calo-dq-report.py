#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import ROOT
import glob
import os
import datetime
import tqdm
import re

def getFiles(runNumbers=[],since=None,until=None,stream="express_express",project="data25_13p6TeV"):
    if since is None: since = datetime.datetime.now() - datetime.timedelta(days=7)
    if until is None: until = datetime.datetime.now()
    out = {}
    if len(runNumbers)==0 or runNumbers==['*']:
        runNumbers = ["*"]
    else:
        # user specified runNumbers, so no need to filter on project or dates
        project = "*"
        since = None
        until = None

    for run in runNumbers:
        for f in glob.glob(f"/eos/atlas/atlastier0/rucio/{project}/{stream}/*{run}/*merge.HIST.x*/*"):
            runNum = f.split("/")[7].lstrip("0")
            if (since and os.path.getmtime(f)<since.timestamp()) or (until and os.path.getmtime(f)>until.timestamp()): continue
            if runNum in out: print("Warning, multiple HIST files for run",runNum)
            out[runNum] = f
    return out

def getMetadata(file):
    f = ROOT.TFile(file)
    runNumber = [a.GetName() for a in f.GetListOfKeys() if a.GetName().startswith("run_")][0]
    out = {}
    out["release"] = f.Get(f"{runNumber}/GLOBAL/DQTDataFlow/m_release_stage_lowStat").GetXaxis().GetBinLabel(1).split("-")[-1]
    # lumi is in fb^-1
    out["lumi"] = f.Get(f"{runNumber}/GLOBAL/Luminosity/AnyTrigger/lumiWeight_vs_LB").Integral()*1e-9 # includes LB without Stable beams
    return out

def runDQ(runNumbers,fromDate,toDate,output,commentFile):

    if runNumbers==[]:
        # no runs given and no "*" specified, so get list of run numbers
        print("Available runs from",fromDate,"to",toDate,":")
        for projName,projPattern in [("pp","data2*eV"),("cosmic","data2*_cos"),("hi","data2*_hi")]:
            print(f" {projName}:")
            for runNum,file in getFiles(project=projPattern,runNumbers=runNumbers,since=datetime.datetime.combine(fromDate, datetime.datetime.min.time()),until=datetime.datetime.combine(toDate, datetime.datetime.min.time())).items():
                print("  ",runNum)

        exit(0)

    if not os.path.exists(commentFile):
        print("ERROR: Comment file",commentFile,"does not exist")
        exit(1)

    hcfg = os.path.expandvars("$BuildArea/$CMTCONFIG/data/DataQualityConfigurations/collisions_run.hcfg")
    if not os.path.exists("./hanResults"): os.mkdir("./hanResults")

    histResults = {}
    allRuns = []
    allMeta = {}

    spotDetections = {}

    import base64

    ROOT.gROOT.SetBatch(True)

    # get the date of the most recent monday that isn't today

    # ["496922","496934","496908","497294","497370","497389","497457"]
    for runNum,file in (pbar := tqdm.tqdm(getFiles(runNumbers=runNumbers,since=datetime.datetime.combine(fromDate, datetime.datetime.min.time()),until=datetime.datetime.combine(toDate, datetime.datetime.min.time())).items(),desc="Collating DQ results",unit='run')):
        if len(runNumbers) and runNum not in runNumbers and "*" not in runNumbers: continue
        allRuns += [runNum]
        allMeta[runNum] = getMetadata(file)
        if not os.path.exists(f"./hanResults/run_{runNum}_han.root"):
            ROOT.dqutils.MonitoringFile(file).getHanResults("./hanResults",file,hcfg,"","")
        f = ROOT.TFile(f"./hanResults/run_{runNum}_han.root")
        ROOT.gStyle.SetOptStat(False) # no stats boxes
        ROOT.gStyle.SetPadRightMargin(0.15)
        ROOT.gErrorIgnoreLevel = ROOT.kWarning
        def findHists(d):
            for k in d.GetListOfKeys():
                if k.IsFolder():
                    if k.GetName().endswith("_"): continue # don't navigate into folders of results of hists
                    findHists(k.ReadObj())
                elif k.ReadObj().InheritsFrom("TH1") or k.ReadObj().InheritsFrom("TEfficiency"):
                    #print(d.GetPath()+k.GetName())
                    #d.ls()
                    #d.Get(k.GetName()+"_").ls()
                    res = d.Get(k.GetName() + "_/Results")
                    if res is None:
                        print(d.GetPath()+"/"+k.GetName(),"ERROR:" + k.GetName())
                    else:
                        histPath = d.GetPath().split(":",1)[-1] + "/"+k.GetName()
                        if histPath not in histResults: histResults[histPath] = {}
                        histResults[histPath][runNum] = eval(res.GetString().Data())
                        conf = d.Get(k.GetName() + "_/Config")
                        dispOpt = []
                        h = k.ReadObj()
                        if conf:
                            confDict = eval(conf.GetString().Data())
                            if "name" in confDict:
                                histResults[histPath][runNum]["algorithm"] = confDict["name"]
                                if confDict["name"]=="L1Calo_BinsDiffFromStripMedian" and "detail/" not in histPath:
                                    # track Hot Cold Dead spots
                                    if histPath not in spotDetections:
                                        spotDetections[histPath] = h.Clone("spotDet_"+h.GetName())
                                        spotDetections[histPath].Reset()
                                        spotDetections[histPath].SetDirectory(0)
                                    for kk,vv in eval(res.GetString().Data()).items():
                                        if any([a in kk for a in ["_Hot(","_Cold(","_Dead("]]):
                                            # extract bin
                                            print(kk)
                                            loc = kk.split("(")[1].split(")")[0].split(",")
                                            loc = (int(loc[0]),int(loc[1]))
                                            binx = spotDetections[histPath].GetXaxis().GetBinCenter(loc[0])
                                            biny = spotDetections[histPath].GetYaxis().GetBinCenter(loc[1])
                                            spotDetections[histPath].Fill(binx,biny,1 if "_Hot" in kk else -1)

                            if "annotations" in confDict and "display" in confDict["annotations"]:
                                dispOpt = confDict["annotations"]["display"].split(",")


                        if h.InheritsFrom("TEfficiency") and h.GetTotalHistogram().GetEntries()==0:
                            # empty tefficiencies don't draw, so draw the total histogram instead
                            h = h.GetTotalHistogram()
                        dString = ""
                        for o in dispOpt:
                            if o.startswith("Draw="): dString = o[5:]
                        if dString=="" and h.InheritsFrom("TH1") and h.GetDimension()==2: dString = "COLZ"
                        # adjust z-axis on 2D heatmap plots, so that not swamped by an outlier
                        extraHist = None
                        if h.InheritsFrom("TH2") and "COL" in dString:
                            allVals = []
                            for i in range(1,h.GetNbinsX()+1):
                                for j in range(1,h.GetNbinsY()+1):
                                    if h.GetBinContent(i,j)!=0:
                                        allVals += [h.GetBinContent(i,j)]
                            allVals.sort()
                            if len(allVals)>0:
                                h.SetMaximum(allVals[int(len(allVals)*0.95)]*1.1+10) # go 10% bigger than 95th percentile value
                                # if any bins greater than this, will need to build a 'white hot' overlay hist
                                if any([v>h.GetMaximum() for v in allVals]):
                                    extraHist = h.Clone("extraHist")
                                    extraHist.Reset()
                                    isProf = h.InheritsFrom("TProfile2D")
                                    for i in range(h.GetNbinsX()+1):
                                        for j in range(h.GetNbinsY()+1):
                                            if isProf and h.GetBinContent(i,j)>h.GetMaximum(): extraHist.SetBinEntries(h.GetBin(i,j),1)
                                            extraHist.SetBinContent(i,j,h.GetMaximum() if h.GetBinContent(i,j)>h.GetMaximum() else 0)
                        pbar.set_description("Drawing " + h.GetName())
                        h.Draw(dString)
                        for o in dispOpt:
                            if o.startswith("SetPalette("): ROOT.gStyle.SetPalette(int(re.findall(r'\d+', o)[0]))
                            elif o.startswith("LogX"): ROOT.gPad.SetLogx(True)
                            elif o.startswith("LogY"): ROOT.gPad.SetLogy(True)
                            elif o.startswith("LogZ"): ROOT.gPad.SetLogz(True)
                            elif o.startswith("SetGridx"): ROOT.gPad.SetGridx(True)
                            elif o.startswith("SetGridy"): ROOT.gPad.SetGridy(True)

                        if h.InheritsFrom("TH1") and h.GetXaxis().GetTitle().strip() in ["LB","LBN","Lumi Block"]:
                            # zoom axis to ignore empty bins on left and right
                            r = []
                            for i in range(h.GetNbinsX()+1):
                                filled=False
                                if h.GetDimension()==2:
                                    for j in range(h.GetNbinsY()+1):
                                        if h.GetBinContent(i,j)!=0:
                                            filled=True
                                            break
                                elif h.GetDimension()==1:
                                    filled = (h.GetBinContent(i)!=0)
                                if filled:
                                    if len(r)==0:
                                        r = [i-1,i+1] # include one earlier and later bin
                                    else:
                                        r[1] = i+1
                            if len(r)>0: h.GetXaxis().SetRange(r[0],r[1])
                        if extraHist is not None:
                            extraHist.SetBit(ROOT.kCanDelete)
                            extraHist.SetFillColor(ROOT.kPink+6)
                            extraHist.SetFillStyle(1001)
                            extraHist.SetLineWidth(0)
                            extraHist.Draw("BOX same")
                            # ROOT.gPad.SetFillColor(ROOT.kGray)
                        ROOT.gPad.SaveAs("/tmp/plt.png")
                        ROOT.gPad.SetFillColor(ROOT.kWhite)
                        ROOT.gStyle.SetPalette(ROOT.kBird)
                        ROOT.gPad.SetLogx(False);ROOT.gPad.SetLogy(False);ROOT.gPad.SetLogz(False)
                        ROOT.gPad.SetGridx(False);ROOT.gPad.SetGridy(False)

                        pltCode = base64.b64encode(open('/tmp/plt.png', 'rb').read()).decode('utf-8').replace('\n','')
                        histResults[histPath][runNum]["imageCode"] = pltCode

        findHists(f)

    allRuns.sort()

    if len(allRuns)==0:
        print("No runs for the report")
        exit(1)

    # read plot comments file
    with open(commentFile,'r') as file:
        for line in file:
            if not line.strip(): continue # skip blank lines
            try:
                path,runs,comment = line.split(":",2)
            except ValueError as e:
                print("Comment line does not match required format:",line)
                raise e
            if path!="": path = "/L1Calo/Expert/"+path
            elif path not in histResults: histResults[path] = {} # allows for run-level comments
            if path not in histResults:
                print("WARNING: Unknown histogram path",path,"- cannot add comment")
                continue
            for run in runs.split(","):
                if (run=="" or path=="") and run not in histResults[path]: histResults[path][run] = {} # allows for all-run comments
                if run not in histResults[path]:
                    print("WARNING: Unknown run",run,"- cannot add comment")
                    continue
                if "comment" in histResults[path][run]: histResults[path][run]["comment"] += ";" + comment
                else: histResults[path][run]["comment"] = comment

    from DQDefects import DefectsDB
    ddb = DefectsDB()

    with open(output,'w') as outFile:
        outFile.write("""
<html><head><style>
thead {
  background-color: white;
  position: sticky;
  top: 0;
}
</style></head><body>
    """)
        outFile.write(f"<h1>L1Calo DQ Report - {fromDate} to {toDate} - Author: {os.getlogin()}</h1>\n")
        outFile.write("<table cellspacing=0><thead><tr><td rowspan=\"5\" valign=top><a href=\"#ByteSreamDecoders\">ByteSreamDecoders</a><br><a href=\"#Efficiency\">Efficiency</a><br><a href=\"#Inputs\">Inputs</a><br><a href=\"#Outputs\">Outputs</a><br><a href=\"#PpmTrex\">PpmTrex</a><br><a href=\"#Sim\">Sim</a></td><td align=right>RunNumber:</td>\n")
        for r in allRuns: outFile.write(f"<td align=center><a href='https://atlas-runquery.cern.ch/query.py?q=find+run+{r}+%2F+show+all' target='_blank'>{r}</a></td>\n")
        outFile.write("</td><tr><td align=right>Athena Release:</td>")
        for r in allRuns: outFile.write(f"<td align=center>{allMeta[r]['release']}</td>\n")
        outFile.write("</td><tr><td align=right>Approx Lumi (incl. unstable)/fb<sup>-1</sup>:</td>")
        for r in allRuns: outFile.write(f"<td align=center>{allMeta[r]['lumi']:.3f}</td>\n")
        outFile.write("</tr><tr><td align=right>Defects:</td>\n")
        # add the defects for the run
        for r in tqdm.tqdm(allRuns,desc="Accessing defects ..."):
            defects = ddb.retrieve(since=(int(r),0), until=(int(r),999999), primary_only=True)
            dStr = ""
            for d in defects:
                if not d.channel.startswith("TRIG_L1_CAL"): continue
                dStr += f"{d.since.lumi}-{d.until.lumi}:{d.channel.split('_',3)[-1]}<br>"
            outFile.write(f"  <td align=center>{dStr}</td>\n")
        outFile.write("</tr>\n")
        if "" in histResults:
            # we have run-level comments ... add row
            outFile.write("<tr><td align=right>Overall DQ Comments:</td>\n")
            for r in allRuns:
                cStr = histResults[''][r]["comment"] if r in histResults[''] else ""
                outFile.write(f"  <td align=center><font size=1>{cStr}</font></td>\n")
            outFile.write("</tr>\n")
        outFile.write("</thead><tbody>\n")
        currentDir = ""
        for k in sorted(histResults.keys(),key=lambda x: x.replace("/detail","/zzzdetail")):
            if "Expert/" not in k: continue
            v = histResults[k]
            # add a hr if starting a new top-level folder
            if currentDir != k.split("/")[3]:
                if currentDir != "":
                    outFile.write(f"<tr><td colspan=\"{len(allRuns)+2}\"><hr width=100%><div id=\"{k.split('/')[3]}\"></div></td></tr>\n")
                currentDir = k.split("/")[3]

            allUndefined = all([m["Status"]=="Undefined" for rr,m in v.items() if rr in allRuns])
            if "detail/" not in k and allUndefined: continue # check status is not always undefined if outside detail folder
            anyRed = any([m["Status"]=="Red" for rr,m in v.items() if rr in allRuns])
            anyYellow = any([m["Status"]=="Yellow" for rr,m in v.items() if rr in allRuns])
            if allUndefined: col = "#cccccc"
            elif anyRed: col = "#fc9797" if "detail/" in k else "#ff0000"
            elif anyYellow: col = "#fcd283" if "detail/" in k else "#ffa500"
            else: col = "#b3e8b3" if "detail/" in k else "#00dd00"
            bgColorStr = ' bgcolor=\"#eeeeee\"' if 'detail' in k else ''
            outFile.write(f"<tr{bgColorStr}><td colspan=\"2\" align=right><div style=\"color:{col}\">{k.replace('/L1Calo/Expert/','')}</div><br><font size=1>{v['']['comment'] if '' in v else ''}</font></td>\n")
            for r in allRuns:
                if r not in v:
                    outFile.write("<td align=center>N/A</td>\n")
                else:
                    if v[r]["Status"]=="Undefined": col = "#cccccc"
                    elif v[r]["Status"]=="Red": col = "#fc9797" if "detail/" in k else "#ff0000"
                    elif v[r]["Status"]=="Yellow": col = "#fcd283" if "detail/" in k else "#ffa500"
                    else: col = "#b3e8b3" if "detail/" in k else "#00dd00"
                    altText = ""
                    if "algorithm" in v[r]: altText += f"Algorithm:{v[r]['algorithm']}&#010;&#010;" # display algname first
                    for kk,vv in v[r].items():
                        if kk not in ["imageCode","Status","name","comment","algorithm"]: altText += f"{kk}:{vv}&#010;"
                    outFile.write(f"<td valign=top align=center width=150><img title=\"{altText}\" src=\"data:image/png;base64,{v[r]['imageCode']}\" width=150 style='border:2px solid {col}' onclick=\"window.open('https://atlasdqm.cern.ch/webdisplay/tier0/1/express_express/run_{r}/run{k}','_blank');\" ondblclick=\"window.open(this.src, '_blank');\"/><br><font size='1'>{v[r].get('comment','')}</font></td>")

            outFile.write("</tr>\n")
        outFile.write("</tbody></table>\n")
        outFile.write("<h4>Spot Detection</h4><br>+1 for each run where location is Hot, -1 where it is Cold/Dead<br>")
        ROOT.gStyle.SetPalette(ROOT.kRainBow)
        for k,v in spotDetections.items():
            if "Expert/" not in k: continue
            outFile.write(k+"<br>")
            v.Draw("COL1Z")
            ROOT.gPad.SaveAs("/tmp/plt.png")
            pltCode = base64.b64encode(open('/tmp/plt.png', 'rb').read()).decode('utf-8').replace('\n','')
            outFile.write(f"<img src=\"data:image/png;base64,{pltCode}\" width=400><br>")

        outFile.write("</body></html>\n")

    # histsByStatus = {"Yellow":[],"Red":[]}
    #
    # for k,v in histResults.items():
    #
    #     if v["Status"]=="Green" or v["Status"]=="Undefined": continue
    #     histsByStatus[v["Status"]] += [k]
    #
    # print(histsByStatus["Yellow"])

    return histResults

if __name__=="__main__":

    today = datetime.date.today()
    fromDate = today + datetime.timedelta(days=-(today.weekday() if today.weekday()>0 else 7), weeks=0)

    import argparse
    parser = argparse.ArgumentParser(prog="l1calo-dq-report",description="Generate an L1Calo DQ Report. Run without specifying any runs to list available runs",formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--since",type=lambda s: datetime.datetime.strptime(s, '%Y-%m-%d').date(),default=fromDate,help="from")
    parser.add_argument("--until",type=lambda s: datetime.datetime.strptime(s, '%Y-%m-%d').date(),default=None,help="If unspecified, will make 1 week long")
    parser.add_argument("--commentFile",type=lambda s: os.path.expanduser(s),default="dqComments.txt",help="path to your comments file")
    parser.add_argument("-o","--output",default="dqReport.html" if not os.path.exists(os.path.expanduser("~/www/")) else (os.path.expanduser("~/www/")+"dqReport.html"),help="Where to create the report")
    parser.add_argument("runNumbers",nargs="*",help="runs to include in the report, use a '*' to indicate all runs between time ranges specified in --since and --until. If None given, will list available runs")

    args = parser.parse_args()


    if args.until is None:
        args.until = args.since + datetime.timedelta(weeks=1)

    runDQ(runNumbers=args.runNumbers,fromDate=args.since,toDate=args.until,output=args.output,commentFile=args.commentFile)