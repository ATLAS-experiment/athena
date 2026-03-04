include("EvgenProdTools/StdEvgenSetup.py")
include("configFile.py")

theApp.EvtMax = 100

import os
from pathlib import Path
os.environ["EPO"] = os.environ["EPO"] + "/"
BASE = os.environ["EPO"]
os.environ["SRC"]    = f"{BASE}/src"
os.environ["SRCEXT"] = f"{BASE}/srcext"
os.environ["CONF"]   = f"{BASE}/config"
os.environ["DAT"]    = f"{BASE}/"
os.environ["OPT"]    = str(Path.cwd()) + "/"
os.environ["HTO"]    = str(Path.cwd()) + "/"
os.environ["CHK"]    = str(Path.cwd()) + "/"
OPT = os.environ["OPT"]
os.environ["OPX"] = str(Path.cwd()) + "/" if OPT == "./" else OPT

from Epos4_i.Epos4_iConf import Epos4
Ep4 = Epos4()
Ep4.BeamMomentum     = -runArgs.ecmEnergy/2.0 
Ep4.TargetMomentum   = runArgs.ecmEnergy/2.0 

energy              = float(runArgs.ecmEnergy)
number_of_events    = int(runArgs.maxEvents)

content = build_config_content(energy, number_of_events)
with open("foo.optns", "w") as f:
    f.write(content)

Ep4.InputCard   = "foo.optns"
genSeq  += Ep4

