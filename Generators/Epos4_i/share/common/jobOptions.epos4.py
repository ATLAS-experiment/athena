# ----------------------------------------------------------------------
# Example Epos4 JO file, minimaly working example
# Author: Andrii Verbytskyi
#

include("EvgenProdTools/StdEvgenSetup.py")

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

# ----------------------------------------------------------------------
# Epos4 control card
# ----------------------------------------------------------------------
content = """!EPOS4 configuration file
!to define the reaction and simulation options
!to define the output
!to add analysis plugins (optional)
!--------------------------------------------------------------------
! proton-proton collision no hydro no hadronic cascade
!--------------------------------------------------------------------
application hadron !hadron-hadron, hadron-nucleus, or nucleus-nucleus
set laproj 1 !projectile atomic number
set maproj 1 !projectile mass number
set latarg 1 !target atomic number
set matarg 1 !target mass number
set ecms 13000 !sqrt(s)_pp
set istmax 25 !max status considered for storage
set iranphi 1 !for internal use.
!if iranphi=1 event will be rotated such that the impact parameter angle
!and the (n=2) event plane angle (based on string segments) coincide.
!Particles rotated back at the end.
ftime on !string formation time non-zero
!suppressed decays (using EPOS id codes, see src/KWt/idt.dt)
nodecays 110 20 2130 -2130 2230 -2230 1130 -1130 1330 -1330 2330 -2330 3331 -3331 end
core off !core/corona not activated
hydro off !hydro not activated
eos off !eos not activated
hacas off !hadronic cascade not activated
set ninicon 1 !number of initial conditions used for hydro evolution
set nfull 200 !number of events
set nfreeze 1 !number of freeze out events per hydro event
set modsho 1 !printout every modsho events
set centrality 0 !0=min bias
"""

with open("foo.optns", "w") as f:
    f.write(content)

# ----------------------------------------------------------------------
# Epos4 initialization 
# ----------------------------------------------------------------------
from Epos4_i.Epos4_iConf import Epos4
Ep4 = Epos4()
Ep4.BeamMomentum     = -6500.0
Ep4.TargetMomentum   = 6500.0
Ep4.InputCard   = "foo.optns"
genSeq  += Ep4
evgenConfig.generators += ["Epos4"]


evgenConfig.description = "Simple EPOS4 production."
evgenConfig.keywords = [ "minbias" ]
evgenConfig.contact  = [ "andrii.verbytskyi@mpp.mpg.de" ]
evgenConfig.nEventsPerJob = 10000

# ----------------------------------------------------------------------
# Drop checks
# ----------------------------------------------------------------------

if hasattr(fixSeq, "FixHepMC"):
   fixSeq.remove(FixHepMC())

if hasattr(testSeq, "TestHepMC"):
   testSeq.remove(TestHepMC())

# ----------------------------------------------------------------------
# Postprocessing options
# ----------------------------------------------------------------------
#include("EvgenProdTools/postJO.CopyWeights.py")
#include("EvgenProdTools/postJO.PoolOutput.py")
#include("EvgenProdTools/postJO.DumpMC.py")

