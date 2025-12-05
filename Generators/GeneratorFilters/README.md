# Instruction for using the harmonised MC filters in release 25

## The aim of the harmonisation.

Since filtering and operations on generator level objects are done both during events generation and the truth derivation production the filters on generation level are harmonised with the code used on the derivation level. It was ensured that the generator level filters make use of the derivation collections and tools for generator event filtering. This way we reduce code duplication and ensure that analysis level definitions of truth objects (like "prompt lepton") match those used during event generation.
For most of the harmonised filters the event selection process is done in two steps:

    1. Preparing reduced (slimmed) sample of events containing given objects (eg. truth electron)
    2. Running filters


## Harmonised filters

Our MC productions will now use a new MC filter format. In this setup the xAOD truth and HepMC truth formats are harmonised at the event generation step.
You can find a list of the harmonised filters [on this Twiki page](https:// twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/GeneratorFilterValidation). The harmonised versions of the filters can be identified by "xAOD" added at the beginning of the filter name. It is checked that the efficiencies of harmonised filters are the same as of original i.e. unharmonised ones. It is recommended to use the harmonised filters for official production. Unharmonised filters may be used for tests, as the generation framework still accepts the unharmonised filters too. All most commonly used filters (in particular all used for mc16 production) are already harmonised, however if you are interested in using one of the filters that are still unharmonised please [contact Valentina](mailto:Valentina.Tudorache@cern.ch).

## Usage in the event generation JOs

For standard usage in JOs there is no significant difference to the format used in release 21. For each filter, a common fragment is provided and should be used in production. You can find [an example JO here](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/421xxx/421431/mc.PhPy8EG_AZNLOCTEQ6L1_Wplusenu_xAODElecFilt.py).

For your JOs, include the common fragment for the filter. The fragment defines the filter and adds it to the appropriate place in the filtering sequence for you. You can then modify values for any cuts as needed. For example:

``` 
include('GeneratorFilters/xAODElectronFilter_Common.py')

filtSeq.xAODElectronFilter.Ptcut = 25000
```

One can find all free parameters of a filter (settable cuts) and their predefined default values [in the filter headers](https://gitlab.cern.ch/atlas/athena/-/tree/main/Generators/GeneratorFilters/GeneratorFilters).

## Release for running the harmonised filters

The harmonised filters can be used in release 22 and newer releases. It is always recommended to use the newest release, however starting from `22.6.11,AthGeneration` all filters [listed in the table](https:// twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/GeneratorFilterValidation) are available in the harmonised form.
