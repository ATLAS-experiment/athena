# DerivationFrameworkPhys

This package contains the configuration files for the unskimmed `PHYS` and `PHYSLITE` data types which are to be used in the majority of ATLAS analyses for run 3. Additionally the common configuration used in these two data types may alse be used as the basis for other ("residual") data types. Since these files impact a large number of physicists they should not be adjusted without prior consultation in the analysis model group (AMG).

The package contains the following files:

* `PHYS.py` and `PHYSLITE.py`: the configuration of the `PHYS` and `PHYSLITE` data types themselves. As well as setting up the driving algorithm ("kernel") for the jobs, these files mainly configure directly the slimming (branch-wise content) but not the common physics content which is configured elsewhere (see below).    
* `SKIM.py`: special format to allow command line skimming. See below.
* `PhysCommonConfig.py`: the configuration of the common physics content, shared by `PHYS`, `PHYSLITE` and any residual DAOD data types that need it. This file pulls in the config fragments from all of the reconstruction domains plus truth and trigger matching (in the case of upgrade samples). 
* `PhysCommonThinningConfig.py`: configures the thinning (object removal) tools settings for the `PHYS` and `PHYSLITE` formats. The two data types share the majority of the settings, but `PHYSLITE` has a few extra in addition. The lists of tools to be used are defined in the individual config files for the two formats. 
* `TriggerMatchingCommonConfig.py`: configures the trigger matching for DAOD content for the run 2 trigger EDM. In run 3 the analysis-level trigger navigation is small enough to be added so the matching is done by the physicists directly, and this file isn't required.

For more details please refer to [the manual](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework)

## Command line skimming

This package defines a special format called SKIM, which allows skimming of PHYS and PHYSLITE to be done on the command line, rather than having to define a new format with its own config file. The usage is as follows:

```
Derivation_tf.py --CA --inputDAOD_PHYSLITEFile DAOD_PHYSLITE.pool.root --outputD2AODFile output.pool.root --formats SKIM --skimmingExpression "count(AnalysisMuons.pt > (1 * GeV)) >= 1" --skimmingContainers xAOD::MuonContainer/AnalysisMuons
``` 

The skimming expression, which must be enclosed in quote marks, uses exactly the same synatx as the other DAOD formats (e.g. it is read by the same ExpressionEvaluation tool). This means that, should a user wish to apply the same skimming to PHYS/PHYSLITE as was applied to their old DAOD, they can use the same selection string, as long as they change the container names in the case of PHYSLITE.  

The last argument is a list of all of the containers that are referenced by the skimming expression, in the form `xAOD::ContainerType/ContainerName`, e.g. `xAOD::MuonContainer/AnalysisMuons` in the example above. If more than one container is referenced each one should be listed (space separated). 

No slimming is applied, e.g. the per-event contents are identical to the input. Note that the output is D2AOD - this is because the input is DAOD.

This is not intended to be used in official DAOD production, because the format name (`D2AOD_SKIM`) is fixed and different skims would have indistinguishable dataset names. Should official production be needed of a PHYS/PHYSLITE skim, the format should be defined with its own config fragment and given its own format name. However, this might be the basis of a future user skimming service, should ATLAS decide to have one. 
