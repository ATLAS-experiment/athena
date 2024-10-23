# Process-specific information for `jj` production

This process is for inclusive dijet production at NLO.

This is largely a stub, collecting experience from @jkretz and a few
others from productions performed:
* https://its.cern.ch/jira/browse/ATLMCPROD-8796
* https://its.cern.ch/jira/browse/ATLMCPROD-9019
* https://its.cern.ch/jira/browse/ATLMCPROD-9436
* https://its.cern.ch/jira/browse/ATLMCPROD-9466
* https://its.cern.ch/jira/browse/ATLMCPROD-9497

Related QTs:
* https://its.cern.ch/jira/browse/AGENE-1661 - [Report in CDS](https://cds.cern.ch/record/2718541)
* https://its.cern.ch/jira/browse/AGENE-1759 - [Report in CDS](https://cds.cern.ch/record/2835525)


## Production workflow

As usual for Powheg processes, LHE production and Pythia8/Herwig7
showering are in principle separated. Furthermore, as 'usual' we slice
the showered samples as function of the leading jet pT into JZ1, JZ2,
JZ3, ... (see
[JetFilter_JZX_Fragment.py](https://gitlab.cern.ch/atlas/athena/-/blob/21.6/Generators/GeneratorFilters/share/common/JetFilter_JZX_Fragment.py)
for the definition of the pT boundaries).

The production for this specific process has the challenge that the jet
pT spectrum is steeply falling, negative weight fraction and spread of
weights can be large, and generation speed is rather low (especially
when computing a lot of PDF weights).
Therefore, we opted to separate the LHE production and showering:
* the LHE file production is the most CPU-intense part (and has to be
  run single core, as PDF reweighting will switch into single core mode)
* multiple LHE files are merged for showering and the jet-slicing

A typical set of LHE jO can be considered these:  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600947/mc.Ph_jj_JZ1_opt_v2.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600948/mc.Ph_jj_JZ2_opt_v2.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600720/mc.Ph_jj_JZ3_opt.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600721/mc.Ph_jj_JZ4_opt.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600722/mc.Ph_jj_JZ5_opt.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600723/mc.Ph_jj_JZ6_opt.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600724/mc.Ph_jj_JZ7_opt.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600725/mc.Ph_jj_JZ8incl_opt.py  

A typical set of Pythia8 shower jO are those:  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600949/mc.PhPy8EG_jj_JZ1_v2.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600950/mc.PhPy8EG_jj_JZ2_v2.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600626/mc.PhPy8EG_jj_JZ3.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600627/mc.PhPy8EG_jj_JZ4.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600628/mc.PhPy8EG_jj_JZ5.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600629/mc.PhPy8EG_jj_JZ6.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600630/mc.PhPy8EG_jj_JZ7.py  
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/600xxx/600631/mc.PhPy8EG_jj_JZ8incl.py

Herwig shower jO can be found at [ATLMCPROD-9497](https://its.cern.ch/jira/browse/ATLMCPROD-9497)

Note that in principle the shower veto should extend to the MPIs, as
here (unusually) the hard process and the MPI process are the same. We
explored this in
[ATLMCPROD-9436](https://its.cern.ch/jira/browse/ATLMCPROD-9436) and
you can find plots on this in Sascha's QT report linked above.


## LHE production setup and integrations

To tune the LHE generation, the parameters
```
PowhegConfig.bornktmin = X
PowhegConfig.bornsuppfact =Y
```
are adjusted for each slice separately:
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

These processes will require integrations to be prepared. The
above-mentioned LHE jO have already the 13 TeV integration grids
stored, but any different setup and different sqrt(s) value would need
integrations to be prepared. The existing jO should be a good starting
point and in case no changes other than the sqrt(s) is needed, new
grid tarballs can be registered to the same jO directory.

**Expect a run time of about 0.5-3 days on 16 CPU cores.** As I had a 64
CPU core node available to myself, I simply launched the integrations
from the command line and waited the needed time. I believe the rough
sequence of commands would be like:
```
mkdir jj_JZ3
cd jj_JZ3
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
# I used 21.6.70 for LHE production, probably safe to stay with that one
asetup AthGeneration, 21.6.70,here
# 16 cores
export ATHENA_CORE_NUMBER=16
# get a jO known to work
cp /cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/600xxx/600720/mc.Ph_jj_JZ3_opt.py .
Gen_tf.py --ecmEnergy=13000 --jobConfig=$PWD --outputEVNTFile=tmp.EVNT.pool.root --randomSeed=432 --maxEvents=100 >& log&
# wait roughly 0.5-3 days
# collect integration_grids.tar.gz and rename to sth like mc_13TeV.Ph_jj_JZ3.GRID.tar.gz
```

This would have to be repeated for all desired slices. Note that the
'optimised' JZ1 and JZ2 slices are slowest.



## Shower setup and Cross section * filter efficiency calculation

Matching should be fine in both Pythia8 and Herwig7 using the
'default' procedures, see above links. Note also the caveat mentioned
above, that in principle MPI processes should be vetoed in the
matching, this has at least for Pythia8 been tested and found to bring
only a minor effect (the effect is clearly visible in some event shape
observables, but there is no clear trend that this would improve or
worsen agreement with data.)

Take note how in the above linked shower jO the pT ranges are extended
below/above the nominal range for the JZ1/JZ9 slices.

The setup **may not work with Pythia8.3**, because of the
`Powheg:nFinal = -1` option, but I may be wrong here. Using release
21.6.70 with Pythia8.245 is an option known to work.

In the preparation of the above samples we have observed some
instabilities to obtain the needed cross section times filter
efficiencies ($`\sigma \times \epsilon_\mathrm{filt}`$). This appears
ultimately related to a sometimes significant spread in event weight,
significant negative weight fraction, and finally to the way that AMI
averages these.

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

Mitigation procedures:
* Reduce spread in weights and negative events - here we are already
  at the limit already with the above jO
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
  has no dependence on $`\sum_\mathrm{LHE} w`$ any more. This
  has been effectively explored in Sasha's QT report linked above.
* In the follow-up QT on [the MiNLO `jjj` process](jjj.md) we are
  currently exploring if removing a handful of outlier events from the
  LHE sets is a viable way forward.

We have also seen that Herwig7 showered samples are more susceptible
to these problems.