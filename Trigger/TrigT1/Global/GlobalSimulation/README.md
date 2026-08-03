To run the standard global simulation, export a valid configuration xml and use `l1calo-ath-mon` with the `Trigger.L1.dogFex`, `Trigger.L1.doeFex` options enabled:

```bash
export GS_CFG_FILE=PATH-TO-SOURCE/athena/Trigger/TrigT1/Global/GlobalSimulation/share/globalSim_AllChainsCfg.xml 
l1calo-ath-mon --postInclude GlobalSimulation/plugin_local_cfg.py --filesInput RAW_RUN3_DATA24 --evtMax 10 -- IOVDb.GlobalTag='CONDBR2-BLKPA-2024-05' Trigger.L1.dogFex=True Trigger.L1.doeFex=True
```

Please see `l1calo-ath-mon --help` for more options and instructions.
If you want to run different chains, then you can create and export your own globalSim.xml

### Enabling debugging output on components

If you want `DEBUG` level output on one of your components, simply add to your command:

```bash
cfg.<AlgorithmTypeOrName>.OutputLevel=2
```

e.g. to turn on debugging on an algorithm called `GlobalSimTestAlg` do:

```bash
l1calo-ath-mon .... -- cfg.GlobalSimTestAlg.OutputLevel=2
```

### Enabling storegate dumping

To turn on StoreGate dumping, do:

```bash
l1calo-ath-mon .... -- cfg.StoreGateSvc.Dump=True
```

### Including the GraphSvc in your job

If you want to generate a graph for your job, you can include the `GraphSvc` by creating another plugin file e.g. in your local dir with the following content:

```python
cfg.addService( CompFactory.GlobalSim.GraphSvc(), create=True )
```
then just add that to your `--postIncludes` argument.
