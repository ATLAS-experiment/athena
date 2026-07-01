## Introduction

This page serves as general documentation for the derivation framework. It contains information on

* basic principles and important common components
* instructions for running the framework locally and on the Grid
* instructions for setting up a new derivation
* list of currently implemented tools

For details of DAOD production, including samples, please refer to [these pages](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationProductionTeam). For information on the content of DAOD_PHYS and PHYSLITE, please see [this page](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DAODPhys).

## What is the derivation framework for?

The derivation framework is a set of Athena tools, algorithms and python configuration scripts used to create analysis data formats called derived AODs (DAOD). DAODs are made either from the output of reconstruction (AOD) or event generator output (EVNT), and are written in the xAOD data structure. This means they can be read directly with the main analysis frameworks, and many of their variables can be read directly by ROOT. DAODs are the starting point for most analyses. They are always made by the central production team, so individual physicists should only need to run the framework themselves when making small scale tests. In common with all central production, the derivation framework uses the job transforms infrastructure to run. It can produce multiple output formats from a single input ("train production").

In run 2, each analysis had its own DAOD. In run 3 this has been rationalised such that most analyses should use either DAOD_PHYS or DAOD_PHYSLITE. Documentation on these formats can be found [here](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DAODPhys). Some special analyses (especially B-physics and long-lived particles) along with combined performance groups will need to continue to use individual DAODs. It is expected that most formats will continue to use the common physics content used in PHYS/PHYSLITE.

If you are content to use an existing DAOD format and don't need to define a new one, most of the information on this page will not be relevant to you.

DAODs are made from the AODs via four operations:

* skimming: removing whole events
* thinning: removing whole objects from within an event, but keeping the rest of the event
* slimming: removing information from within objects, but keeping the rest of the object
* augmentation: adding data not found in the input data

## Basic software

A given derivation is defined by a set of python scripts. These configure:

* a series of tools for skimming, thinning and augmenting the data
* a kernel algorithm (AthFilterAlg) to define the event loop, to apply the tools and make the skim decision
* an output stream for the derivation. This also controls the slimming of the data

The job is steered via the `Derivation_tf.py` job transformation.

### Software layout in Git

The derivation framework code is found in the location [PhysicsAnalysis/DerivationFramework](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework) and is laid out in several packages:
* [DerivationFrameworkCore](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkCore) &rarr; kernel algorithm, slimming machinery, some format list
* [DerivationFrameworkInterfaces](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkInterfaces) &rarr; definitions of tool interfaces
* [DerivationFrameworkTools](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkTools) &rarr; common tools
* [DerivationFrameworkConfiguration](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkConfiguration) &rarr; common configuration, job transformations machinery
* [DerivationFrameworkExamples](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples) &rarr; example implementations
* [DerivationFrameworkPhys](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys) &rarr; configuration for PHYS, PHYSLITE and common physics content
* DerivationFramework{X} &rarr; locations used by individual groups to define their own formats, and content lists defined by the combined performance groups.

### Software components: tools, kernels and streams

The derivations themselves are coded up entirely in the form of AlgTools. These tools may be provided as part of the common framework, by the CP groups or other physics groups, or by the physics group defining the derivation. There is no limit on the number of tools or their complexity; essentially the tool authors have complete freedom, aside of the fact that that must inherit from one of three interfaces:

* `ISkimmingTool`: interface for skimming. Tools inheriting from this must implement a method `bool eventPassesFilter()` which returns true or false depending on whether the event passes the criteria
* `IThinningTool`: interface for thinning. Tools inheriting from this must implement a method `void doThinning()` which contains the relevant call(s) to the thinning service
* `IAugmentationTool`: interface for augmentation. Tools inheriting from this must implement a method `void addBranches()` which contains the relevant record commands to StoreGate

It should be noted that in fact this division between skimming, thinning and augmentation is rather cosmetic, since a skimming tool could equally contain thinning and record commands inside it, which would be executed when the `eventPassesFilter()` method was called. It should also be noted that slimming is not included in the above list, since it is done directly by the streaming mechanism and so does not require a tool.

Each DAOD making job must create an instance of the Kernel algorithm. This is an Athena `AthAlgorithm` which takes lists of tools as arguments and then, one by one, calls the `addBranches()`, `doThinning()` and `eventPassesFilter()` methods. If all of the `eventPassesFilter()` calls return true, the `FilterReporter` is set to true, and then, provided the Kernel is registered as an `acceptAlg` with the output stream, the event will be written to disk. The `CutFlowSvc` is also called automatically due to the use of the `FilterReporter`. The Kernel only has AND logic (so if any one skimming tool returns false then the filter as a whole will return false). The reason for this is that OR is possible via the output stream, so if more complex logic is needed then one simply sets up more than one Kernel. This can be revised if necessary.

It is important to realise that an individual setting up a derivation never needs to modify the kernel - it is simply a shell for scheduling the tools in Athena. Everything about a specific derivation should be in the tools.

