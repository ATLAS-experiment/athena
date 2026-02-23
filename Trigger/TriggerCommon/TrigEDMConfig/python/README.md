# TriggerEDM

The Trigger EDM (Event Data Model) defines our interface to trigger reconstructed content that is written out to RAW files, RDOTrig files, and data or MC AODs.

Trigger EDM content is defined seperately for each LHC Run in the `TrigEDMConfig` package:
```
Trigger/TriggerCommon/TrigEDMConfig/python
|-- TriggerEDMRun1.py
|-- TriggerEDMRun2.py
|-- TriggerEDMRun3.py
|-- TriggerEDMRun4.py
```
Each `TriggerEDMRun{X}.py` contains a list of _EDM entries_.

Common functions are defined in `TriggerEDM.py`, see [TriggerEDM handling](#triggeredm-handling).

## TriggerEDM entries

Our EDM mostly consists of `xAOD` objects. Each EDM entry is a tuple containing all EDM details for a specific xAOD interface or aux store container. An example of two (interface + aux) EDM entries:
```python
('xAOD::CaloClusterContainer#HLT_CaloEMClusters_Electron',               'BS ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionCaloElectronViews')]),
('xAOD::CaloClusterTrigAuxContainer#HLT_CaloEMClusters_ElectronAux.',    'BS ESD AODFULL AODSLIM', 'Egamma'),

('xAOD::CaloClusterContainer#HLT_TopoCaloClustersLC',                             'BS ESD AODFULL', 'Tau', [InViews('tauCaloMVAViews'), allowTruncation]),   
('xAOD::CaloClusterTrigAuxContainer#HLT_TopoCaloClustersLCAux.nCells.CENTER_MAG', 'BS ESD AODFULL', 'Tau', [allowTruncation]),

```
Every EDM entry will _at minimum_ define the container type and container name, the "EDM targets" and signature category. Additional dynamic Aux variables
to be recorded are specified as part of the Aux container name string.

"EDM targets" define the output type to which the containers will be recorded in the case of `RAW`, `RDO_TRIG`, `ESD` and `AOD` files. The trigger signature is used to categorise the total EDM size per signature for monitoring purposes.

### EDM targets

Every Run has a different target labelling system and naming scheme.
To nevertheless give an example, Run 3 EDM targets use the following EDM targets and mapping:

| Label | Output | Purpose |
|-------------|-------------|-------------|
|`BS`| ByteStream - Any content that is to make it out of Point 1 and be recorded to RAW file for a full event stream (PhysicsMain etc.). | Required for recording to data AOD after or kept in BS for debugging in case of data taking issues.|
|`ESD`| ESD files, RDO_TRIG files | Low-level reconstructed information such as HLT calorimeter clusters for specialised reconstruction studies. Also used for `RDO_TRIG` (MC equivalent of a RAW file) output.|
|`AODFULL`| data AOD at Tier0 | Required for regular trigger efficiency studies in data or validation during commissioning.|
|`AODSLIM`| bulk MC production AODs for physics analysis | Required for physics analysis. Examples: Final precision HLT leptons/b-jets needed for trigger matching and trigger scale factors.|
|`PhysicsTLA`/`EgammaPEBTLA`/`DarkJetPEBTLA`/`FTagPEBTLA`| respective RAW output of a Trigger-Level Analysis (TLA) or TLA Partial Event Building stream| Content required for a trigger-level physics analysis or TLA with partial event building (PEB) information.|

The following flags will set the target labels for AOD and ESD production respectively (with example):
```
flags.Trigger.AODEDMSet = "AODSLIM"
flags.Trigger.ESDEDMSet = "ESD"
```

### Optional EDM details

There are several additional EDM details that can be specified as a list constituting the last entry in the tuple.
These are defined in [`TrigEDMConfig.TriggerEDMDefs`](TriggerEDMDefs.py).

#### InViews

In the case of reconstruction in a limited region of the detector, this names the event view in which this collection is reconstructed, so that these collections are properly merged in the [`HLTEDMCreator`](../../../TrigSteer/TrigOutputHandling/src/HLTEDMCreator.h), and additional information allowing the items to be accessed in event views is recorded. For more details, see [the atlas software docs on Event Views](https://atlas-software.docs.cern.ch/athena/trigger/developers/eventviews/).

#### allowTruncation

Only valid for data taking. It is used to indicate that this HLT container can be dropped from an event during serialisation of the HLT content if the summed HLT result size supersedes the "truncation" threshold, which would send the event to the debug stream. For technical reasons, "allowTruncation" entries need to appear at the end of the EDM list.

#### Aliases

This EDM detail object marks containers that need a particular treatment by the [`HLTEDMCreator`](../../../TrigSteer/TrigOutputHandling/src/HLTEDMCreator.h).
It is used in the case of ShallowCopy containers: An alias with suffix `ShallowCopy` is set for all `ShallowAuxContainer` type EDM entries so that the [`HLTEDMCreator`](../../../TrigSteer/TrigOutputHandling/src/HLTEDMCreator.h) knows to treat them as ShallowCopy collections.

## TriggerEDM handling

Common functions for EDM handling are defined in `TriggerEDM.py`. Particularly useful to note are:

```py
recordable( arg, runVersion=3 )
```
-> A handy function used in signature configuration code to ensure that an algorithm
output collection name is properly formatted and has a corresponding entry in the
corresponding `TriggerEDM` file (the latter is only verified for Run 3 input currently).


```py
getRawTriggerEDMList(flags, runVersion=-1)
```
-> Retrieves a copy of the Trigger EDM content for the given LHC Run (`runVersion`). If `runVersion=-1` then the Run version is inferred from the input flags.

```py
testEDMList(edm_list, error_on_edmdetails = True)
```
-> Checks the validity and ordering of a list of EDM entries, such as
- EDM entry length.
- Container name string format
- Interface container followed by matching Aux. container.
- Valid EDM targets
- Duplicates
- `AllowTruncation` entries appear at end of list.

Used in EDM unit testing and when adding new EDM via
`Trigger.ExtraEDMList` flag on the fly.

```
flags.Trigger.ExtraEDMList -> _addExtraCollectionsToEDMList(edmList, extraList)
```
-> The `Trigger.ExtraEDMList` flag is used to add or update "user input" Trigger EDM entries during configuration.
It triggers a call to `_addExtraCollectionsToEDMList`, which adds the new EDM entries if not
already found in the TriggerEDM or updates the dynamic variables or EDM targets of existing
EDM entries.
The flag must provide a list of _complete_ EDM entries (both interface and Aux container), for example:
```py
flags.Trigger.ExtraEDMList = [
    ('xAOD::ElectronContainer#HLT_NewParticles',        'BS ESD AODFULL', 'EGamma', [InViews('NewViews')]),
    ('xAOD::ElectronAuxContainer#HLT_NewParticlesAux.', 'BS ESD AODFULL', 'EGamma'),
]
```

## unit tests

Unit tests check the validity of EDM entries and their ordering as well as the functionality
of adding extra EDM during configuration.
