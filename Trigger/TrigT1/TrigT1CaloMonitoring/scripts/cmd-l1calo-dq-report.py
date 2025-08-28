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

    for run in runNumbers:
        files = glob.glob(f"/eos/atlas/atlastier0/rucio/{project}/{stream}/*{run}/*merge.HIST.{'f' if stream!='express_express' else 'x'}*/*")
        if run != "*" and len(files)==0:
            input(f"No HIST file for run {run} and stream {stream}. {'Please report this' if stream=='express_express' else 'Perhaps bulk processing not ready yet, please try again later'}. Press any key to continue ...")
        for f in files:
            runNum = f.split("/")[7].lstrip("0")
            if (stream=="express_express") and ((since and os.path.getmtime(f)<since.timestamp()) or (until and os.path.getmtime(f)>until.timestamp())):
                x = input(f"Run {runNum} is outside of your reporting dates ({since} to {until}), you do not need to review it. Do you still want to include it? [n]/y:")
                if x=="" or x=="n": continue
            if runNum in out: print("Warning: multiple HIST files for run",runNum," (using latter):",out[runNum],f)
            out[runNum] = f
    return out

def getMetadata(file):
    f = ROOT.TFile(file)
    runNumber = [a.GetName() for a in f.GetListOfKeys() if a.GetName().startswith("run_")][0]
    out = {}
    out["release"] = f.Get(f"{runNumber}/GLOBAL/DQTDataFlow/m_release_stage_lowStat").GetXaxis().GetBinLabel(1).split("-")[-1]
    # lumi is in fb^-1
    lumiPlot = f.Get(f"{runNumber}/GLOBAL/Luminosity/AnyTrigger/lumiWeight_vs_LB") # missing in cosmic runs
    out["lumi"] = lumiPlot.Integral()*1e-9 if lumiPlot else 0 # includes LB without Stable beams
    return out

