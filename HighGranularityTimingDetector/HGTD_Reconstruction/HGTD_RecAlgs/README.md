# HGTD Reconstruction Algorithms

## Vertex Time Reconstruction

Reconstruction of vertex time is implemented in the algorithm `VertexTimeAlg`
(defined in
[`src/VertexTimeAlg.h`](https://gitlab.cern.ch/atlas/athena/-/blob/main/HighGranularityTimingDetector/HGTD_Reconstruction/HGTD_RecAlgs/src/VertexTimeAlg.h)).
Its configuration is located in the
[`HGTD_Config`](https://gitlab.cern.ch/atlas/athena/-/tree/main/HighGranularityTimingDetector/HGTD_Config)
package in
[`HGTD_VertexTimeConfig.py`](https://gitlab.cern.ch/atlas/athena/-/blob/main/HighGranularityTimingDetector/HGTD_Config/python/HGTD_VertexTimeConfig.py).
The algorithm runs during reconstruction with `Reco_tf.py` during the RDO-to-AOD
stage, which is configured in
[`HGTD_RecoConfig.py`](https://gitlab.cern.ch/atlas/athena/-/blob/main/HighGranularityTimingDetector/HGTD_Config/python/HGTD_RecoConfig.py).

For details on the reconstruction algorithm and its implementation, see the
[TWiki
page](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/VertexTimeReconstruction).

### How to retrieve vertex time information from AOD

Vertex time information is saved in AODs as decorations on `xAOD::Vertex`. Since
only a subset of all vertices have a time reconstructed, a `hasValidTime`
decoration is available to check if the vertex has a valid precision time.
`hasValidTime` (type `uint8_t`) is set to `1` if a valid precision time is
available, and is set to `0` otherwise. Vertex time and its resolution are
available via the corresponding `time` and `timeResolution` decorations (type
`float`) - the values are stored and returned in nanoseconds (`ns`). If a valid
precision time is not available for the vertex, these two values are set to the
default values of `time` $=0$ and `timeResolution` $=50/\sqrt{12}$.

The algorithm reconstructs a time for the hard scatter interaction only.
Consequently, all vertices for which
```c++
// Assuming we have a variable `const xAOD::Vertex* vertex`
vertex->vertexType() == xAOD::VxType::VertexType::PriVtx
```
returns `false` (i.e. all vertices that are not primary) have `hasValidTime` set
to `0`.

The code below shows an example of obtaining timing information of a vertex
```c++
// Assuming we have a variable `const xAOD::Vertex* vertex`

// Always first check if a vertex has a time available
if (vertex->hasValidTime()) {
  float vertex_time { vertex->time() };
  float vertex_timeResolution { vertex->timeResolution() };
}
```

### How to run vertex time reconstruction

To run vertex time reconstruction and have vertex timing information available
in AODs, full reconstruction from either HITS or RDO can be run with
`Reco_tf.py` with Athena version 25.0.1 or later. For example, to run locally do
```bash
asetup Athena,25.0.1  # or any later version

Reco_tf.py --AMIConfig r15365 \
           --inputHITSFile <path-to-sample>/mc21_14TeV.601237.PhPy8EG_A14_ttbar_hdamp258p75_allhad.simul.HITS.e8481_s4272/* \
           --outputAODFile <outputFileName>
```
The suggested sample is a $t\bar{t}$ allhad $\mu=0$ sample from the
[ATLMCPROD-10908](https://its.cern.ch/jira/browse/ATLMCPROD-10908) production
intended for HGTD studies.

Alternatively, the algorithm can also be run standalone on AODs (e.g. if vertex
time information is not saved in the AOD, or some changes in the algorithm are
needed). In this case, the algorithm needs to be configured in a JO file. An
example JO showing how to run the algorithm is available
[here](https://gitlab.cern.ch/jedebevc/athena/-/blob/189deb26a551c7aabdd00017c1972cfed7d20c95/Control/Local/VertexTimePerformance/share/VertexTimePerf_JO.py)
(the suggested input file is the same $t\bar{t}$ allhad $\mu=0$ sample mentioned
previously). An algorithm that uses vertex time information should then be
merged after `VertexTimeAlgCfg`, so that it has the vertex time decorations
available.
