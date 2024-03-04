evgenConfig.description = "SFGen MC gamma + gamma pp collisions at 13000 GeV to 2 muons with dissociation on single side"
evgenConfig.keywords = ["2photon","2muon","dissociation"]
evgenConfig.contact = ["lucian.harland-lang@physics.ox.ac.uk"]

from SFGen_i.SFGenUtils import SFGenConfig, SFGenRun

#class with the sfgen initialization parameters.  Please see SFGenUtils for a complete list of tunable parameters.
scConfig = SFGenConfig(runArgs)

scConfig.PDFname = 'SF_MSHT20qed_nnlo' # PDF set name
scConfig.PDFmember = 0              # PDF member
scConfig.proc = 2                  # Process number, see SFGen Manual for labelling at https://sfgen.hepforge.org/

scConfig.diff = 'sdb'                # interaction: elastic ('el'), single ('sd','sda','sdb') and double ('dd') dissociation.
scConfig.genunw  = True
scConfig.ymin  = -5.0              # Minimum dilepton rapidity
scConfig.ymax  = 5.0               # Maximum dilepton rapidity
scConfig.mmin  = 20                 # Minimum dilepton mass
scConfig.mmax  = 2000               # Maximum dilepton mass
scConfig.gencuts  = True           # Generate cuts below

scConfig.ptamin  = 10.0             # Minimum pT of outgoing object a 
scConfig.ptbmin  = 10.             # Minimum pT of outgoing object b 
scConfig.etaamin  = -2.5           # Minimum eta of outgoing object a
scConfig.etaamax   = 2.5           # Maximum eta of outgoing object a
scConfig.etabmin  = -2.5           # Minimum eta of outgoing object b
scConfig.etabmax   = 2.5           # Maximum eta of outgoing object b


SFGenRun(scConfig, genSeq)
include('SFGen_i/Pythia8_SD_Common.py')
