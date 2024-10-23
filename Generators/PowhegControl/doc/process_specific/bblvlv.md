# Process-specific information for `bb4l` production


This is largely a stub, collecting some experience from @jkretz from test productions performed:
* https://its.cern.ch/jira/browse/ATLMCPROD-9325
* https://its.cern.ch/jira/browse/ATLMCPROD-9414
* https://its.cern.ch/jira/browse/ATLMCPROD-9931

Extensive validation has been presented in [ATL-PHYS-PUB-2021-042](https://atlas.web.cern.ch/Atlas/GROUPS/PHYSICS/PUBNOTES/ATL-PHYS-PUB-2021-042)

Some list of open issues has been collected in these [Google slides from May 2022](https://docs.google.com/presentation/d/1sGMMBOwQztY7NJOoEQxmqKsfjiT7rJOhdDA1i8hhfkE).

This module generates what is colloquially referred to as ttbar+Wt
with dileptonic decays. Technically also include diagrams that have no
top-quark propagators (diboson WW + bb).  For same-flavour leptons it
should in principle also contain diboson ZZ + bb, but this
contribution is not implemented.  As the full calculation is performed
at NLO including the "top" decays, there is no ambiguity how to remove
the "NLO overlap" between ttbar and Wt. On the other hand the
limitation to (different-flavour) leptonic decays may pose a strong
limitation for analyses.

The baseline job option that should be considered is at https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950535/mc.PhPy8EG_NNPDF30_A14_bb4l_mt172p5_gt1p32_valid.py

A few comments:

## Matching

Matching is to be performed with the special user hook (not the
vanilla Main31!) given in the above job option. The setup goes back to
interactions of Simone Amoroso with Tomas Jezo. It is known that it
disagrees with earlier public code versions and what may be shown in
the (earlier) publications by Tomas or ATLAS.

As of last checks (mid 2022), the setup **does only work in
Pythia8.244/5 and specifically *not* with Pythia8.3**, because the
`Powheg:nFinal = -1` option is only implemented in those specific
releases.

A Pythia8 issue on the topic is pending https://gitlab.com/Pythia8/releases/-/issues/154

Also note that the intricacies of the matching procedure are at least
partially behind two further limitations:
* no 'lepton+jet' or fully hadronic decays available, even though the
  promise is typically 'end of the year'
* no matching in Herwig (7), even though the code supposedly exists somewhere

## General jO parameters

In the above job Options the settings for mTop, GammaTop and PDFs are
synchronised to those of the default ttbar and Wt jO. Note that in
principle `bb4l` would want to calculate GammaTop internally, but it
appears happy enough to take the override value.

## Speed-related options and comments

The folding-parameters were retuned to (2,2,2) which gives an
acceptable negative weight fraction of about 4%.

The parameter
```
PowhegConfig.for_reweighting = 0
```
slows down the event generation, but brings a significant reduction in
spread of weights. Setting the parameter to 1 would apply some correction
as extra weight rather than unweighting it.

While not formally tested, the speedup and the loss in statistical
power per event appeared to be roughly balanced, in which case it is
more beneficial to go for the option that achieves the same
statistical power with less events, i.e. `for_reweighting = 0`.

On the other hand, it may be that integrations have a harder time to
converge, so in case one may opt for `for_reweighting = 1`. See also
specific comments below.

## Weight-related options and comments

Calculating PDF and scale weights takes a very significant fraction of
the time and (if I recall well) does not make use of multi-core
generation. Therefore in some of the test samples the number of PDF
weights was reduced.

In some earlier test version, we tried to enable variations of mTop
and GammaTop as weights
https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950305/mc.PhPy8EG_MENNPDF30_A14_bb4l_llDF_nofilter_fr0_valid.py
By inspection of the samples, it would appear this does not work.

## Lepton flavours

The Powheg code by itself only computes the $\mu^+e^-$ final
state. Other final states are obtained in a separate section of the
code simply by 'relabelling' the lepton flavours. The available
options can be seen
[here](https://gitlab.cern.ch/atlas/athena/-/blob/21.6/Generators/PowhegControl/python/processes/powheg/bblvlv.py#L183-185)
Specifically, the 'maximum' that the default code version produces is
all different-flavour combinations, aka
```
PowhegConfig.decay_mode = "b l+ vl b~ l- vl~"
```

Given the simplicity of the 'relabelling procedure', one may wonder if
the procedure cannot be extended to cover all dilepton final
states. This has indeed been done in https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950540/mc.PhPy8EG_NNPDF30_A14_bb4l_alldilepflav_valid.py

Note that this:
* relies on a 'self-hacked' version of the Powheg code, the two
  changed files are saved in
  [here](./bblvlv)
  and are supposed to replace the files in
  `$POWHEGPATH/POWHEG-BOX-RES/b_bbar_4l/`. The Powheg executable has
  then to be recompiled, you can pick out the required commands from 
  `$POWHEGPATH/Make.sh`. The needed executables and libraries are
  stored together with the integration files, you may check e.g.  
  `/cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/950xxx/950540/mc_13TeV.Ph_bb4l_mt172p5_gt1p32_alldilepflav.GRID.tar.gz`  
  which files are needed and in which structure.
  During production the `$POWHEGPATH` is changed to point to the local
  directory (first line of above jO) to pick up the patched executable.
* it requires some 'trust' that extra diagrams from diboson ZZ + bb
  are not relevant (or at least their interference with the top-quark
  diagrams is not relevant)


## Integrations

Integrations appear a bit touchy - they can take long and they have
the tendency to not complete. So it is advisable to start from setting
that are known to work.

**Expect a run time of about 4 days on 12 CPU cores.** As I had a 64
CPU core node available to myself, I simply launched the integrations
from the command line and waited the 4 days. I believe the rough
sequence of commands would be like:
```
mkdir lovelybb4l
cd lovelybb4l
export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase
source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh
# !really start with 21.6.74!
asetup AthGeneration, 21.6.74,here
# 12 cores
export ATHENA_CORE_NUMBER=12
# get a jO known to work
cp /cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/950xxx/950535/mc.PhPy8EG_NNPDF30_A14_bb4l_mt172p5_gt1p32_valid.py .
Gen_tf.py --ecmEnergy=13000 --jobConfig=$PWD --outputEVNTFile=tmp.EVNT.pool.root --randomSeed=432 --maxEvents=100 >& log&
# wait roughly 4-5 days
# collect integration_grids.tar.gz and rename to sth like mc_13TeV.Ph_bb4l_mt172p5_gt1p32.GRID.tar.gz
# if you have hacked the Powheg executable, you need to: unpack the tarball, put the `POWHEG-BOX-RES/b_bbar_4l` dir in, and retar
```

A few further random memories:
* If you may think changing some of the [integrations
  parameters](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950535/mc.PhPy8EG_NNPDF30_A14_bb4l_mt172p5_gt1p32_valid.py#L34-39)
  or releases is a great idea - rather don't do this at least
  initially. E.g. I wasted weeks after I made a small looking change
  on `itmx1` to a value larger than 2 and that made the integration
  loop.  There may be other parameter changes that lead to jobs never
  converging.
* As remarked above, it may be easier to make the integrations
  converge with `for_reweighting = 1`, although I cannot recall
  testing systematically.
* Make sure to not run too many events / PDF weights, this will just
  lead to a long time needed to finish the job after integration.