def runDQ(args):

    runNumbers=args.runNumbers
    fromDate=args.since
    toDate=args.until
    output=args.output
    commentFile=args.commentFile
    doWeb=args.web

    runsFromFiles = {}

    if runNumbers==[]:
        if len(args.filesInput)>0:
            # open the files and determine run numbers
            for f in args.filesInput:
                rf = ROOT.TFile.Open(f)
                for k in rf.GetListOfKeys():
                    if k.GetName().startswith("run_"):
                        runsFromFiles[k.GetName()[4:]] = f
                rf.Close()
        else:
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

    if "previous" in runNumbers:
        # replace with list of previously generated han results for this stream

        previousRuns = []
        for f in glob.glob(f"./hanResults/{args.stream}*.root"):
            if os.path.getmtime(f)>=datetime.datetime.combine(args.since, datetime.datetime.min.time()).timestamp():
                previousRuns += [f.split("/")[-1].split(".")[1].split("_")[1]]
        print("Including previous runs from this week:",previousRuns)
        newRunList = []
        for r in runNumbers:
            if r=="previous":
                newRunList+=previousRuns
            else:
                newRunList+=[r]
        runNumbers = newRunList

    histResults = {}
    allRuns = []
    allMeta = {}

    spotDetections = {}

    import base64

    ROOT.gROOT.SetBatch(True)

    tmpFileName = "/tmp/plt.png"
    i=0
    while os.path.exists(tmpFileName):
        tmpFileName = f"/tmp/plt{i}.png"
        i+=1

    # get the date of the most recent monday that isn't today
    firstDraw = True
    for runNum,file in (pbar := tqdm.tqdm(runsFromFiles.items() if len(runsFromFiles)>0 else getFiles(runNumbers=runNumbers,since=datetime.datetime.combine(fromDate, datetime.datetime.min.time()),until=datetime.datetime.combine(toDate, datetime.datetime.min.time()),stream=args.stream).items(),desc="Collating DQ results",unit='run')):
        if len(runNumbers) and runNum not in runNumbers and "*" not in runNumbers: continue
        allRuns += [runNum]
        allMeta[runNum] = getMetadata(file)
        allMeta[runNum]["project"] = file.split("/")[5]
        hanFiles = glob.glob(f"./hanResults/{args.stream}.run_{runNum}_han*.root")
        if len(hanFiles)==0:
            # will run the DQ monitoring, but also will run web display if doWeb
            # if doWeb, can just copy the result from the webdisplay run to our local dir, avoiding a need to rerun
            if doWeb:
                import random
                r = random.randrange(100000,999999)
                cmdStr = f"DQWebDisplay.py {file} TestDisplay \"{r}\""
                import subprocess
                print("Executing:",cmdStr, "... Please be patient ...")
                process = subprocess.Popen(cmdStr, shell=True)
                process.wait()
                if process.returncode==0:
                    # copy the results file into the hanResults dir ...
                    import shutil
                    #os.rename(f"./hanResults/run_{runNum}_han.root",f"./hanResults/run_{runNum}_han_{r}.root")
                    shutil.copy(f"/afs/cern.ch/user/a/atlasdqm/dqmdisk/han_results/test/{r}/{args.stream}/run_{runNum}/run_{runNum}_han.root",f"./hanResults/{args.stream}.run_{runNum}_han.{r}.root")
                    hanFiles = [f"./hanResults/{args.stream}.run_{runNum}_han.{r}.root"]
                else:
                    print("WARNING: Failed to run DQWebDisplay ... please report this!")
                    input("Press Enter to continue (will run DQ algorithms locally)...")
            if len(hanFiles)==0:
                ROOT.dqutils.MonitoringFile(file).getHanResults("./hanResults",file,hcfg,"","")
                os.rename(f"./hanResults/run_{runNum}_han.root",f"./hanResults/{args.stream}.run_{runNum}_han.root")
                hanFiles = [f"./hanResults/{args.stream}.run_{runNum}_han.root"]
            print("Saved results of DQ algorithms to:",hanFiles[0])
        allMeta[runNum]["hanFile"] = hanFiles[0].split("/")[-1]
        f = ROOT.TFile(hanFiles[0])
        ROOT.gStyle.SetOptStat(False) # no stats boxes
        ROOT.gStyle.SetPadRightMargin(0.15)
        ROOT.gErrorIgnoreLevel = ROOT.kWarning
        def findHists(d,firstDraw):
            for k in d.GetListOfKeys():
                if k.IsFolder():
                    if k.GetName().endswith("_"): continue # don't navigate into folders of results of hists
                    findHists(k.ReadObj(),firstDraw)
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
                        pbar.set_description("Drawing " + h.GetName() + ("(Please be patient, the first draw is the slowest)" if firstDraw else ""))
                        firstDraw=False
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
                        ROOT.gPad.SaveAs(tmpFileName)
                        ROOT.gPad.SetFillColor(ROOT.kWhite)
                        ROOT.gStyle.SetPalette(ROOT.kBird)
                        ROOT.gPad.SetLogx(False);ROOT.gPad.SetLogy(False);ROOT.gPad.SetLogz(False)
                        ROOT.gPad.SetGridx(False);ROOT.gPad.SetGridy(False)

                        pltCode = base64.b64encode(open(tmpFileName, 'rb').read()).decode('utf-8').replace('\n','')
                        os.remove(tmpFileName)
                        histResults[histPath][runNum]["imageCode"] = pltCode

        findHists(f,firstDraw)

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
        if args.stream!="express_express":
            outFile.write(f"<h1>L1Calo Validation DQ Report - {fromDate} to {toDate} - {args.stream} - Author: {os.getlogin()}</h1>\n")
        else:
            outFile.write(f"<h1>L1Calo DQ Report - {fromDate} to {toDate} - Author: {os.getlogin()}</h1>\n")
        outFile.write("<table cellspacing=0><thead><tr><td rowspan=\"5\" valign=top><a href=\"#ByteSreamDecoders\">ByteSreamDecoders</a><br><a href=\"#Efficiency\">Efficiency</a><br><a href=\"#Inputs\">Inputs</a><br><a href=\"#Outputs\">Outputs</a><br><a href=\"#PpmTrex\">PpmTrex</a><br><a href=\"#Sim\">Sim</a></td><td align=right>RunNumber:</td>\n")
        for r in allRuns: outFile.write(f"<td align=center><a href='https://atlas-runquery.cern.ch/query.py?q=find+run+{r}+%2F+show+all' target='_blank'>{r}</a></td>\n")
        outFile.write("</td><tr><td align=right>Athena Release:</td>")
        for r in allRuns: outFile.write(f"<td align=center>{allMeta[r]['release']}</td>\n")
        outFile.write("</td><tr><td align=right>Project:</td>")
        for r in allRuns: outFile.write(f"<td align=center>{allMeta[r]['project']}</td>\n")
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
                    linkUrl = f"https://atlasdqm.cern.ch/webdisplay/tier0/1/{allMeta[r]['hanFile'].split('.')[0]}/run_{r}"
                    if len(allMeta[r]["hanFile"].split("."))==4: # True if WebDisplay was also run in the report generation
                        webDisplayNum = allMeta[r]["hanFile"].split(".")[-2]
                        linkUrl = f"https://atlasdqm.cern.ch/webdisplay/test/{webDisplayNum}/{allMeta[r]['hanFile'].split('.')[0]}/run_{r}"
                    outFile.write(f"<td valign=top align=center width=150><img title=\"{altText}\" src=\"data:image/png;base64,{v[r]['imageCode']}\" width=150 style='border:2px solid {col}' onclick=\"window.open('{linkUrl}/run{k}','_blank');\" ondblclick=\"window.open(this.src, '_blank');\"/><br><font size='1'>{v[r].get('comment','')}</font></td>")

            outFile.write("</tr>\n")
        outFile.write("</tbody></table>\n")
        outFile.write("<h4>Spot Detection</h4><br>+1 for each run where location is Hot, -1 where it is Cold/Dead<br>")
        ROOT.gStyle.SetPalette(ROOT.kRainBow)
        for k,v in spotDetections.items():
            if "Expert/" not in k: continue
            outFile.write(k+"<br>")
            v.Draw("COL1Z")
            ROOT.gPad.SaveAs(tmpFileName)
            pltCode = base64.b64encode(open(tmpFileName, 'rb').read()).decode('utf-8').replace('\n','')
            os.remove(tmpFileName)
            outFile.write(f"<img src=\"data:image/png;base64,{pltCode}\" width=400><br>")

        outFile.write("</body></html>\n")

    print("DQ Report Created @",output)


    return histResults

