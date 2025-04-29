# DerivationFrameworkCore

This package contains various common code and configuration scripts needed for running the derivaton framework. For more information please refer to the [derivation framework manual](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework)


## src/DerivationFrameworkCore

These directories contain the following source code:

* The `DerivationKernel` algorithm which is used to drive the event loop for each DAOD output format. It is an `AthFilterAlgorithm` which executes arrays of tools for skimming (`ISkimmingTool`), thinning (`IThinningTool`) and augmenting the data (`IAugmentationTool`), which are passed to it via the format definition configuration file.
* The `CommonAugmentation` algorithm. It is used to run common augmentation before the main format-making kernels run. It is similar to the DerivationKernel but only permits augmentation, aand is an AthAlgorithm rather than AthFilterAlgorithm - this avoids pointless entries being made in the cut flow as the job runs.
* `GoodRunsListFilterAlgorithm`: accepts or rejects events depending on whether they are in the GRL. This may be used in tandem with kernel algorithm to filter out unwanted events. **Obsolete?**

## python

This directory contains a large number of python scripts, in particular defining the slimming mechanism.

* `SlimmingHelper` and `ContentHandler`: the main python scripts defining the slimming mechanism. Users interact with the SlimmingHelper in their format configuration, indicating which CP variables should be included and which individual branches should be retained. Using information from the user and the smart slimming lists (some of which are in this directory and others which are in the CP group packages) the SlimmingHelper builds the full list of variables that need to be kept, and applies them to the relevant output stream. The ContentHandler is used to format the strings which define the output lists.
* Smart slimming lists for the triggers (several files), event info (`EventInfoContent`), compulsory content needed for all formats (`CompulsoryContent`)
* `ContainersOnTheFly`: list of containers that are added via common augmentation and which are listed centrally to avoid users having to write them explicitly into their format definitions
* `FullListOfSmartContainers`: list of all of the containers with smart slimming lists. **Obsolete?**
* `StaticNamesAndTypes`: fall-back dictionary of container names and their types, to be used in case the job can't automatically generate the dictionary as it reads in the file metadata. **Obsolete?**

## scripts

**Contents are obsolete?**

