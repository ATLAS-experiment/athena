# Graph Neural Network for ITk tracking

## Dump Athena space points, clusters and training information to root ntuple for ACORN.

The following is an example on how to dump the training / evaluation data to ACORN. The examples uses an old ttbar (geometry ATLAS-P2-RUN4-03-00-00) but is only intended to show the Reco_tf configuration. Some additional information:

- Clustering, space point formation and tracking still use the Athena legacy code so a converter (InDetToXAODSpacePointConversionCfg) from the InDet:: to xAOD:: EDM needs to be scheduled. Once the tracking chain will be moved from legacy to Acts based, all inputs will be in xAOD format and such conversion won't be necessary.

- 'all:Campaigns.PhaseIIPileUp200' : this option only makes sense if we run from HITS files and it's ignored when running on RDO inputs.

- The RDO input file is just an example.

```bash
RDO_FILENAME=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1

Reco_tf.py \
         --CA 'all:True' --autoConfiguration 'everything' \
         --conditionsTag 'all:OFLCOND-MC21-SDR-RUN4-02' \
         --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \
         --multithreaded 'True' \
         --steering 'doRAWtoALL' \
         --digiSteeringConf 'StandardInTimeOnlyTruth' \
         --postInclude 'all:PyJobTransforms.UseFrontier,InDetConfig.SiSpacePointFormationConfig.InDetToXAODSpacePointConversionCfg' \
         --preInclude 'all:Campaigns.PhaseIIPileUp200' 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude'\
         --postExec 'from InDetGNNTracking.InDetGNNTrackingConfig import DumpObjectsCfg; cfg.merge(DumpObjectsCfg(flags))' \
         --inputRDOFile ${RDO_FILENAME} \
         --outputAODFile 'test.aod.gnnreader.debug.root'  \
         --maxEvents 5 2>&1 | tee log.gnnreader_debug.txt
```





## To Fit track candidates from ACORN

GNN can be configured through the flags defined in `InDetGNNTrackingConfigFlags.py`. For example, to change the input dictory for the TrackReader, one can set the flag `flags.Tracking.GNN.TrackReader.inputTracksDir = "gnntracks"`. The following is an example of how to run the GNN track fitting on the ACORN track candidates.

```bash
function gnn_tracking() {
    rm InDetIdDict.xml PoolFileCatalog.xml
    # export ATHENA_CORE_NUMBER=6
    #--skipEvents 44

    Reco_tf.py \
        --CA 'all:True' --autoConfiguration 'everything' \
        --conditionsTag 'all:OFLCOND-MC21-SDR-RUN4-02' \
        --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \
        --multithreaded 'True' \
        --steering 'doRAWtoALL' \
        --digiSteeringConf 'StandardInTimeOnlyTruth' \
        --postInclude 'all:PyJobTransforms.UseFrontier' \
        --preInclude 'all:Campaigns.PhaseIIPileUp200' 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude' 'InDetGNNTracking.InDetGNNTrackingFlags.gnnReaderValidation' \
        --preExec 'flags.Tracking.GNN.TrackReader.inputTracksDir = "gnntracks" \        
        --inputRDOFile ${RDO_FILENAME} \
        --outputAODFile 'test.aod.gnnreader.debug.root'  \
        --athenaopts='--loglevel=INFO' \
        --postExec 'msg=cfg.getService("MessageSvc"); msg.infoLimit = 9999999; msg.debugLimit = 9999999; msg.verboseLimit = 9999999;' \
        --maxEvents 1  2>&1 | tee log.gnnreader_debug.txt
}
````
