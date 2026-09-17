# Graph Neural Network for ITk tracking

## Dump Athena space points, clusters and training information to root ntuple for ACORN

The following is an example on how to dump the training / evaluation data to ACORN. The examples uses an old ttbar (geometry ATLAS-P2-RUN4-03-00-00) but is only intended to show the Reco_tf configuration. Some additional information:

- Clustering, space point formation and tracking still use the Athena legacy code so a converter (InDetToXAODSpacePointConversionCfg) from the InDet:: to xAOD:: EDM needs to be scheduled. Once the tracking chain will be moved from legacy to Acts based, all inputs will be in xAOD format and such conversion won't be necessary.

- 'all:Campaigns.MC23PhaseIIPileUp200' : this option only makes sense if we run from HITS files and it's ignored when running on RDO inputs.

- The RDO input file is just an example.

```bash
RDO_FILENAME=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

Reco_tf.py \
         --conditionsTag 'all:${conditionsTag}' \
         --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \
         --multithreaded 'True' \
         --steering 'doRAWtoALL' \
         --digiSteeringConf 'StandardInTimeOnlyTruth' \
         --postInclude 'all:PyJobTransforms.UseFrontier,InDetConfig.SiSpacePointFormationConfig.InDetToXAODSpacePointConversionCfg' \
         --preInclude 'all:Campaigns.MC23PhaseIIPileUp200' 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude'\
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

    conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

    Reco_tf.py \
        --conditionsTag 'all:${conditionsTag}' \
        --geometryVersion 'all:ATLAS-P2-RUN4-03-00-00' \
        --multithreaded 'True' \
        --steering 'doRAWtoALL' \
        --digiSteeringConf 'StandardInTimeOnlyTruth' \
        --postInclude 'all:PyJobTransforms.UseFrontier' \
        --preInclude 'all:Campaigns.MC23PhaseIIPileUp200' 'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude' 'InDetGNNTracking.InDetGNNTrackingFlags.gnnReaderValidation' \
        --preExec 'flags.Tracking.GNN.TrackReader.inputTracksDir = "gnntracks" \        
        --inputRDOFile ${RDO_FILENAME} \
        --outputAODFile 'test.aod.gnnreader.debug.root'  \
        --athenaopts='--loglevel=INFO' \
        --postExec 'msg=cfg.getService("MessageSvc"); msg.infoLimit = 9999999; msg.debugLimit = 9999999; msg.verboseLimit = 9999999;' \
        --maxEvents 1  2>&1 | tee log.gnnreader_debug.txt
}
```

## Run ACTS-based GNN pipeline

To run the ACTS integrated pipeline, one can use the following `Reco_tf` script:
The GNN model can either be an `.onnx`, `.pt` or `.engine` file.
The MM path should points to the directory where the two `*.doublets.root` and `*.triplets.root` files are.
MMG expect a path without the 2 suffix.

For example, `module_map_path` can be `/path/to/modulemaps/merged_ttbar_plus_singles_mmg1.3.0_cleaned_mean_rms_replaced_thr5_tol1e-10_float` (without `*.doublets.root` and `*.triplets.root` at the end).

```bash
    conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

    Reco_tf.py --CA 'all:True' \
        --conditionsTag "all:${conditions_tag}" \
        --digiSteeringConf 'StandardInTimeOnlyTruth' \
        --geometryVersion "all:ATLAS-P2-RUN4-03-00-00" \
        --multithreaded 'True' \
        --steering 'doRAWtoALL' \
        --preInclude 'all:Campaigns.MC23PhaseIIPileUp200' \
            'InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude' \
            'InDetGNNTracking.InDetGNNTrackingFlags.gnnActsPipelineValidation' \
        --preExec "flags.ITk.doEndcapEtaNeighbour=True; \
            flags.Tracking.GNN.ActsPipeline.moduleMapPath=\"${module_map_path}\"; \
            flags.Tracking.GNN.ActsPipeline.gnnPath=\"${gnn_model_path}\"; \
            flags.Tracking.GNN.ActsPipeline.saveEdgeScore = True" \
        --postInclude 'all:PyJobTransforms.UseFrontier' \
        --inputRDOFile "$RDO_FILE" \
        --outputAODFile "$AOD_FILE" \
        --maxEvents 10 \
```
