# Process-specific information for `jjj` production


This is **WORK IN PROGRESS**, collecting *prelimiary* experience from @jkretz.

This process targets inclusive dijet and trijet production at NLO,
merged with the MiNLO procedure.

Relevant test productions:
* https://its.cern.ch/jira/browse/ATLMCPROD-10232

Related QTs:
* https://its.cern.ch/jira/browse/AGENE-2073


## Production workflow

As usual for Powheg processes, LHE production and Pythia8/Herwig7
showering are in principle separated. Furthermore, as 'usual' we slice
the showered samples as function of the leading jet pT into JZ1, JZ2,
JZ3, ... (see
[JetFilter_JZX_Fragment.py](https://gitlab.cern.ch/atlas/athena/-/blob/21.6/Generators/GeneratorFilters/share/common/JetFilter_JZX_Fragment.py)
for the definition of the pT boundaries).

The production for this specific process has the challenge that the jet
pT spectrum is steeply falling, negative weight fraction and spread of
weights can be large, and generation speed is rather low (**WIP** computing PDF weights has not even been explored).
Therefore, we opted to separate the LHE production and showering:
* the LHE file production is the most CPU-intense part
* multiple LHE files are merged for showering and the jet-slicing

A typical set of LHE and shower jO can be found at the above JIRA ticket and productions links within.

## LHE production setup and integrations

Similar to the `jj` process, the LHE generation is tuned by adjusting the parameters
```
PowhegConfig.bornktmin = X
PowhegConfig.bornsuppfact =Y
```
for each slice separately:
* `bornktmin` cuts lower pT and needs to be sufficiently low to not
  bias the slide turnon and sufficiently high to not make generation
  inefficient.
* `bornsuppfact` is a value in a suppression function that will
  smoothly down-weight the cross section below that value to enhance
  the phase space around this value. It is typically set around the
  upper end of the JZX slice

In addition, the folding parameters were tuned for each slice to get a
good compromise between negative weights and generation speed (see
general PowhegColtrol doc if you want to know more).

**WIP**: The functional form of the *Born suppression factor* was
found to be not quite sufficiently steep to efficiently generate
events, therefore we are currently exploring a modification. We are
also still testing two options for the `bornktmin` parameter.

These processes will require integrations to be prepared. The
above-mentioned LHE jO have already the 13 TeV integration grids
stored, but any different setup and different sqrt(s) value would need
integrations to be prepared.

**Expect a run time of about 7-30(!) days on 8 CPU cores.** As I had a 64
CPU core node available to myself, I simply launched the integrations
from the command line and waited the needed time. I believe the rough
sequence of commands would be like:
```
mkdir jjj_JZ6
cd jjj_JZ6
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
# I used 21.6.99 for LHE production, probably safe to stay with that one
asetup AthGeneration, 21.6.99,here
# 8 cores, go higher if you have more?!
export ATHENA_CORE_NUMBER=8
# get a jO known to work
cp https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950562/mc.Ph_jjj_MiNLO_expo4_kttight_JZ6_valid.py .
Gen_tf.py --ecmEnergy=13000 --jobConfig=$PWD --outputEVNTFile=tmp.EVNT.pool.root --randomSeed=432 --maxEvents=100 >& log&
# wait roughly 7-30 days
# collect integration_grids.tar.gz and rename to sth like mc_13TeV.Ph_jjj_MiNLO_expo4_kttight_JZ6.GRID.tar.gz
```

This would have to be repeated for all desired slices. Note that the
JZ1 and JZ2 slices are slowest, while JZ6-9 are fastest (this appears to almost exclusively depend on the folding settings).



## Shower setup and Cross section * filter efficiency calculation

Matching should be fine in Pythia8 using Main31 using the
'default' procedures, see above links. 

**WIP**: Herwig7 showering. Note also that in principle MPI processes
should be vetoed in the matching. For the `jj` process this has at
least for Pythia8 been tested and found to bring only a minor effect.

Take note how in the above linked shower jO the pT ranges are extended
below/above the nominal range for the JZ1/JZ9 slices.

The setup **will most certainly not work with Pythia8.3**, because of
the missing `Powheg:nFinal = -1` option. Above showering used Using
release 21.6.82 with Pythia8.245.

In the preparation of the samples we have observed some instabilities
to obtain the needed cross section times filter efficiencies ($`\sigma
\times \epsilon_\mathrm{filt}`$). This appears ultimately related to a
sometimes significant spread in event weight, significant negative
weight fraction, and finally to the way that AMI averages these.

For the weighted Powheg generation, the cross section is obtained as
$`\sigma = \frac{\sum_\mathrm{LHE} w}{N}`$, while the filter
efficiency is $`\epsilon_\mathrm{filt} = \frac{\sum_\mathrm{EVNT}
w}{\sum_\mathrm{LHE} w}`$ (this is already implemented in the tools,
it is just written out here for explanation purposes). Here the two
sum of weights refer to all (processed) events in the LHE file and the
output EVNT (passing the filter), respectively. The way the
computation and averaging is implemented, one or a few large/negative
weight events in the LHE file that even do not make it into the final
EVNT sample will upset both calculations in a correlated way and lead
to broad non-Gaussian distributions.

**WIP** Mitigation procedures:
* Reduce spread in weights and negative events - testing 4 configs.
* Enlarge batch sizes so the uncertainties in $`\sigma`$ and
  $`\epsilon_\mathrm{filt}`$ are both well-behaved - this is already
  pushed quite far by splitting the production into two LHE &
  shower steps. Specifically, this proceeds by merging multiple LHE files for
  the second step, that is critical for $`\sigma \times
  \epsilon_\mathrm{filt}`$ calculation.
* One will note that computing $`\sigma`$ and
  $`\epsilon_\mathrm{filt}`$ separately is likely a bad idea and one
  may instead try to directly calculate $`\sigma \times
  \epsilon_\mathrm{filt} = \frac{\sum_\mathrm{EVNT} w}{N}`$, which
  has no dependence on $`\sum_\mathrm{LHE} w`$ any more.
* **MEGA WIP**: We are currently exploring if removing a handful of
  outlier events from the LHE sets before showering is a viable way
  forward. In the above-linked production these samples are referred
  to as 'weight cap' (`wgtcap`). The cap proceeds through
  [lheWeightKiller.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950665/lheWeightKiller.py)
  with the minimum abs(weight) set to 0 always and the maximum
  abs(weight) tuned by inspection to the specific slice at a value
  that rejects typically 0.01%-0.1% events that are well beyond the
  'bulk' of the distribution.