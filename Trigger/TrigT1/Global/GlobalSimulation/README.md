To run the standard global simulation, export a valid configuration xml and use `l1calo-ath-mon` with the `Trigger.L1.dogFex`, `Trigger.L1.doeFex` options enabled:

```bash
export GS_CFG_FILE=PATH-TO-SOURCE/athena/Trigger/TrigT1/Global/GlobalSimulation/share/globalSim_AllChainsCfg.xml 
l1calo-ath-mon --postInclude GlobalSimulation/plugin_local_cfg.py --filesInput RAW_RUN3_DATA24 --evtMax 10 -- IOVDb.GlobalTag='CONDBR2-BLKPA-2024-05' Trigger.L1.dogFex=True Trigger.L1.doeFex=True
```

Please see `l1calo-ath-mon --help` for more options and instructions.
If you want to run different chains, then you can create and export your own globalSim.xml