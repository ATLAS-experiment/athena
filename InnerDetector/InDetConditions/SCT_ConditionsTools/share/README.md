Masking of Strip Modules in ITk can be done by:

- Reading bad module IDs from a JSON file.
- Reading bad module IDs from the database.

Creating a JSON file for masking:

- Reding bad module IDs from a JSON file allows more flexibility in choosing specific modules e.g. selecting modules from only one specific layer or disc, selecting random modules from a particular layer etc.

- In order to make this selection first the entire list of modules has to be converted into a JSON file and then another script can be use to select specific modules from this list.

- First run the script: /athena/InnerDetector/InDetExample/InDetDetDescrExample/tools/RunPrintSiDetElements.py using: python RunPrintSiDetElements.py
- This will create 2 files, one for Pixels and one for Strips, named `PixelGeometry.dat` and `StripGeometry.dat` containing the list of modules along with their specifications e.g. whether the module in the barrel or end cap (#barrel_ec), to which layer or disk does (#layer_disk) it belong to, its number in phi and eta (#phi_module and #eta_module) and which side of the detector does it belong to (#side) etc.
- Use the python script /athena/InnerDetector/InDetExample/InDetDetDescrExample/tools/geometry_dat_to_json.py to convert a geometry.dat into a json file say geometry.json
`python InDetDetDescrExample/geometry_dat_to_json.py --infile PixelGeometry.dat --outfile PixelGeometry.json`

- Use /athena/InnerDetector/InDetConditions/SCT_ConditionsTools/share/module_selector_from_json.py to select modules from the geometry.json and create a separate json file.

Masking Strip Modules using a JSON file:
- To use a json file the reconstruction command should use just a preExec. In this example the jsonFile=/path/to/jsonfile is a variable with the absolute path to the json file with the modules to mask
   --preExec "ConfigFlags.ITk.doStripModuleVeto = True; ConfigFlags.ITk.JsonPathStripModuleVeto=\"$jsonFile\"" \
- The preExec command is the flag for masking Strip modules and passing the json path to all the ITkStripConditionSummaryTool instances

Job Example:

```
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=50
jsonFile=/eos/home-p/pibutti/sw/defects/run/SCT_Defects/test_selected_modules.json
outFile="AOD.withDefects.pool.root"

Reco_tf.py \
     --inputRDOFile  ${input_rdo} \
     --outputAODFile ${outFile} \
     --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
     --preExec "ConfigFlags.ITk.doStripModuleVeto = True; ConfigFlags.ITk.JsonPathStripModuleVeto=\"$jsonFile\"" \
     --maxEvents ${n_events} \
     --multithreaded 'True'
```



Masking Strip Modules using SQLite DB (not recently tested):
- To use database one needs to turn on the flag "kwargs.setdefault("useDB", False)" in the /athena/InnerDetector/InDetConditions/SCT_ConditionsTools/python/ITkStripConditionsToolsConfig.py
- Also the reconstruction command should include:
   --preExec 'ConfigFlags.ITk.doStripModuleVeto = True'
