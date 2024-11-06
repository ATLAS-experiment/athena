# Simple Sherpa job option example

Event generation jobs in ATLAS are steered using jobOption files and run using the `Gen_tf.py` transform as described in [PmgMcSoftware#Production_transforms_and_job_op](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgMcSoftware#Production_transforms_and_job_op).

A simple example JO for running Sherpa v3.x on-the-fly within Athena is contained in this directory and will be explained below:

```yaml
include("Sherpa_i/Base_Fragment.py")
include("Sherpa_i/PDF4LHC21.py")

evgenConfig.description = "Sherpa 3.0.x example JO, Z+0,1-jet production."
evgenConfig.keywords = [ "2lepton" ]
evgenConfig.contact  = [ "atlas-generators-sherpa@cern.ch", "chris.g@cern.ch"]
evgenConfig.nEventsPerJob = 10000

genSeq.Sherpa_i.RunCard="""
 ME_SIGNAL_GENERATOR: Amegic

PROCESSES:
- 93 93 -> 11 -11 93{1}:
  Order: {QCD: 0, EW: 2}
  CKKW: 20

SELECTORS:
- [Mass, 11, -11, 40, E_CMS]
"""

genSeq.Sherpa_i.OpenLoopsLibs = []
genSeq.Sherpa_i.ExtraFiles = []
genSeq.Sherpa_i.NCores = 1
```

Features from top to bottom:
* Common include files containing all the default settings, which can be found in the [Sherpa_i interface package of Athena](../../../share/common)
* Evgen metadata
* A Sherpa run card containing the process-specific settings, e.g. hard scattering setup, ME level cuts, model settings. For details on how to write this part of the JO file, please refer to the [Sherpa manual](https://sherpa-team.gitlab.io/)
* All Sherpa parameters in this RunCard segment will overwrite the ones previously specified in the base fragment.
* The additional options like `ExtraFiles` or `NCores` are process-specific metadata used by the [`sherpaTarCreator`](../../../python/sherpaTarCreator/) to create input tarballs for central production.
* Note that the base fragments will automatically add event weights for hundreds of on-the-fly systematic variations. If you do not want those, you can switch them off by adding
  ```yaml
  SCALE_VARIATIONS: None
  PDF_VARIATIONS: None
  ```
