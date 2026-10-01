# Traccc-as-a-Service Triton Client

This package contains the client which interfaces with a Triton backend running `traccc` for 
full-chain tracking. To properly test, the Triton server needs to be setup with an ingress point 
set to `$TRITON_URL` with port set to `$TRITON_PORT` as used below. 

## Setting up the backend

The backend can be setup on the EF tracking testbed using the following instructions. First, pull
the image with:

```bash
apptainer pull --disable-cache <PATH_TO_EOS>/traccc-aas.sif \
    docker://milescb/traccc-aas:send_cell_buffer_traccc_v1.6.0_triton26.06
```

You'll need to get the `ITk` geometry files, which is possible by being a memeber of the 
`atlas-tdaq-phase2-EFTracking-developers` e-group. To mount these files in the image run:

```bash
# needed for proper mounting from eos
mkdir -p /tmp/$USER/itk-geo
cp /eos/project/a/atlas-eftracking/GPU/ITk_data/FinalReport/* /tmp/$USER/itk-geo/

# launch the container
apptainer run --nv \
    --bind /tmp/$USER/itk-geo:/traccc/itk-geometry:ro \
    <PATH_TO_EOS>/traccc-aas.sif bash
```

Once in the container, run `tritonserver --model-repository=/traccc-aaS/traccc-aaS/backend/models/`. 
If things have worked properly, you should see in the output somewhere

```
+------------+---------+--------+
| Model      | Version | Status |
+------------+---------+--------+
| traccc-gpu | 1       | READY  |
+------------+---------+--------+
```

## Running the client

The client can then be run simply with `Reco_tf.py`. First, setup appropriate environment variables:

```bash
RDO_FILENAME="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583_tid39626672_00/RDO.39626672._001121.pool.root.1"
AOD_OUTFILE="AOD.test.pool.root"
export ATHENA_CORE_NUMBER=1
TRITON_URL="localhost"
TRITON_PORT=8001
```

Then, run this package with:

```bash
Reco_tf.py \
    --CA \
    --inputRDOFile "${RDO_FILENAME}" \
    --outputAODFile "${AOD_OUTFILE}" \
    --preInclude "TracccTritonClient.TracccTritonClientConfigFlags.tracccTritonFlagsPreInclude" \
    --preExec "flags.Tracking.Traccc.Triton.model = \"traccc-gpu\"; \
        flags.Tracking.Traccc.Triton.url = \"$TRITON_URL\"; \
        flags.Tracking.Traccc.Triton.port = $TRITON_PORT;" \
    --steering doRAWtoALL \
    --postInclude "TracccTritonClient.TracccTritonClientConfig.TritonTracccTrackMakerCfg,ActsConfig.ActsPostIncludes.ACTSClusterPostInclude" \
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
        "OfflineTrkKey": "TracccTrackParticles",
        "plotResolutions": false,
        "MatchingType"  : "TruthMatch",
        "doClusterValidation"  : false,
        "plotTechnicalEfficiencies": true,
        "useActsSiMeasurements": true,
        "PixelClusterKey"  : "TracccPixelClusters",
        "StripClusterKey"  : "TracccStripClusters",
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