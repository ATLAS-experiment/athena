#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import ROOT
import argparse
import subprocess

if __name__=="__main__":

    parser = argparse.ArgumentParser(prog="l1calo-dq-test",description="Runs the DQ algorithm for a given histogram",formatter_class=argparse.ArgumentDefaultsHelpFormatter
                                 )
    parser.add_argument("run",help="run number")
    parser.add_argument("hist",help="hist path and name")
    args = parser.parse_args()

    xx = [f for f in subprocess.run(['l1calo-dq-file',args.run],stdout=subprocess.PIPE).stdout.decode("utf-8").split('\n') if f!='']
    if len(xx)==0:
        print("ERROR, could not get file for run",args.run)
        exit(1)
    f = ROOT.TFile(xx[0])
    #h = f.FindObjectAny(args.hist)
    h = f.Get("run_"+args.run+"/"+args.hist)
    if not h:
        print("Cannot find histogram:",args.hist)
        exit(1)
    print("INPUT HISTOGRAM:",h)

    import os
    cFile = ROOT.TFile(os.path.expandvars("$BuildArea/$CMTCONFIG/data/DataQualityConfigurations/collisions_run.hcfg"))
    conf = cFile.Get("top_level").GetNode(args.hist.rsplit('/',1)[0]).GetAssessor(args.hist)
    cItr = conf.GetAllAlgPars()
    c = ROOT.dqm_algorithms.tools.SimpleAlgorithmConfig()
    print("DQ ALGORITHM:",conf.GetAlgName())
    exec(f"a = ROOT.dqm_algorithms.{conf.GetAlgName()}()")
    print("DQ CONFIGURATION:")
    while (x := cItr.Next()):
        print("  ",x.GetName(),"=",x.GetValue())
        c.addParameter(x.GetName(),x.GetValue())
    cItr = conf.GetAllAlgStrPars()
    while (x := cItr.Next()):
        print("  ",x.GetName(),"=",x.GetValue())
        c.addGenericParameter(x.GetName(),x.GetValue())

    r = a.execute("testAlg",h,c)

    print("RESULTS:")
    print("---------------------")
    print("Status=",r.status_)
    print(r.tags_)
