#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from glob import glob
import subprocess
import os, shutil

import argparse

parser = argparse.ArgumentParser(description='Run trigger rate analysis with prescale keys')
parser.add_argument('aod_dir', type=str,
                    help='Directory containing AOD files from a rate reprocessing')
parser.add_argument('--hlt_ps', type=str, help='HLT Prescale JSON')
parser.add_argument('--l1_ps',  type=str, help='L1 Prescale JSON')
parser.add_argument('--targetlumi', type=float, default=2.0e34, help='Target luminosity')
parser.add_argument('--maxEvents', type=int, default=-1, help='Target luminosity')
parser.add_argument('--workdir',  type=str, default='rate_analysis/', help='Working directory')

args = parser.parse_args()

# Glob input files with abs path
filesIn=glob(f'{os.path.abspath(args.aod_dir)}/*')
print(f"Found {len(filesIn)} input files in '{args.aod_dir}'")

# Make working dir if needed
if not os.path.isdir(args.workdir):
    os.makedirs(args.workdir)

hlt_ps_filen=args.hlt_ps.split('/',1)[1] if '/' in args.hlt_ps else args.hlt_ps
l1_ps_filen=args.l1_ps.split('/',1)[1] if '/' in args.l1_ps else args.l1_ps
shutil.copy(args.hlt_ps, f"{args.workdir}/{hlt_ps_filen}")
shutil.copy(args.l1_ps, f"{args.workdir}/{l1_ps_filen}")

origdir = os.getcwd()
os.chdir(args.workdir)

# Execute analysis commands
filesIn_str = ",".join(filesIn)
cmd_analysis = [
    'RatesAnalysisFullMenu.py',
    '--inputPrescalesHLTJSON', hlt_ps_filen,
    '--inputPrescalesL1JSON', l1_ps_filen,
    '--outputHist=TrigCostRoot_Results.root',
    '--targetLuminosity', str(args.targetlumi),
    '--filesInput', filesIn_str
    ]
if args.maxEvents>0:
    cmd_analysis += ['--evtMax',str(args.maxEvents)]
cmd_postproc = [
    'RatesAnalysisPostProcessing.py',
    '--file=TrigCostRoot_Results.root',
    ]

print(f"Analysis cmd > {' '.join(cmd_analysis)}")
with open("log_RatesAnalysis.txt",'w') as log_analysis:
    assert(subprocess.run(cmd_analysis, stdout=log_analysis, stderr=subprocess.STDOUT, text=True).returncode==0)
log_analysis.close()
print(f"Postprocess cmd > {' '.join(cmd_postproc)}")
with open("log_RatesPostProcess.txt",'w') as log_postproc:
    assert(subprocess.run(cmd_postproc, stdout=log_postproc, stderr=subprocess.STDOUT, text=True).returncode==0)
log_postproc.close()

if not os.path.isdir('output/csv'):
    os.makedirs('output/csv')
else:
    shutil.rmtree('output')
    os.makedirs('output/csv')
for csv in ['Table_Rate_Group_HLT_All.csv','Table_Rate_ChainL1_HLT_All.csv','Table_Rate_ChainHLT_HLT_All.csv']:
    shutil.move(csv,'output/csv/')
for f in ['TrigCostRoot_Results.root','metadata.json','rates.json']:
    shutil.move(f,'output')

os.chdir(origdir)

print(f"Output placed in {args.workdir}/output")
print("Format for trig-cost page dirs: costMonitoring_[tag]_[runNumber]")
