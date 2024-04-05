Masking of Strip Modules in ITk can be done by:

- Reading bad module IDs from a JSON file.
- Reading bad module IDs from the database.


Creating a JSON file for masking:

- Reding bad module IDs from a JSON file allows more flexibility in choosing specific modules e.g. selecting modules from only one specific layer or disc, selecting random modules from a particular layer etc.

- In order to make this selection first the entire list of modules has to be converted into a JSON file and then another script can be use to select specific modules from this list.

- First run the script: /athena/InnerDetector/InDetExample/InDetDetDescrExample/tools/RunPrintSiDetElements.py using: python RunPrintSiDetElements.py
- This will create a file named geometry.dat containing the list of modules along with their specifications e.g. whether the module in the barrel or end cap (#barrel_ec), to which layer or disk does (#layer_disk) it belong to, its number in phi and eta (#phi_module and #eta_module) and which side of the detector does it belong to (#side) etc.
- Use the python script /athena/InnerDetector/InDetExample/InDetDetDescrExample/tools/geometry_dat_to_json.py to convert geometry.dat into a json file say geometry.json
- Use /athena/InnerDetector/InDetConditions/SCT_ConditionsTools/share/module_selector_from_json.py to select modules from the geometry.json and create a separate json file.

Masking Strip Modules using a JSON file:
- To use a json file the reconstruction command should use include 
   --preExec 'ConfigFlags.ITk.doStripModuleVeto = True'
   --postExec 'cfg.getEventAlgo("ITkStripClusterization").conditionsTool.ConditionsTools["ITkStripModuleVetoTool"].JsonPath = "<path_to_json_file>"'
- The preExec command is the flag for masking Strip modules.
- The postExec command sets the path where the json file is located.

Masking Strip Modules using SQLite DB:
- To use database one needs to turn on the flag "kwargs.setdefault("useDB", False)" in the /athena/InnerDetector/InDetConditions/SCT_ConditionsTools/python/ITkStripConditionsToolsConfig.py
- Also the reconstruction command should include:
   --preExec 'ConfigFlags.ITk.doStripModuleVeto = True'
