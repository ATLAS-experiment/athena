#!/usr/bin/env python

import sys
import os, os.path
import numpy as np 
import json
from collections import OrderedDict
import re

test_status = int(os.environ["ATLAS_CTEST_TESTSTATUS"])
if test_status != 0:
    print(f"Post evaluation: ERROR: previous step went wrong (exit code {test_status})")
    sys.exit(test_status)

test_output_file = os.environ["ATLAS_CTEST_TESTNAME"]+".log"
bdt_config_file = os.environ["TAU_EFEX_BDT_CONFIG_PATH"]

def get_vector(lines, startswith):
    """
    Extract a vector of numbers from a line that starts with a string from a list of Athena log file lines
    """
    vals = [x[x.rfind(':')+1:].split() for x in lines if startswith in x]
    vals = np.array(vals).astype(int)
    return vals

out_log_content = open(test_output_file, "r").read()

# Filter to get only BDT-related lines
bdt_algo_lines = [l for l in out_log_content.split('\n') if re.findall(r"eFEXDriver\..+eFEXtauBDTAlgo", l)]

# Get supercells used to compute the variables, as appears in the test output. Check that they're the same as in the bdt config file
rgx_vartitle = re.compile(r"DEBUG\s+([0-9]+) is (l[0-4]_d[0-9_pld]*), sum of supercells$")
rgx_coretitle = re.compile(r"DEBUG\s+([0-9]+) is CORE, sum of supercells$")
rgx_etaphilayer = re.compile(r"DEBUG\s+eta=([0-9]+)\s+phi=([0-9]+)\s+layer=([0-9]+)$")
variables = OrderedDict()
last_var_line = [i for i,x in enumerate(bdt_algo_lines) if "Will use sum of supercells" in x][0]
for l in bdt_algo_lines[:last_var_line]:
    vartitle = rgx_vartitle.findall(l)
    coretitle = rgx_coretitle.findall(l)
    etaphilayer = rgx_etaphilayer.findall(l)
    if len(vartitle) > 0:
        variables[vartitle[0][1]] = []
        vt = vartitle[0][1]
    elif len(coretitle) > 0:
        vt="CORE"
        variables[vt] = []
    elif len(etaphilayer) > 0:
        variables[vt].append([int(x) for x in etaphilayer[0]])

bdt_config=json.load(open(bdt_config_file, "r"))

assert set(v['name'] for v in bdt_config['variables']) == set(variables.keys()), "Variables as read from this test's log file are not as they appear in the data/bdt_config.json. They must be consistent with each other."

for v in bdt_config['variables']:
    assert sorted(v['scells']) == sorted(variables[v['name']]), f"For BDT variable {v['name']}, the supercells used to compute it are different in the log file vs. in data/bdt_config.json"

# Check BDT scores are within range
bdt_scores=get_vector(bdt_algo_lines, 'DEBUG BDT Score: ').reshape(-1)
assert ((bdt_scores>500) &(bdt_scores < 2000)).all()
