HLT Menu configuration conventions
=====

Folder contents
-----

This folder contains the HLT menu definitions and menu-dependent configuration.
All HLT menus depend on a specific L1 menu.
See [L1/Menu/MenuMapping.py](../../L1/Menu/MenuMapping.py) for the L1 menus corresponding to each HLT menu.

Standard high-mu proton-proton collision menu:
- `Physics_pp_run3_v1.py`: Physics triggers (run at P1 and in MC)
  - This menu is never run on its own in production.

Operations menu:
- `PhysicsP1_pp_run3_v1.py`: Triggers for P1 operation (monitoring etc), not simulated
  - This is combined with the physics chains when collecting data
  - No triggers in this file should stream to the `Main` stream

Standard high-mu proton-proton menus for simulation:
- `MC_pp_run3_v1.py`: Triggers for MC simulation only
- `Dev_pp_run3_v1.py`: Triggers in development and being validated
  - `MC_pp` includes `Physics_pp`
  - `Dev_pp` includes `MC_pp`
  - *N.B both MC and Dev exclude certain triggers in Physics_pp whose L1 triggers are omitted fom the MC L1 menu.*

Specialised proton-proton menus:
- `PhysicsP1_pp_run3_v1.py`: Triggers for low-mu collisions

Heavy ion menus:
- `PhysicsP1_HI_run3_v1.py`: Physics triggers (run at P1 and in MC)
- `Dev_HI_run3_v1.py`: Triggers in development and being validated
  - `Dev_HI` includes `PhysicsP1_HI`

Rules for adding triggers
-----

Trigger and signature developers may add triggers to the appropriate `Dev` menus as needed for their development work, in consultation with the [Trigger Menu Coordinators](https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerOrganisation). The `MC` and `Physics` menus are only to be modified in assigned tasks by menu experts.

A trigger is specified by a `ChainProp` object, with the following attributes:
- `name` (required): This identifies the trigger, but also encodes the HLT algorithm configuration. Must be of the form 'HLT_[hltinfo]_L1[l1name]'.
  - Further information about the HLT naming scheme is at [TriggerNamingRun3](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TriggerNamingRun3)
  - The decoding of the [hltinfo] part is done by [HLT/Config/Utility/DictFromChainName.py](../../HLT/Config/Utility/DictFromChainName.py) with reference to [HLT/Menu/SignatureDicts.py](SignatureDicts.py)
- `l1SeedThresholds`: The L1 thresholds used to seed RoI-based HLT reconstruction. One value per leg of the trigger.
  - For full-scan legs, the value should be 'FSNOSEED'. This is also used for (noalg) streamers.
  - If not specified, the menu will try to auto-configure from the L1 trigger. Any topological L1 triggers cannot be interpreted reliably, so require an explicit specification.
- `stream`: The target data stream to which events are streamed. Default is a list with one element: ['Main']. Each trigger can only send events to one stream, the exception being the 'express' stream, which can be combined with one other stream for monitoring purposes.
- `groups`: Labels applied for menu validation and trigger categorisation. An inexhaustive list of examples is:
  - `Primary/Support/EOF`: Indicates prescale strategy for the trigger. Primary triggers are always unprescaled, and are the main drivers of analyses. Support triggers may be prescaled. End-of-Fill triggers are activated or have their prescales reduced later in the run when luminosity has decayed.
    - For Run 3 commissioning, the Primary/Support/EOF labels are also combined with information about the L1 systems used: `Legacy` or `PhaseI`may be applied for L1Calo, or `L1Muon`
  - Also for commissioning, we indicate the L1Topo module (`Topo2`, `Topo` or `LegacyTopo`) used to run the algorithms for topological L1 trigger selections.
  - The signature is indicated by a combination of `RATE` and `BW` groups, which are primarily used for studies of the rate and bandwidth distribution between different sets of triggers.
    - A special case of `RATE` prescales is to define Coherent Prescale Sets. These group together HLT items sharing the same L1 input, and ensures that the HLT items with higher prescales will activate on a subset of the events on which lower prescale items are activated, rather than the prescales being computed without correlations. The naming scheme is `RATE:CPS_[l1name]` corresponding to `L1_[l1name]`.
      - E.g. `RATE:CPS_J100` can be applied to triggers seeded from `L1_J100`.
      - CPS groups should not be used for primary triggers, and will be checked to ensure they contain at least 2 triggers. Defining CPS for a single trigger will break the menu generation.
      - As far as possible, all support triggers should have a CPS group defined.
  - `PS` groups are used to situationally predefine prescales to implement Menu Prescale Sets, defined in [HLT/Menu/MenuPrescaleConfig.py](MenuPrescaleConfig.py). Notably, the following veto labels take effect cumulatively:
    - `PS:NoRepro` deactivates the tagged triggers in the `HLTReprocessing_prescale` set and all following prescale sets
    - `PS:NoTrigVal` deactivates the tagged triggers in the `TriggerValidation_prescale` set and all following prescale sets
    - `PS:NoBulkMCProd` deactivates the tagged triggers in the `BulkMCProd_prescale` set
  - `monGroups`: Defines sets of triggers used to fill monitoring histograms. These include both a category of histogram (e.g. `t0`, `shifter`, `expert`) and the relevant signature (e.g. `egammaMon`, `idMon`).