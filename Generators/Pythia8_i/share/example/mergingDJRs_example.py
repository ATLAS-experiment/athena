# Check Pythia8 README for more details and the full steps on how to generate the lhe file.
# In this example, we considered the production of Z+2jets where Z decays to e+e-

include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")

#### Shower
evgenConfig.description = 'Example UserHook for computing CKKWL merging DJRs'
evgenConfig.keywords+=['Z','jets']
evgenConfig.generators += ['MadGraph', 'EvtGen', 'Pythia8']

include("Pythia8_i/Pythia8_LHEF.py")

PYTHIA8_nJetMax=2
PYTHIA8_Process='pp>e-e+'
PYTHIA8_Dparameter=0.4
PYTHIA8_TMS=20.0
PYTHIA8_nQuarksMerge=4

# Include to activate the CKKWL merging
include("Pythia8_i/Pythia8_CKKWL_kTMerge.py")

# Include to compute the DJRs and save them in ROOT file
include("Pythia8_i/Pythia8_mergingDJRs.py")

