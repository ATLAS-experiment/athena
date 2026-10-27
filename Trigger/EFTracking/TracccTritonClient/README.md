# Traccc-as-a-Service Triton Client

This package contains the client which interfaces with a Triton backend running `traccc` for 
full-chain tracking. To properly test, the Triton server needs to be setup with an ingress point 
set to `$TRITON_URL` with port set to `$TRITON_PORT` as used below. 

## Setting up the backend

To run the server, all that is needed is:

```sh
tritonserver --model-repository=${WorkDir_DIR:-$Athena_DIR}/data/TracccTritonBackend/models --grpc-port 8042
```

A successful start ends with the below somewhere in the output

```
+------------+---------+--------+
| Model      | Version | Status |
+------------+---------+--------+
| traccc-gpu | 1       | READY  |
+------------+---------+--------+
```

For more details, the backend is build from the `Trigger/EFTracking/TracccTritonBackend` package.

## Running the client

The client is the primary ITk tracking pass of a standard reconstruction job, with
`flags.Acts.Device.tritonMode = DeviceTritonMode.Client`. In this mode
`InDetConfig.ITkActsDeviceTrackRecoConfig.ITkActsDeviceTrackRecoCfg` schedules
`TracccTritonClientConfig.TritonTracccTrackMakerCfg`, which converts the RDOs into traccc cells,
sends them to the server with `TritonTracccTrackMaker`, and converts the traccc collections the
server returns as in the local device chain. The tracks and clusters therefore end up in
the standard containers (`ActsTracks`, `ITkPixelClusters`, `ITkStripClusters`, `InDetTrackParticles`)
and the usual truth association and AOD content apply. The `Acts.Device.doClusterization`,
`doSeeding` and `doTrackReconstruction` flags must stay `False` on the client.
The client builds the same traccc detector description as the server, so it needs the ACTS Gen3
(blueprint) tracking geometry with the Detray conversion, and the same detector (no HGTD).

The server runs the same function with `DeviceTritonMode.Server`, so it uses the cuts of the
same tracking pass as the client.

First, setup appropriate environment variables:

```bash
RDO_FILENAME="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583_tid39626672_00/RDO.39626672._001121.pool.root.1"
AOD_OUTFILE="AOD.test.pool.root"
export ATHENA_CORE_NUMBER=1
TRITON_URL="localhost"
TRITON_PORT=8042
```
(Due to a bug in the TritonClient code, the default port of 8001 cannot be used. This will be updated in a future MR)

Then, run this package with:

```bash
Reco_tf.py \
    --CA \
    --inputRDOFile "${RDO_FILENAME}" \
    --outputAODFile "${AOD_OUTFILE}" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,TracccTritonClient.TracccTritonClientConfigFlags.tracccTritonFlagsPreInclude" \
    --preExec "from ActsConfig.ActsConfigFlags import DeviceTritonMode; \
        flags.Acts.Device.tritonMode = DeviceTritonMode.Client; \
        flags.Detector.EnableHGTD = False; \
        flags.Acts.TrackingGeometry.UseBlueprint = True; \
        flags.Acts.TrackingGeometry.BuildDetrayGeometry = True; \
        flags.Tracking.Traccc.Triton.model = \"traccc-gpu\"; \
        flags.Tracking.Traccc.Triton.url = \"$TRITON_URL\"; \
        flags.Tracking.Traccc.Triton.port = $TRITON_PORT;" \
    --steering doRAWtoALL \
    --postInclude "ActsConfig.ActsPostIncludes.ACTSClusterPostInclude" \
    --maxEvents 1 --conditionsTag "all:COND-MC21-SDR-RUN4-06" \
    --jobNumber '1' \
    --perfmon 'fullmonmt'
```

## Plotting the output

To quickly make plots, create the configuration file `EFTracking_TrkAnaConfig.json` with contents:

```
{
    "TrkAnaEF_TM" : {
        "enabled" : true,
        "TestType"  : "Offline",
        "RefType"   : "Truth",
        "OfflineTrkKey": "InDetTrackParticles",
        "plotResolutions": false,
        "MatchingType"  : "TruthMatch",
        "doClusterValidation"  : false,
        "plotTechnicalEfficiencies": true,
        "useActsSiMeasurements": true,
        "PixelClusterKey"  : "ITkPixelClusters",
        "StripClusterKey"  : "ITkStripClusters",
        "OfflineQualityWP": "EFTracking",

        "plotVertexParameters" : false,
        "OfflineVtxKey" : "",
        "TruthVtxKey" : ""
    }
}
```

Then, run the command:

```bash
runIDTPM.py --inputFileNames ${AOD_OUTFILE} \
    --outputFile IDTPM_output \
    --trkAnaCfgFile ../EFTracking_TrkAnaConfig.json
```

The output will contain the parameters of the tracks from traccc, as well as efficiencies when 
compared to truth.