if __name__=="__main__":

    # defaults to last monday!
    today = datetime.date.today()
    fromDate = today + datetime.timedelta(days=-(today.weekday() if today.weekday()>0 else 7), weeks=0)

    import argparse
    parser = argparse.ArgumentParser(prog="l1calo-dq-report",description="Generate an L1Calo DQ Report. Run without specifying any runs to list available runs",formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--since",type=lambda s: datetime.datetime.strptime(s, '%Y-%m-%d').date(),default=fromDate,help="from")
    parser.add_argument("--until",type=lambda s: datetime.datetime.strptime(s, '%Y-%m-%d').date(),default=None,help="If unspecified, will make 1 week long")
    parser.add_argument("--stream",default="express_express",help="Which stream to process. Using something other than express_express for validation reports")
    parser.add_argument("--web",default=True,action="store_true",help="If given, will generate WebDisplay page and set links to them from report")
    parser.add_argument("--filesInput",nargs="*",default=[],help="Use this option to list the HIST files to process, instead of run numbers")
    parser.add_argument("--commentFile",type=lambda s: os.path.expanduser(s),default="dqComments.txt",help="path to your comments file")
    parser.add_argument("-o","--output",default="dqReport.html" if not os.path.exists(os.path.expanduser("~/www/")) else (os.path.expanduser("~/www/")+"dqReport.html"),help="Where to create the report")
    parser.add_argument("runNumbers",nargs="*",help="runs to include in the report, use a '*' to indicate all runs between time ranges specified in --since and --until. Use word 'previous' to include previously generated runs from this week in the same stream. If None given, will list available runs")

    args = parser.parse_args()

    # defaults to a week later
    if args.until is None: args.until = args.since + datetime.timedelta(weeks=1)

    # include stream in dqReport name if doing a validation check
    if args.stream != "express_express":
        args.output = args.output.replace(".html","."+args.stream+".html")

    runDQ(args)