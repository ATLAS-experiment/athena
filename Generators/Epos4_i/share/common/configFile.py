#!/usr/bin/env python3

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


def build_config_content(energy, number_of_events, laproj=1, maproj=1, latarg=1, matarg=1):
    content = f"""!EPOS4 configuration file
!to define the reaction and simulation options
!to define the output
!to add analysis plugins (optional)
!--------------------------------------------------------------------
! proton-proton collision no hydro no hadronic cascade
!--------------------------------------------------------------------
application hadron !hadron-hadron, hadron-nucleus, or nucleus-nucleus
set laproj {laproj} !projectile atomic number
set maproj {maproj} !projectile mass number
set latarg {latarg} !target atomic number
set matarg {matarg} !target mass number
set ecms {energy} !sqrt(s)_pp
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
set nfull {number_of_events} !number of events
set nfreeze 1 !number of freeze out events per hydro event
set modsho 1 !printout every modsho events
set centrality 0 !0=min bias
"""

    

    return content
