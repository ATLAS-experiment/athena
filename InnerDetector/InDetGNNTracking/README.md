# Graph Neural Network for ITk tracking

## To Fit track candidates from ACORN

GNN can be configured through the flags defined in `InDetGNNTrackingConfigFlags.py`. For example, to change the input dictory for the TrackReader, one can set the flag `flags.Tracking.GNN.TrackReader.inputTracksDir = "gnntracks"`. The following is an example of how to run the GNN track fitting on the ACORN track candidates.

```bash
function gnn_tracking() {
    rm InDetIdDict.xml PoolFileCatalog.xml
    # export ATHENA_CORE_NUMBER=6
    #--skipEvents 44

    Reco_tf.py \
        --CA 'all:True' --autoConfiguration 'everything' \
        --conditionsTag 'all:OFLCOND-MC15c-SDR-14-05' \
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
