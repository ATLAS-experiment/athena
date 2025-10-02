import sys, os, glob
import ROOT
import re

def copyHist(indir, outdir, dirfilt):
    print("Decending into directory", indir.GetName())
    for key in indir.GetListOfKeys():
        obj = indir.Get(key.GetName())

        isdir = obj.InheritsFrom("TDirectory")
        ishist = obj.InheritsFrom("TH1")

        match = True
        if isdir: match = re.match(dirfilt, obj.GetName()) != None

        #print(key.GetName(), match, isdir, ishist)
        if not match: continue

        outdir.cd()
        if isdir:
            newoutdir = outdir.mkdir(obj.GetName())
            newindir = obj
            copyHist(newindir, newoutdir, dirfilt)

        if ishist:
            obj.Write()


dirfilt = 'run_.*|GLOBAL|lb_.*|lowStat_LB.*|DQTGlobalWZFinder'

source = 'test.root'
if len(sys.argv) > 1:
    source = sys.argv[1]

# output file
target = 'copy.root'
if len(sys.argv) > 2:
    target = sys.argv[2]

print ("Copy from", source, "to", target, "with directory filter", dirfilt)

# open source file
infile = ROOT.TFile(source)
if not infile.IsOpen():
    print ('Problem with input file', source)
    exit(1)
# open output file
outfile = ROOT.TFile.Open(target, "RECREATE")
if not outfile.IsOpen():
    print ('Problem with output file', target)
    exit(1)

copyHist(infile, outfile, dirfilt)

infile.Close()
outfile.Close()
