## Setting path to Pythia8 data
import os
os.environ['PYTHIA8DATA']=os.environ['PY8PATH']+'/share/Pythia8/xmldoc/'

# Overwrite hadronizationModel metadata field
evgenConfig.hadronizationModel = "SherpaPythia8"

## Adjusting ANY other Pythia8 settings via the PYTHIA8 block overrides these settings here.
## So you have to include them again in your own settings
## even if you do not wish to explicitely change their values.
genSeq.Sherpa_i.BaseFragment += """
SHERPA_LDADD: SherpaPythia
FRAGMENTATION: Pythia8
PYTHIA8:
  DECAYS: true
  PARAMETERS:
    - Main:timesAllowErrors: 500
    - ParticleDecays:limitTau0: on
    - ParticleDecays:tau0Max: 10.0
"""