## Running the framework locally

To run a format that is already defined you need to do the following:

* Log into LXPLUS or a CVMFS-aware machine
* `setupATLAS`
* Go into a clean working directory
* To set up the latest nightly build: `asetup Athena,main,latest`
* To set up a defined release: `asetup Athena,25.0.51`
* Obtain an xAOD file and copy/link it to your working area

```bash
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
Derivation_tf.py --CA --inputAODFile input.AOD.pool.root --outputDAODFile output.pool.root --formats PHYS
```

`PHYS` can be replaced with any of the format names. In particular you can experiment with the test formats `TEST{1-6}` defined and described in the [DerivationFrameworkExamples](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples) package.

If the `setupATLAS` command doesn't work you may need to run the following lines:

```bash
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
alias setupATLAS='source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh'
```

## How to define DAOD formats

This next section is only relevant if you want to define a new DAOD format or if you are migrating run 2 style DAODs to run 3. If you need guidance on how to modify and test ATLAS software, please see [these instructions](https://atlassoftwaredocs.web.cern.ch/gittutorial/)

### Basic templates

When setting up a new format you should use the following examples as templates. It may be that this alone is sufficient for your purposes. If you need more detailed information, please see the detailed sections below.

* For the basics: [DerivationFrameworkExamples](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples)
* [TEST1.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST1.py): TEST1 - demonstration of skimming via a dedicated tool implemented in C++, plus smart slimming
* [TEST2.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST2.py): TEST2 - demonstration of skimming via the generic string parsing tool, plus smart slimming
* [TEST3.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST3.py): TEST3 - demonstration of thinning via a dedicated tool implemented in C++, plus smart slimming
* [TEST4.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST4.py): TEST4 - demonstration of smart slimming using the slimming helper, and how to include variables more generally
* [TEST5.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST5.py): TEST5 - illustration of object decoration, using an example tool and a CP (muon) tool
* [TEST6.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST6.py): TEST6 - illustration of scheduling CPU-heavy operations after a pre-selection skimming step to avoid running expensive calculations for events that will be rejected
* For inclusion of the common physics content: [DerivationFrameworkPhys](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys)
* [PHYS.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys/python/PHYS.py)

Please follow these guidelines when defining the format:
* All format must have a [definition in this file](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkConfiguration/python/DerivationConfigList.py), or it will not be located by the job transforms machinery. Follow the patterns established by the other formats, especially related to documentary comments.
* Use the `DerivationFrameworkX` package associated with physics/CP activity that the new format targets
* The main python script defining the format should have the same name as the format itself, e.g. the DAOD_PHYS script should be called PHYS.py.
* All python defining the new format must be in the `python` directory of the package. Don't use the `share` directory which is a feature of the run 2 legacy set-up.
* Follow strictly the conventions established in the existing scripts on how configuration methods are constructed and passed to the component accumulator. More information on that topic can be found on [these pages](https://atlassoftwaredocs.web.cern.ch/guides/ca_configuration/).

### Slimming

Slimming involves the removal either of variables or whole containers from the data. The general rule is: you should only store the variable that you actually need. It is done differently depending on whether the required collections are xAOD container types, or non-xAOD - note that an AOD file will usually contain some non-xAOD collections, for instance various collections of `Trk::Tracks` and the like. xAOD containers can be easily identified since they are all in the namespace `xAOD::`. The slimming of xAOD collections is also known as _smart slimming_, since generally more work is done automatically, behind the scenes, reducing the workload on the user.

All of the information given below is demonstrated in the slimming example [TEST4](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST4.py) as described above. When setting up your slimming you may wish to implement it first in this simple example so that you can test it without any additional complications.

You first need to set up an output stream and a slimming helper as follows:

```python
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
TEST4SlimmingHelper = SlimmingHelper("TEST4SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections)
```

You are now ready to set up the slimming options.

#### xAOD variables (smart slimming)

You should only add variables that you need. It can be difficult to figure out exactly what variables you actually need if you are using a variety of tools or a complicated framework, so to simplify things a set of pre-set slimming lists are available. If you know that your analysis needs muons, you can add `Muons` as a `SmartCollection`. This will automatically include all of the variables needed by the tools written by the muon combined performance group, whether they are actually muon variables or not. If you add `Muons` as a `SmartCollection` you are guaranteed that all of the muon analysis tools will work on your format.

```python
TEST4SlimmingHelper.SmartCollections = ["EventInfo",
                                        "Electrons",
                                        "Photons",
                                        "Muons",
                                        "PrimaryVertices",
                                        "InDetTrackParticles",
                                        "AntiKt4EMTopoJets",
                                        "AntiKt4EMPFlowJets",
                                        "BTagging_AntiKt4EMPFlow",
                                        "BTagging_AntiKtVR30Rmax4Rmin02Track", 
                                        "MET_Baseline_AntiKt4EMTopo",
                                        "MET_Baseline_AntiKt4EMPFlow",
                                        "TauJets",
                                        "DiTauJets",
                                        "DiTauJetsLowPt",
                                        "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                        "AntiKtVR30Rmax4Rmin02PV0TrackJets"]
```
* If you find you need additional variables, beyond those provided in the `SmartCollections`, you can add extra variables individually, in the following way (note that both of the examples below are included in the `SmartCollections` - they are used for illustrative purposes only). Note also that for this option you can include any xAOD container - not just those listed above (so in the event that your container doesn't have a pre-set list, this is a way of including the relevant variables).

```python
TEST4SlimmingHelper.ExtraVariables = ["PhotonCollection.weta2.f1.phi.weta1.emaxs1","Muons.momentumBalanceSignificance"]
```

* To add all variables for a given collection you need to do the following - again note that for this option you can include any xAOD container.

```python
TEST4SlimmingHelper.AllVariables = ["Muons"]
```

The lists themselves are found in the individual group packages, so for example, the muons pre-set list is here: [MuonsCPContent.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkMuons/python/MuonsCPContent.py). All of these pre-set lists end with `CPContent`, and are defined for all of the reconstruction domains.

After all settings have been determined you need to tell the SlimmingHelper to append the relevant contents to the output stream. This is done as follows:

```python
TEST4ItemList = TEST4SlimmingHelper.GetItemList()

acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_TEST4", ItemList=TEST4ItemList, AcceptAlgs=["TEST4Kernel"]))

```

Note that you must *not* include the following items in your slimming list, since they are added automatically:
* Trigger decision, trigger navigation (see separate section on trigger content)
* Metadata

Also in all the examples above note that all that is needed is the *collection name*, e.g. `Muons`, `Electrons`. The `Aux` and `Dyn` that you see in the ROOT browser *MUST NOT* be used.

#### Trigger objects

Trigger objects are handled as usual by the slimming helper, but is not done via the usual smart slimming interface - instead a menu of options, allowing the user to pick which trigger slices to include - is provided. The slices are included or removed by True/False flags (the default is False so leaving a menu item out of the job options will also lead to the relevant containers being omitted). Most analyses do not need trigger objects, but can instead rely on matching information to offline objects. See the common physics section below for more details on this.

```python
SlimmingHelper.IncludeMuonTriggerContent = True
SlimmingHelper.IncludeEGammaTriggerContent = True
SlimmingHelper.IncludeBPhysTriggerContent = True
#JetTauEtMissTriggerContent: split in 4 slices
SlimmingHelper.IncludeJetTriggerContent = True
SlimmingHelper.IncludeEtMissTriggerContent = True
SlimmingHelper.IncludeTauTriggerContent = True
SlimmingHelper.IncludeBJetTriggerContent = True
SlimmingHelper.IncludeMinBiasTriggerContent = True
```

Setting these items to True causes the relevant containers (of which there are often several per slice) to be included, and it also slims them, keeping only the variables needed by the trigger analysis tools.

#### On-the-fly xAOD containers
xAOD containers that do not exist in the input file, but are made on-the-fly as part of the job, can still be slimmed, but it is necessary to notify the slimming helper of the name and type of each of these containers in advance, since in these cases it does not have the opportunity to get this information from the input file. To do this you simply need to add the following:

```python
TEST4SlimmingHelper.AppendToDictionary.update({'ContainerName': 'xAOD::ContainerType'})
```
e.g.
```python
TEST4SlimmingHelper.AppendToDictionary.update({'SomeNewJetCollectionName': 'xAOD::JetContainer','SomeNewJetAuxCollectionName': 'xAOD::JetAuxContainer', etc})
```

Then proceed as described above - with the caveat that (of course) there are no smart slimming lists defined for such containers, so you must either include `AllVariables`, or pick which ones you want manually via `ExtraVariables`. You can see this demonstrated in [the PHYS format](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys/python/PHYS.py)

Note that it is important to use `update` to avoid over-writing entries already added elsewhere.

#### Non-xAOD containers

Non-xAOD containers cannot be slimmed variable-by-variable, and so you must include either the whole container, or not at all. This is done by using the `StaticContent` method of the slimming helper, e.g.:

```python
TEST4SlimmingHelper.StaticContent = ["HLT::HLTResult#HLTResult_HLT", "HLT::HLTResult#HLTResult_L2", "HLT::HLTResult#HLTResult_EF"]
```

Note that to avoid problems with clashes between expanded and unexpanded collections (see red warning below) this method forbids:
* use of wildcards - please list all collection names explicitly
* xAOD collections which are included in the normal slimming mechanism described above - this is basically all xAOD containers that exist in the xAOD files as output by reconstruction. These should be included via the method above - if you do not want to slim at all, please use the `AllVariables` option

*Important*: do not use the `AddItems` method to attach content directly to the output stream for xAOD collections, even if you want to include the full content and aren't bothered about about smart slimming. If you do, it will corrupt the output data. 

### Common physics content for Run 3

The _common physics content_ is a set of reconstructed object containers and variables which is built from the AOD and written into DAOD_PHYS alongside those variables that are copied directly from the AOD. It includes analysis-level jets, MET, flavour tagging, a number of decorations on AOD-level reconstructed objects, as well as a common truth record. Part of it is also written into the PHYSLITE format. Groups managing formats other than PHYS or PHYSLITE should in general add the common physics content to these formats as well, unless there is a very good reason for not doing so. If you need to deviate from the common physics content, you should contact the relevant combined performance group for advice on how to correctly configure the revisions you need.

Since the common physics content defines DAOD_PHYS, a full description of the actual content can be found [here](DAODPhys). This section describes the technical steps needed to include the common physics content in a format.

The following needs to go into the config method for the derivation framework kernel:

```python
def NAMEKernelCfg(ConfigFlags, name='NAMEKernel', **kwargs):
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(ConfigFlags, TriggerListsHelper = kwargs['TriggerListsHelper']))
    ...
    # Other configuration
    ...
    return acc
```

Then, in the main config method for the format itself:

```python
def NAMECfg(ConfigFlags):
    acc = ComponentAccumulator()
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    NAMETriggerListsHelper = TriggerListsHelper()
    acc.merge(NAMEKernelCfg(ConfigFlags, name="NAMEKernel", StreamName = 'StreamDAOD_NAME', TriggerListsHelper = NAMETriggerListsHelper))
    ...
    # Other configuration and slimming settings
    ...
    return acc
```

In the above, it is assumed that NAME is replaced with the name of your format. Finally, since the common physics content does not exist in the input file, a full list of the variables to be stored and their types must be passed to the slimming helper, as decribed in the slimming section above. You should copy exactly the slimming set-up in the [PHYS](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys/python/PHYS.py) format, unless there are elements you know you definitely don't need, in which case these can be omitted.

#### Common trigger content

The trigger content is complex and should be handled carefully. Deviation from the standard recipe provided by the trigger group should only be done in consultation with them - otherwise, use the following recipe.

The standard trigger content in DAOD consists of the trigger _decision_ and trigger _matching_ information (run 2) or trigger _navigation_ information (run 3). Decision indicates which triggers were fired by a given event, and is accessed via the trigger decision tool. Matching information is constituted of decorations on offline reconstructed objects, indicating whether that particular object was responsible for firing a trigger. Navigation information allows the user to do the matching themselves at the analysis level. Trigger _objects_ themselves are not usually stored in DAODs but can be added as described in the [section above](#trigger-objects). Trigger objects should only be added for specific reasons after consultation with the trigger group. 

The template for including common trigger information in a DAOD can be found in the PHYS format. The relevant lines should go in the top level config method for the format (`PHYSCfg` in the case of PHYS). In the following it is assumed that the PHYS string will be replaced by the name of your format. First you need to create a `TriggerListsHelper` which build the list of triggers for which the matching is supported:

```python
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
PHYSTriggerListsHelper = TriggerListsHelper()
```

This should then be passed to the method that configures the derivation framework main kernel, `PHYSKernelCfg` in the case of PHYS:

```python
acc.merge(PHYSKernelCfg(ConfigFlags, name="PHYSKernel", StreamName = 'StreamDAOD_PHYS', TriggerListsHelper = PHYSTriggerListsHelper))
```

Then, at the end of the slimming part of the top level config method, you should add these lines (adjusting for the name of your format):

```python
# Trigger matching
# Run 2
if ConfigFlags.Trigger.EDMVersion == 2:

    from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
    AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper, 
                                     OutputContainerPrefix = "TrigMatch_", 
                                     TriggerList = PHYSTriggerListsHelper.Run2TriggerNamesTau)

    AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper, 
                                     OutputContainerPrefix = "TrigMatch_",
                                     TriggerList = PHYSTriggerListsHelper.Run2TriggerNamesNoTau)
# Run 3

if ConfigFlags.Trigger.EDMVersion == 3:

    from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
    AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(PHYSSlimmingHelper)        

```

You can also explicitly disable the trigger object inclusion as is done in PHYS for documentary purposes, but the default is `False` so this isn't strictly necessary.

### Skimming

Event removal, or skimming, can be done in two ways:
* with a dedicated skimming tool implemented in C++, demonstrated in the [TEST1](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST1.py) format
* with the string parser tool, demonstrated in the [TEST2](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST2.py) format

The former will normally be used for complicated skimming whereas the latter is used for simpler selections.

#### Dedicated skimming tool

Setting up a dedicated skimming tool is demonstrated in [TEST1](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST1.py) - follow this pattern exactly for your own case. The actual C++ tool used in this example can be found [here](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/src/SkimmingToolExample.cxx). If you need to implement such a tool, again, follow this pattern. The code should be located in the same package as the format which it is targeting.

#### Generic skimming tool

It is not desirable to have to implement a separate C++ tool for each and every possible event selection, since the number of tools will proliferate. To avoid this a generic skimming tool (inheriting from ISkimmingTool) has been developed to enable cuts to be written as human-readable strings, e.g.:

```python
count(abs(ElectronCollection.eta) < 1.0) > 0 && count(Muons.pt > (20 * GeV)) >= 1
```

As can be seen above, to access quantities via strings the syntax `ContainerName.MethodName` should be used, where
* `ContainerName` is the name of the object container in StoreGate, e.g. `Muons`, `ElectronCollection` as seen above
* `MethodName` is the name of the object's C++ method which returns this quantity, e.g. `eta`, `pt` as seen above (one omits the `()` in the string parsing)

Triggers are accessed in exactly the same way, e.g.

```python
EF_mu24i_tight && count(abs(ElectronCollection.eta) < 1.0) > 0 && count(Muons.pt > (20 * GeV)) >= 1
```

The tool currently has some limitations. It can process any quantity that:
* is a unique single value for a given object (e.g. electron pT is OK because there is only one value of pT per electron)
* can be accessed from the xAOD object directly by a simple method returning the single value - e.g. pt(), eta() etc
* is a decoration added to the objects
* is a quantity in the aux store (even those accessed through helper functions)
* MET map-based accessors

Details on the correct syntax can be found in these slides:
* [Simple variables](https://indico.cern.ch/getFile.py/access?contribId`4&resId`0&materialId`slides&confId`273466)
* [Complex variables](https://indico.cern.ch/event/333980/contribution/3/material/slides/0.pdf)

However, the tool _cannot_ handle:
* access to specific elements of vectors
* vectors-of-vectors
* arbitrary function calls and multiple layers of accessors, e.g. `myObj-&gt;someFunction().someOtherFunction().someValue()`
* Any data types that can’t meaningfully be cast to int, double or vectors thereof
* ElementLinks between xAOD objects

Dedicated C++ should be written if such things are needed in a selection (and this dedicated C++ can then be daisy-chained with text-based selections as described below). For all selections that can be implemented in strings, it is *strongly recommended* that this tool is used to avoid a proliferation of trivial C++ selection tools.

You can see how the tool can be used in the [TEST2](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST2.py) example. Follow this pattern exactly for your own case. If you are setting up a DAOD without the common augmentations, pay attention to the note in this example about pre-loading the names and types of the containers to the sequence that runs the job.

Whist there is no practical limit to how complex the string-based selections can be in the generic tool, beyond a certain level of complexity such strings become very difficult for a human being to read. In such a case a dedicated C++ skimming tool should be implemented as shown above.

#### Combining the results of several skimming tools

Sometimes the selections may be so complicated that it is more convenient to express selections in more than one skimming tool. As stated above if the are all inserted into the kernel, a grand AND will be performed (meaning that each tool must pass for the event to pass). To do OR-ed combinations or more complicated logical operations without having to create more than one kernel, you can use some special combiner tools called `FilterCombinationOR` and `FilterCombinationAND`, which are themselves ISkimmingTools. This would look something like:

```python
mySkimTool1 = acc.getPrimaryAndMerge(MyTool1Cfg(flags,**kwargs))
mySkimTool2 = acc.getPrimaryAndMerge(MyTool2Cfg(flags,**kwargs))
acc.addPublicTool(CompFactory.DerivationFramework.FilterCombinationOR(name="myLogicalCombination", FilterList=[mySkimTool1,mySkimTool2]),primary = True))
return acc
```

This `FilterCombinationOR` would then be added to the DerivationKernel as a skimming tool. Similar operations for AND may be performed with an equivalent tool `FilterCombinationAND`. One can build up arbitrarily complex logical selections in this way.


#### Applying prescales

A tool is provided to accept every n-th event, where n is a user-defined quantity. This tool can either be passed directly to the kernel to effect a general prescale, or it can be AND-ed with other tools to rescale part of the selection. Note that only integer prescales are permitted. The usage is as follows:

```python
acc.addPublicTool(CompFactory.DerivationFramework.PrescaleTool(name = "TEST1SkimmingTool", Prescale = 10), primary = True)
return acc
```

#### Skimming on triggers with dots and dashes in their names

In Run 2 and 3, some trigger chain names contain symbols that cannot be handled by the text parser. Dots and dashes are particularly problematic, since they are interpreted by the parser as the accessor and subtraction operations respectively, rendering the trigger names incomprehensible. For these cases it is necessary to use a dedicated tool that interfaces directly to the trigger decision tool. The example below shows how it should be used.

```python
acc.addPublicTool(CompFactory.DerivationFramework.TriggerSkimmingTool(name = "TEST1SkimmingTool", TriggerListAND = ["HLT_j360_a10_nojcalib_L1HT150-J20.ETA30"] ), primary = True)
return acc
```

You can add as many triggers as you like to the list, and as the name suggests it will do an AND (e.g. require that they all fire before accepting the event). To achieve an OR, just use TriggerListOR instead. The tool should then be added to the SkimmingTools along with whatever else you are selecting (or combined via the tools mentioned immediately above).

#### Command line skimming

The framework has a special format `SKIM`, which allows skimming of PHYS and PHYSLITE input to be done on the command line, rather than having to define a new format with its own config file. The usage is as follows:

```bash
Derivation_tf.py --CA --inputDAOD_PHYSLITEFile DAOD_PHYSLITE.pool.root --outputD2AODFile output.pool.root --formats SKIM --skimmingExpression "count(AnalysisMuons.pt > (1 * GeV)) >= 1" --skimmingContainers xAOD::MuonContainer/AnalysisMuons
```

The skimming expression, which must be enclosed in quote marks, uses exactly the same synatx as the other DAOD formats (e.g. it is processe by the same [ExpressionEvaluation](https://indico.cern.ch/getFile.py/access?contribId`4&resId`0&materialId`slides&confId`273466) tool). This means that, should a user wish to apply the same skimming to PHYS/PHYSLITE as was applied to their old DAOD, they can use the same selection string, as long as they change the container names in the case of PHYSLITE.

The last argument is a list of all of the containers that are referenced by the skimming expression, in the form `xAOD::ContainerType/ContainerName`, e.g. `xAOD::MuonContainer/AnalysisMuons` in the example above. If more than one container is referenced each one should be listed (space separated).

No slimming is applied, e.g. the per-event contents are identical to the input. Note that the output is D2AOD - this is because the input is DAOD.

This is not intended to be used in official DAOD production, because the format name (`D2AOD_SKIM`) is fixed and different skims would have indistinguishable dataset names. Should official production be needed of a PHYS/PHYSLITE skim, the format should be defined with its own config fragment and given its own format name. However, this might be the basis of a future user skimming service, should ATLAS decide to have one.

#### How to disable skimming (pass-through mode)
It is possible to disable the skimming for a given job which may be needed to prepare certain special MC or zero-bias samples. In order to do this set the 'passThrough' flag in Derivation_tf:

```bash
Derivation_tf.py ... --passThrough True
```

When set, this causes the skimming tools to be removed from all of the derivation kernels just before the final configuration is set, leading to all events being accepted. Note that this may lead to very large output samples so it should be used only when absolutely needed. Unskimmed formats such as PHYS, PHYSLITE and the TRUTH formats do not require this option to be set since there are no skimming tools in use in those cases.

### Object decoration

Tools can "decorate" (add information to) the objects as the derivation framework is running, and then user code can make use of these decorations to select events, or store them in the output format, or both. An example of this can be found in [TEST5](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST5.py). Two decorations are made:
* on muons, using a dedicated [example C++ tool](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/src/AugmentationToolExample.cxx)
* on inner detector tracks, using a combined performance tool for selecting inner detector tracks, which is wrapped in a [tool for turning the decision into a decoration](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkTools/src/AsgSelectionToolWrapper.cxx)
In both cases the decorations are categorical variables, but could just as well be float quantities. In the example, the variables are stored via the smart slimming, but could also be passed to a string-based skimming or thinning tool to allow event or object selection on the basis of these decorations. An example of this would be:

```python
expression = 'count((Muons.TEST5GoodMuons) && (Muons.pt > 15*GeV)) >= 2'
```

There are other tools operating in this way, some of which are documented elsewhere in this page. If you need to write your own decoration tool, use the examples above as templates.

### Adding simple data types

To add simple types (floats, ints, std::vectors, etc) to each event without association to any object, one should provide C++ of [this form](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/src/AugmentationToolExample.cxx#L78-84) (in this example a `std::vector<float>` is written for each event). On the Python side one must use the `StaticContent` method of the slimming helper, as follows (again this is for the specific example given in the code snippet):

```python
SlimmingHelper.StaticContent.append("std::vector<float>#DFAugmentationExample")
```


### Including preselections

It is possible that your derivation includes augmented information or event selections that involve the use of CPU-intensive calculations, that should only be run once an event has passed certain pre-selections. If you only use a single kernel this will not be possible, since the kernel will run the tools for every event. To avoid this problem, you should set up a pre-selection kernel, which runs before the main kernel. Events that do not pass the pre-selection will never reach the main kernel and the CPU-heavy tools will therefore be shielded from having to run over every event. The pre-selection veto is achieved by means of a user-defined sequence, into which both kernels are inserted. This is demonstrated in the [TEST6 example](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST6.py).

The main difference from the usual set-up is the necessity to declare a separate kernel into which the CPU-heavy tasks are added, and a new sequence. Both can be defined in the same config fragment, which can be the same as the one configuring the main kernel (as shown here and in TEST6), or can be separate.

```python
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND
...
...

def TEST6KernelCfg(flags, name='TEST6Kernel', **kwargs):
    acc = ComponentAccumulator()
    # Subsequence
    acc.addSequence( seqAND("TEST6Sequence") )
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    skimmingTool = acc.getPrimaryAndMerge(TEST6SkimmingToolCfg(flags))
    skimmingKernel = DerivationKernel(kwargs["PreselectionName"], SkimmingTools = [skimmingTool])

    # Add skimming tool to subsequence
    acc.addEventAlgo( skimmingKernel, sequenceName="TEST6Sequence" ) 

    # Add CPU heavy augmentation tool to same subsequence
    augmentationTool = acc.getPrimaryAndMerge(TEST6AugmentationToolCfg(flags))

    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = [augmentationTool]), sequenceName="TEST6Sequence")

    return(acc)
```

Since this is done in a single config method the merging into the main component accumulator is done in one step, e.g.

```python
acc = ComponentAccumulator()
acc.merge(TEST6KernelCfg(ConfigFlags, name="TEST6Kernel", PreselectionName="TEST6PreselectionKernel"))
```

If you choose to use separate methods you should merge the pre-selection kernel first.

Note the name of the sequencer: `seqAND`, which immediately suggests the existence of a `seqOR`. This is indeed the case, but since it allows the sequence to pass if any member is true, it is unlikely to be of use in derivation workflows. However, if it should prove to be useful, it would be set up in exactly the same way.

### Thinning

_Thinning_ is the operation where whole objects are removed from the event, according to some criteria. For instance one might choose to throw away all tracks that are not associated with a reconstructed muon or which have a pT of less than 10 GeV, etc. The underlying machinery removes the objects and re-organises any links to the remaining objects to make sure that associations between objects aren't broken. Thinning must be implemented in a tool which implements `IThinningTool`. In these tools, at the C++ level, the container(s) to be thinned are looped over and a mask (a vector of bools) is constructed corresponding to which objects should be kept (`true`) and which should be discarded (`false`). This mask is attached to the container using the method `container.keep(mask);`. On the python side, the tool is added to the kernel as a `ThinningTool`.

The basics are demonstrated in [TEST3](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST3.py). The accompanying C++ tool for this example is called [ThinningToolExample](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/src/ThinningToolExample.cxx). The main business, the construction of the mask, is done in the `doThinning` which as described in the introduction is the common method for all IThinningTools:

```c++
StatusCode DerivationFramework::ThinningToolExample::doThinning() const
{
const EventContext& ctx ` Gaudi::Hive::currentContext();

// Get the track container
SG::ThinningHandle<xAOD::TrackParticleContainer> tracks (m_inDetSGKey, ctx);
m_ntot+`tracks->size();
// Loop over tracks, see if they pass, set mask
std::vector<bool> mask;
for (xAOD::TrackParticleContainer::const_iterator trackIt`tracks->begin(); trackIt!`tracks->end(); ++trackIt) {
if ( (*trackIt)->pt() > m_trackPtCut ) {++m_npass; mask.push_back(true);}
else { mask.push_back(false); }
}
// This is the important line - attaching the mask to the container. Everything after this is done automatically.
tracks.keep (mask);

return StatusCode::SUCCESS;
}
```

On the python side, the configuration of the thinning tools is very similar to skimming, except that the stream name must be passed to the tool. See [TEST3.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkExamples/python/TEST3.py) for a clear example of this. In the case where several thinning tools are run together, a grand OR is applied, such that if an object is retained by any one tool, that object will be retained, irrespective of how many other tools reject it.

#### Defined thinning tools

Similarly to skimming, before launching into writing your own thinning tools, you should ascertain whether one of the existing thinning tools can be used. Again, the `ExpressionEvaluation` tool, which can also select objects as well as events, is extremely useful in avoiding having to write dozens of C++ tools. The available thinning tools are listed below.

---++++ Tools for thinning TrackParticles

This is the most common thinning operation due to the large number and size of TrackParticle objects. There are six tools for thinning track particles, all of which are located in [DerivationFrameworkInDet](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkInDet/src). These are:

* `TrackParticleThinning` : direct removal of TrackParticles, using the text parser tool
* `EGammaTrackParticleThinning` : removal of TrackParticles not associated to electrons/photons, with the text parser being used to select the relevant electrons/photons whose TrackParticles should be retained
* `MuonTrackParticleThinning` : removal of TrackParticles not associated to muons, with the text parser being used to select the relevant muons whose TrackParticles should be retained
* `JetTrackParticleThinning` : removal of TrackParticles not associated to jets, with the text parser being used to select the relevant jets whose TrackParticles should be retained
* `TauTrackParticleThinning` : removal of TrackParticles not associated to taus, with the text parser being used to select the relevant taus whose TrackParticles should be retained
* `DiTauTrackParticleThinning`: removal of TrackParticles not associated to di-taus, with the text parser being used to select the relevant taus whose TrackParticles should be retained

They are demonstrated in the [PHYS format](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys/python/PHYS.py) and have very similar interfaces. For example, to thin TrackParticles directly:

```python
from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg

PHYS_thinning_expression = "InDetTrackParticles.DFCommonTightPrimary && abs(InDetTrackParticles.DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"

PHYSTrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
    ConfigFlags,
    name                    = "PHYSTrackParticleThinningTool",
    StreamName              = kwargs['StreamName'], 
    SelectionString         = PHYS_thinning_expression,
    InDetTrackParticlesKey  = "InDetTrackParticles"))
```

whereas to remove tracks that are not associated with muons:

```python
from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg
PHYSMuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
    ConfigFlags,
    name                    = "PHYSMuonTPThinningTool",
    StreamName              = kwargs['StreamName'],
    MuonKey                 = "Muons",
    InDetTrackParticlesKey  = "InDetTrackParticles"))
```

In this latter case no thinning selection is provided because muon objects are never thinned.

---++++ Generic xAOD objects
The [GenericObjectThinning](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkTools/src/GenericObjectThinning.cxx) tool allows the thinning of any xAOD object. It is used to thin di-taus in the [PHYS format](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkPhys/python/PHYS.py):

```python
from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg
tau_thinning_expression = "(TauJets.ptFinalCalib >= 0)"
PHYSTauJetsThinningTool = acc.getPrimaryAndMerge(GenericObjectThinningCfg(ConfigFlags,
    name            = "PHYSTauJetThinningTool",
    StreamName      = kwargs['StreamName'],
    ContainerName   = "TauJets",
    SelectionString = tau_thinning_expression))
```

Other object types can be treated similarly.

---++++ Truth thinning

Many groups now use one of the pre-defined menus of truth content. Please see [this page](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TruthDAOD) for more information. If you wish to just thin the main truth record as found in the AOD, there are two tools available in [DerivationFrameworkMCTruth](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkMCTruth/src) for this purpose:
* [GenericTruthThinning](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkMCTruth/src/GenericTruthThinning.cxx): requirements on particles to be kept are set via the ExpressionParser (so arbitrary selections are possible). It has options to retain all descendants or ancestors of the selected particles (both vertices and particles), thereby preserving graph completeness.
* [MenuTruthThinning](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkMCTruth/src/MenuTruthThinning.cxx): requirements on particles to be kept are set via a menu. It has options to retain all descendants or ancestors of the selected particles (both vertices and particles), thereby preserving graph completeness.

Refer to the class definitions for further information.

## Running with pAthena

You can run the derivation framework on the Grid, via the following pAthena command:

`pathena --trf "Derivation_tf.py --CA --inputAODFile=%IN --outputDAODFile=%OUT.pool.root --maxEvents=5000 --skipEvents=0 --formats=PHYSLITE" --inDS INPUTDATASET --outDS OUTPUTDATASET`

e.g.

`pathena --trf "Derivation_tf.py --CA --inputAODFile=%IN --outputDAODFile=%OUT.pool.root --maxEvents=5000 --skipEvents=0 --formats=PHYSLITE" --inDS mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.merge.AOD.e6337_s3681_r12960_r12963 --outDS user.username.410470.PHYSLITE_CA_v2`

## Special instructions for running from EVNT

Running the truth derivations (TRUTH0, TRUTH1, TRUTH3) from EVNT requires the use of the flag `--inputEVNTFile`, as follows:

`Derivation_tf.py --CA --inputEVNTFile evnt.pool.root --outputDAODFile test.pool.root --formats TRUTH3`

See [TruthDAOD](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TruthDAOD) for more information.

## Technical validation of the Derivation Framework

In order to assess the impact of various changes in the derivation framework, ART tests run every night. See DerivationFrameworkART for more information.

## Useful Tools
### Checking What Branches an Analysis Needs

It's often useful when checking smart slimming lists, or debugging missing variables to run over a full AOD file and dump a list of all used variables.
Analysis frameworks studies can do this by including the following lines in the code.

```c++
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/ReadStats.h"
//after execution loop
xAOD::IOStats::instance().stats().printSmartSlimmingBranchList();
```
## Guidelines on migrating to run 3
All of the instructions above pertain to run 3, so if you need to migrate run 2 DAOD job options to run 3, you should follow this template. Here are some additional guidelines that should be followed:

* Do not change the name of your formats just because it is being migrated from run 2 to run 3. The name should stay the same if it is substantially the same format.
* The name of the script defining your format should also stay the same - FORMAT.py - but should be relocated to the `python` directory as described above.
* Config methods within your format should be named as follows: `FORMATCfg` for the top level; `FORMATKernelCfg` for the method setting up the kernel; `FORMATBlahToolCfg` for methods configuring tools
* Always pass the `ConfigFlags` to these methods, even if they aren't currently needed
* Always set tools as public tools, otherwise it will not be possible to configure them differently from the default
* If you need to set up separate scripts for configuring tools, they should be called `BlahConfig.py` (and then any method within them should end with `Cfg`)
* All formats used in run 3 must be listed in [this spreadsheet](https://docs.google.com/spreadsheets/d/1GUB1XLlpcHQ9BcARSdvnoSuG7FnB9jcctu43ZcsbY8Q/edit#gid`0), and before you add to it, please discuss with the DAOD coordination group in advance.

