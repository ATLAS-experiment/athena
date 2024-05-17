# FPGATrackSimSGInput
This package contains components necessary to generate FPGATrackSim Simulation wrapper files
They work only in the RDO to ESD transform. That is they need both RDO input objects as well as the reconstructed objects. 

Example command:
```sh
Reco_tf.py --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" --inputRDOFile  RUN4_muons.RDO.pool.root --outputESDFile ESD.test.root --steering doRAWtoALL  --postInclude "FPGATrackSimSGInput.FPGATrackSimSGInputConfig.FPGATrackSimSGInputCfg" --preExec='flags.Trigger.FPGATrackSim.wrapperFileName="NewWrapper.root";flags.Trigger.FPGATrackSim.wrapperMetaData="This Is My Meta Data, Put It All here!!!!"'
```
