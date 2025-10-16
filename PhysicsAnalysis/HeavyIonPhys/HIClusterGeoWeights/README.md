# HIClusterGeoWeights package

This package contains all the code to create the `cluster.geo.XXX.root` file that is later used by [`getHIClusterGeoWeightFile`](../../../Reconstruction/HeavyIonRec/HIJetRec/python/HIJetRecUtilsCA.py#L7) and provided to the heavy-ion jet reconstruction.
The final files are eventually stored in `/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HIJetCorrection`. There should be at least one file per data-taking period.


## Content of the `cluster.geo.XXX.root` file

* `h3_w` - created by [`HICaloGeoExtract.py`](python/HICaloGeoExtract.py); needs only a single event to get the geometry
* `h3_eta` - created by [`HICaloGeoExtract.py`](python/HICaloGeoExtract.py); needs only a single event to get the geometry
* `h3_phi` - created by [`HICaloGeoExtract.py`](python/HICaloGeoExtract.py); needs only a single event to get the geometry
* `h3_R` - created by [`HICaloGeoExtract.py`](python/HICaloGeoExtract.py); needs only a single event to get the geometry

* `h3_eta_phi_response` - created by [`HIClusterGeoFiller`](HIClusterGeoWeights/HIClusterGeo_HistoFiller.h) and [`makeHIResponse`](util/makeHIResponse.cxx) after the final GRL is known
* `h3_eta_phi_offset` - created by [`HIClusterGeoFiller`](HIClusterGeoWeights/HIClusterGeo_HistoFiller.h) and [`makeHIResponse`](util/makeHIResponse.cxx) after the final GRL is known
* `h1_run_index` - created by [`makeHIResponse`](util/makeHIResponse.cxx) after the final GRL is known


## Content of this package to create the `cluster.geo.XXX.root` file

### HICaloGeoExtract

Extracts areas of calorimeter cells and stores them.
This is done once per data-taking period.
Creates `cluster.geo.W_ETA_PHI_R.root`.

### HIClusterGeoFiller

The algorithm reads `HIClusters` from AOD and fills 2D histograms.
This is done once for each run separately.

It needs to run over both CC and PC streams, so the input will represent the minimum-bias event selection.

### makeHIResponse

After the data-taking period, when the final GRL is available, this macro combines the histograms created by `HIClusterGeoFiller` from the good lumiblocks in runs passing GRL, and creates `h3_eta_phi_response`, `h3_eta_phi_offset`, and `h1_run_index` histograms.
Ensure that you combine outputs from CC and PC streams beforehand. There shall be one input file per run with the run number in its name.
Creates `cluster.geo.RESPONSE_OFFSET_RUNINDEX.root`.

## Final `cluster.geo.XXX.root` file

To get the final file, which is about to be stored, one has to simply merge the two output files:
```
hadd cluster.geo.XXX.root cluster.geo.W_ETA_PHI_R.root cluster.geo.RESPONSE_OFFSET_RUNINDEX.root
```
Don't forget to replace `XXX` with some appropriate name, e.g. `DATA_PbPb_2025`.

## Additional content of this package

### compareHIClusterGeoFiles

Script comparing two `cluster.geo.XXX.root` files. It may compare only some of the histograms.

### makeHIRunIndex

Creates `h1_run_index` from the provided GRL. For testing purposes only; nominally, the histogram is created by `makeHIResponse`. 

## Submission to grid

It might be useful to submit `HIClusterGeoFiller` to the grid.
That can be achieved with a command analogous to this: 
```
lsetup panda
pathena --trf "python -m HIClusterGeoWeights.HIClusterGeoFiller --filesInput=%IN" --inDS data18_hi.00367273.physics_CC.merge.AOD.f1030_m2048 --outDS user.$USER.data18_hi.00367273.physics_CC.merge.AOD.f1030_m2048.cluster.geo --extOutFile=HIClusterGeo_HistoFiller.root
```
