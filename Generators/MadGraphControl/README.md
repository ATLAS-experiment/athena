# MadGraph5_aMC@NLO for ATLAS

[TOC]

# **Introduction**

Last reviewed: January 2024

This page contains information about running the MadGraph5_aMC@NLO Monte
Carlo generator.  

**Please use the updated reference for this generator: [arXiv:1405.0301](http://arxiv.org/abs/arXiv:1405.0301)**

**NLO corrections in theories other than QCD (eg NLO EW) require the citation of: [arXiv:1804.10017](http://arxiv.org/abs/arXiv:1804.10017)**

These references are part of the [ATLAS-useful.bib](https://gitlab.cern.ch/atlas-physics-office/atlaslatex/-/blob/master/bib/ATLAS-useful.bib) file you are hopefully using for your paper.

If you are using special features like
-  MadSpin
-  loop-induced
-  decay rates (MadWidth)
-  merging at LO or NLO
- ...

consider citing those according as described [here](http://amcatnlo.web.cern.ch/amcatnlo/list_refs.htm)


**ATLAS mailing list** for all questions that cannot be answered by reading this README: <atlas-generators-madgraphcontrol@cern.ch>  

**External MG5_aMC@NLO pages:**  
-  Webpage: <http://amcatnlo.web.cern.ch/amcatnlo/>  
-  Wiki: <https://cp3.irmp.ucl.ac.be/projects/madgraph/>  
-  Problem reporting (and answers): <https://answers.launchpad.net/mg5amcnlo> **Note that if you are
having problems with running MG5_aMC in Athena you should report them on
the ATLAS mailing list above first and _not_ to the launchpad page. We
only send questions to the authors once we are sure the problem is not
with ATLAS software.**
-  Code download: <https://launchpad.net/mg5amcnlo>  

MadGraph5_aMC@NLO has the ability to produce LO and NLO events. The
information provided on these pages will usually discuss LO running and
NLO running separately as there are some significant practical
differences between the two running modes. NOTE: These instructions are
focused on running in **Release >=21**. Older pages for running in
Release19 are available browsing the
[history](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MadGraph5aMCatNLOForAtlas?rev=156)
of the twiki.
For running in Release17, they are available at the bottom of the [twiki](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MadGraph5aMCatNLOForAtlas#Links_to_older_pages_e_g_MC12_re)
but will not be regularly updated.  

The information below gives specific information on several different
ways of running MadGraph5_aMC@NLO. Running within Athena is recommended,
however it is often useful for more detailed studies and validation to
run externally in "StandAlone" mode. Another important consideration is
the matching/merging scheme for samples that are produced with
additional partons. All of this is described in more detail below.

**For a general introduction to MadGraph5_aMC@NLO, see the tutorial
[here](https://indico.cern.ch/event/555228/).**


**Please check the [Software version page](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgMcSoftware#Versions) page
to find out which releases are available, which generator/package
versions they correspond to and which are _NOT recommended_ due to
known issues.**

We generally try to install the newest MadGraph5_aMC@NLO release
available. If you aren't certain of the status of a new release, please
feel free to [write to the list](mailto:atlas-generators-madgraphcontrol@cern.ch). We apply a set
of private patches on top of the standard code which can be found
[here](https://gitlab.cern.ch/sft/lcgcmake/tree/master/generators/patches).

# The Basics (running standalone)


Here are some basic instructions for running MadGraph5_aMC@NLO.
We recommend playing around with the (interactive) standalone version of the code first
if you are not yet familiar with the generator.

## Setup

You can download the code from the [webpage](https://launchpad.net/mg5amcnlo)

Alternatively, when you set up any `AthGeneration` release, an environment variable,
`$MADPATH` will be defined, pointing to the installation of MadGraph
that you are using. This path also appears in the `$PYTHONPATH` in newer
releases, so that if you want to change versions you should remember to
replace `$MADPATH` and modify `$PYTHONPATH`.

New version of MadGraph are provided by the CERN SFT team. Normally they
are requested by experts (feel free to get in touch with us if you would
like a new version deployed or validated). When they build, they apply
[private
patches](https://gitlab.cern.ch/sft/lcgcmake/-/tree/master/generators/patches)
for us; here is the example patch for
[3.4.1](https://gitlab.cern.ch/sft/lcgcmake/-/blob/master/generators/patches/madgraph5amc-3.4.1.atlasTest.patch).


## Syntax

The following section explains some of the basics for defining your
process and includes some additional options like excluding diagrams and
decaying particles.

For learning the basic syntax, start the MG5_aMC interactive shell with

    <PATH_TO_MG5_aMC>/bin/mg5_aMC

Typing

    tutorial

will start a fairly comprehensive interactive tutorial.

### Central processes (LO & NLO)

To generate a LO process you can do something as simple as:

    generate p p > z

The same process at NLO would be:

    generate p p > z [QCD]

### Excluding diagrams and interference

For example if your central process is `p p > e+ e-` including the Z and
gamma\* contribution and interference terms, you would define your
process like this:

    generate p p > e+ e-

Then if you wanted to separate out the z and gamma\* components you have
several options:

-   **Z component**: Require diagrams to have s-channel Z-boson:
    `generate p p > z > e+ e-`
-   **gamma\*** **component**: Exclude **all** diagrams with a photon in
    the ME diagram: `generate p p > e+ e- / z`
-   **gamma\*** **component**: Exclude all diagrams with **s-channel** Z
    bosons in the diagrams: `generate p p > e+ e- $$ z`

**NOTE: The options above are _not_ gauge invariant and remove
diagram interference and can result in unphysical distributions! They
should be avoided if possible.**

  
However, the following options are safe to use:

-   **Z component**: Produce on-shell Z which is then decayed:
    `generate p p > z, z > e+ e-`
-   **gamma\*** **component**: Keep all diagrams but remove events with
    an **on-shell** Z boson: `generate p p > e+ e- $ z`

Note that the above
decay syntax only works for LO generation, for NLO generation MadSpin
must be used to decay particles.

# **Running in Athena**

## Different running modes

There are essentially three ways of producing MadGraph files in the
ATLAS production system.

-   **Running with inputs**: the requester produces LHEF inputs and then 
    showering is done in the official production system
    (this is _not_ the preferred way, as it is not easily reproducable)
-   **On-the-fly (OTF) running**: The inputs are produced inside the
    athena job and then passed to the shower program. In this case
    everything is done in the official production system.
-   **Gridpack running**: Similar to the on-the-fly running except the
    the integration grids are produced privately by the user and these
    are shipped off to the production jobs. From the gridpack event
    generation is run and then passed to for showering - so in this
    regard it is still "on-the-fly"

## Generating LHE files (MadGraphControl)

Generating MG5_aMC@NLO LHE files in Athena requires using the on-the-fly
framework called MadGraphControl. The instructions for this differ
slightly whether you want LO or NLO samples. The "standard" OTF
generation mode for LO and NLO will be described first and then
"gridpack mode" running (which is common to both LO and NLO) is
described later. Also running with LHAPDF and MadSpin as common to both
LO and NLO so these are also described later.

### Athena setup

One should first setup a recent 23.6 or 24.6 AthGeneration release. It
is always best practice to start from a setup that you know worked. At
the moment, the most recent release 23.6 caches are a good starting
place if you aren't sure.  
**Also check [PmgMcSoftware Versions](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgMcSoftware#Versions) to be sure that a given release is suitable for your needs**

    cd <workdir>
    asetup 2X.6.Y,AthGeneration
    mkdir run/
    cd run/


### Example Job Options and Local Testing

Some example JOs can be found
[here](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/950xxx/),
starting with DSID 950100.

To test your private job option, copy it into a local folder and (after
setting up a release) run it with a command like the one below.

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=/path/to/my/JOfolder

Only one JO is allowed per JO folder.

### LO on-the-fly generation

To use
[mc.MG_ttbar_LHEtest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/950xxx/950100/mc.MG_ttbar_LHEtest_valid.py)
jobOptions for testing, one can run the following command:

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950100

Note: This JO is purely for testing, one should always set PDF and
systematics via a base fragment, see below.

### NLO on-the-fly generation

Then to use
[mc.aMC_ttbar_LHEtest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/tree/master/950xxx/950101/mc.aMC_ttbar_LHEtest_valid.py)
jobOptions for testing, one can run the following command:

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950101

Note: This JO is purely for testing, one should always set PDF and
systematics via a base fragment, see below.

### PDF usage

In general, LHAPDF should be used rather than the internal MG5_aMC@NLO
PDFs, they are likely to be more regularly updated and versioned. The
list of all available LHAPDF6 PDF sets can be found
[here](http://lhapdf.hepforge.org/pdfsets.html).

Unless there are good reasons against it, the PMG recommendations, which
are based on the PDF4LHC recommendations, should be used. They can
easily be implemented as described in the Systematics section below and
do not require reading this section.

### Systematic event weights

It is possible to get scale and PDF
variations via event weights. Setting this up is simple examples for LO
and NLO running are provided below. 

#### PDF and systematics via Base fragment include

Central PDF and weights that allow to estimate PDF, alphaS, and muR/muF
uncertainties at LO and NLO according to the [PMG recommendations](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgSystematicUncertaintyRecipes) and the PDF4LHC
prescription can be easily set by including a PDF base fragment.

It only requires adding

    import MadGraphControl.MadGraph_NNPDF30NLO_Base_Fragment

at the top of the job option, where `NNPDF30NLO`
corresponds to the central PDF and additional pdfs are included as
weights.

Alternative base framgents are defined [here](python/)

If the default base fragments can not be used, a user defined
base frament can created used, with the following structure:

    import MadGraphControl.MadGraphUtils
    MadGraphControl.MadGraphUtils.MADGRAPH_PDFSETTING={
        'central_pdf':90400, # the lhapf id of the central pdf, see https://lhapdf.hepforge.org/pdfsets
        'pdf_variations':[260000,90400], # pdfs for which all variations (error sets) will be included as weights
        'alternative_pdfs':[266000,265000,13100,25200], # pdfs for which only the central set will be included as weights
        'scale_variations':[0.5,1.,2.], # variations of muR and muF wrt the central scale, all combinations of muF and muR will be evaluated
    }

An example JO with base fragment can be found here:
[mc.MG_ttbar_BaseFragmentTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950103/mc.MG_ttbar_BaseFragmentTest_valid.py)
and can be run with

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950103

#### Manual PDF setting, LO example

If you want to add PDFs beyond the usual recommendation, as central PDFs
or as weights, they can be configured as in the examples below:

For a LO example see the jobOptions
[mc.MG_ttbar_SystTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950105/mc.MG_ttbar_SystTest_valid.py)
and run:

     
    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950105

#### Manual PDF setting, NLO example

For a NLO example see the jobOptions
[mc.aMC_ttbar_SystTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950106/mc.aMC_ttbar_SystTest_valid.py)
and run:

     
    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950106

### Event weights from re-weighting

MG5_aMC allows the use of re-weighting to derive rather complex weights
for downstream analysis usage. The use of the reweighting module is
explained at MadGraph5aMCatNLOreweight. Make sure
that your reweighting includes the `--rwgt_info` option, which sets a
name for the weight that will be propagated automatically through the
software. For example:

    launch --rwgt_info=aewm1_100
    set aewm1 100 

Sets the weight name for this weight to `aewm1_100`. Weight names may
only contain letters, digits, hyphens, and underscores.

#### Checking weight names

You can check if the weights made it into your LHE file by following
the instructions [here](doc/checkweights.md)

#### Systematics extras for SUSY event generation

For job configurations using the SUSY pre- and post-include fragments,
matching scale variations are straightforward to set up. You can either
include `_msup` or `_msdw` in the physics short of your job config,
or in your job config you can set `syst_mod='ms_up'`, for example, to
run an upward variation of the matching scale. The variation is by
a factor of two from the nominal.

Note that based on guidance in the MG5 documentation, the matching scale
is set by default to 1/4 of the heavy mass being generated in the SUSY
events. It is bounded above at 500 GeV, and bounded below at 15 GeV.

### SM parameters

It is possible to set default parameters using [MadGraphParamHelpers](https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/MadGraphControl/python/MadGraphParamHelpers.py)	

### MadSpin usage

MadSpin can be used for decaying heavy resonances produced at both LO
and NLO. One need to generate the `madspin_card.dat` on-the-fly in the
jobOptions like this:

    madspin_card=process_dir+'/Cards/madspin_card.dat'
    if os.access(madspin_card,os.R_OK):
        os.unlink(madspin_card)
    mscard = open(madspin_card,'w')                                                                                                                                      
    mscard.write("""#************************************************************
    #*                        MadSpin                           *
    #*                                                          *
    #*    P. Artoisenet, R. Frederix, R. Rietkerk, O. Mattelaer *
    #*                                                          *
    #*    Part of the MadGraph5_aMC@NLO Framework:              *
    #*    The MadGraph5_aMC@NLO Development Team - Find us at   *
    #*    https://server06.fynu.ucl.ac.be/projects/madgraph     *
    #*                                                          *
    #************************************************************
    #Some options (uncomment to apply)
    #
    # set seed 1
    # set Nevents_for_max_weigth 75 # number of events for the estimate of the max. weight
    # set BW_cut 15                # cut on how far the particle can be off-shell
     set max_weight_ps_point 400  # number of PS to estimate the maximum for each event
    #
    set seed %i
    # specify the decay for the final state particles
    decay t > w+ b, w+ > all all
    decay t~ > w- b~, w- > all all
    decay w+ > all all
    decay w- > all all
    decay z > all all
    # running the actual code
    launch"""%runArgs.randomSeed)
    mscard.close()


For a LO example see the jobOptions
[mc.MG_ttbar_MadSpinTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950107/mc.MG_ttbar_MadSpinTest_valid.py)
and run

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950107

For a NLO example (the syntax is the same as for LO) see the jobOptions
[mc.aMC_ttbar_MadSpinTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950108/mc.aMC_ttbar_MadSpinTest_valid.py)
and run

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950108

Things are a little more complicated when running NLO gridpacks, see an
example below.

**Warning: If branching ratios are
not set explicitly, MadGraph will calculate them at LO. Branching ratios
are specified in the DECAY block of the param_card, see the example
below.**

    ## The syntax is:
    ## DECAY ID WIDTH
    ##      NDA ID1 ID2 ...
    ## For example:
    DECAY -24 2.085000e+00
          3.377e-01 2 1 -2
          3.377e-01 2 3 -4
          1.082e-01 2 11 -12
          1.082e-01 2 13 -14
          1.082e-01 2 15 -16


### How to use MadSTR?


MadSTR package is a collection of techniques that are used for Diagram Removal and Diagram Subtraction. Here STR stands for *Simplified Treatment of Resonances*. 

Consider the $pp\rightarrow t\bar{t}$ production at Leading Order. Now also consider the  Next-to-leading Order in QCD for the $pp\rightarrow tW$ process. Here, $tW$ process at NLO interferes with $t\bar{t}$ and leads to double counting. The motivation for using MadSTR package is to avoid this double counting by systematically removing amplitudes related to diagrams that overlap.   


**How to Run the Code and some examples**


Step 1: Software Setup 

```
setupATLAS
asetup AthGeneration,23.6.34
```

This sets up AthGeneration to 23.6.34 branch where the working version of MadGraph+MadSTR are installed (Any later release of AthGeneration should work as well). 


The examples includes two scripts  (1) [MadSTR](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/421xxx/421498/mc.MGPy8_tW_madstr.py) which runs with istr parameter set to 1 and does not include MadSpin. For different istr parameters, the user can edit the `settings` parameter. (2) [MadSpin+MadSTR](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/421xxx/421499/mc.aMCPy8_tW_MadSpin_MadStr.py) script runs for istr=1 including MadSpin usage. 


Command line to execute the script- 

```python
Gen_tf.py --ecmEnergy=13600. \
        --maxEvents=100 \
        --randomSeed=123456 \
        --outputEVNTFile=evgen.root \
        --jobConfig=421498
```


### Gridpack mode

When generating complicated processes, especially at NLO,
a lot of time is required for the initial stage of the  matrix element
calculation, including the integration.
A significant amount of grid CPU time for these processes can be saved using gridpacks.
Using gridpacks, the initial calcuation will be done only once and is distributed to the grid, where it is re-used by each job.

General information and an introduction to gridpacks in MadGraph can be
found
[here](https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/IntroGrid).

Gridpack running functionality is incorporated into MadGraphControl for
both the LO and NLO cases. It involves changes to the "generate"
function wrt usual OTF running.

The basic idea is that generation can now be split up into two stages:

1\) Generate the gridpack

-   This performs the matrix element calculation - you only ever have to
    do this once (not once for every 5000 event subjob) - hence saving
    grid CPU time.
-   The resulting gridpack packages everything up to be sent to the grid
    for the event generation step.
-   **This should be run locally** (but with same athena setup as grid
    jobs) - hence overcoming the grid 24 hour limit - and it is even
    possible to parallelize the calculation, i.e. send multiple parallel
    jobs to your local cluster.

2\) Generate the events

-   You can then send jobs to the grid with the gridpack attached as the
    "inputGenConfFile" to generate the events and pass them though
    Pythia etc.
-   All you need to tell the gridpack is how many events you want and
    what random seed to use.

#### Mind the Operating System for official MC production

Production jobs are now run in Centos7 (and eventually will be run on Alma9).
Previous releases (before around 21.6.50) were run in slc6.
There may be some incompatabilities between
these releases for gridpacks, which can contain pre-compiled code.
Be careful that you generate the gridpack on the appropriate architecture.
If you are working, e.g., in an slc6 release, you consistently use
an slc6 container (`lxplus6` or `setupATLAS -c slc6`).

#### LO example

Attached are some example jobOptions for generating ttbar at LO with
LHAPDF and MadSpin via gridpack mode
([mc.MG_ttbar_gridpackTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950109/mc.MG_ttbar_gridpackTest_valid.py)).
This one jobOptions file can be used to generate both the gridpack and
the events. The jobOptions decide which of the two steps to perform
based on whether there is an inputGenConfFile argument passed in the
Gen_tf.py command or not.

These commands explain how to generate the gridpack and then how to use
it to generate events.

To generate the gridpack execute the following command:

    mkdir run_makeGridpack 
    cd run_makeGridpack 
    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=950109 --outputFileValidation=False

At the end of the the gridpack generation you will see:

     
    "RuntimeError: Gridpack sucessfully created, exiting the transform. IGNORE ERRORS if running gridpack generation!"

This is just because it's hard to exit the Generate_tf.py quietly
without running the shower, please ignore!

  
Then, to generate events using a previously generated gridpack execute
the following command:

    cd ../
    mkdir run_generateFromGridpack
    cd run_generateFromGridpack

    # In order to test out the new gridpack, we must run off of local files
    cp -r /cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/950xxx/950109/ .
    # Move the gridpack tarball into place
    cp ../run_makeGridpack/mc*.tar.gz 950109/

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=./950109

This will now fail because we are not running the shower.

#### NLO example

Included are some example jobOptions for generating ttbar at NLO with
LHAPDF and MadSpin via gridpack mode
([mc.aMC_ttbar_gridpackTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950110/mc.aMC_ttbar_gridpackTest_valid.py)
). This one jobOptions file can be used to generate both the gridpack
and the events. The jobOptions decide which of the two steps to perform
based on whether there is an inputGenConfFile argument passed in the
Generate_trf.py command or not.

These commands explain how to generate the gridpack and then how to use
it to generate events.

To generate the gridpack execute the following command:

    mkdir run_makeGridpack
    cd run_makeGridpack

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=950110 --outputFileValidation=False

At the end of the the gridpack generation you will see:

     
    "RuntimeError: Gridpack sucessfully created, exiting the transform. IGNORE ERRORS if running gridpack generation!"

This is just because it's hard to exit the Generate_tf.py quietly
without running the shower, please ignore!

  
Then, to generate events using a previously generated gridpack execute
the following command:

    cd ../
    mkdir run_generateFromGridpack
    cd run_generateFromGridpack

    # In order to test out the new gridpack, we must run off of local files
    cp -r /cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/950xxx/950110/ .
    # Move the gridpack tarball into place
    cp ../run_makeGridpack/mc*.tar.gz 950110/

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=./950110


#### Tidying gridpacks after creation

Gridpacks generated with MadGraph come with a number of files that
aren't really needed in production. If you generate the gridpack with
`MadGraphControl`, we attempt to clean up the gridpack for you. Rough
command line equivalents of what we currently use for LO and NLO are:

    tar cvzf ../${gridpack_name} --exclude=lib/PDFsets --exclude=SubProcesses/P*/G*/*_results.dat --exclude=SubProcesses/P*/G*/*.log --exclude=SubProcesses/P*/G*/*.txt .

and:

    tar czf ${gridpack_name} ${MADGRAPH_GRIDPACK_LOCATION} --exclude=lib/PDFsets --exclude=Events/*/*events*gz --exclude=SubProcesses/P*/G*/log*txt --exclude=SubProcesses/P*/G*/events.lhe* --exclude=*/*.o --exclude=*/*/*.o --exclude=*/*/*/*.o --exclude=*/*/*/*/*.o

Some further improvements should be possible (e.g. excluding `.ps`
files). These can be seen within `MadGraphControl`
[here](https://gitlab.cern.ch/atlas/athena/-/blob/master/Generators/MadGraphControl/python/MadGraphUtils.py#L554)
and
[here](https://gitlab.cern.ch/atlas/athena/-/blob/master/Generators/MadGraphControl/python/MadGraphUtils.py#L568).

### Running in cluster (parallel) mode

The jobOption
[mc.aMC_ttbar_gridpackClusterTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950111/mc.aMC_ttbar_gridpackClusterTest_valid.py)
can be used to generate a NLO gridpack (see above) using the CERN
lxbatch (lsf) cluster. The relevant extra parameters are:

    cluster_type="condor"
    cluster_queue="workday"

these are set in the config card before running

    modify_config_card(process_dir=process_dir,settings={'cluster_type':'condor','cluster_queue':'workday'})

To generate the gridpack execute the following command:

     
    mkdir run_makeGridpackCluster
    cd run_makeGridpackCluster
    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=950111 --outputFileValidation=False

At the end of the the gridpack generation you will see:


    "RuntimeError: Gridpack sucessfully created, exiting the transform. IGNORE ERRORS if running gridpack generation!"

This is just because it's hard to exit the Generate_tf.py quietly
without running the shower, please ignore!

#### Recovering a failed parallel run at LO

Currently it is rather hard to recover a run where only a small subset
of subjobs failed. Here are some tips to try to help with these kinds of
problems.

Firstly one can play with the parameters in the configuration card to
attempt more retries:

-   cluster_nb_retry: How to deal with failed submission (can occurs on
    cluster mode) - 0: crash, -1: print error, hangs the program up to
    manual instructions, N(>0) retry up to N times.
-   cluster_retry_wait: How much time to wait for the output file before
    resubmission/crash (filesystem can be very slow)

Secondly if this still doesn't work there is still a small amount of
hope... the procedure to attempt is the following: 1 Locate missing
subjobs and execute commands to run these jobs by hand. In practise this
is nearly impossible without hacking the `cluster.py` file in MG5_aMC.
We are contacting the authors to see if this can be improved... 1 Run
missing subjobs by hand 1 Create gridpack by hand

-   LO mode:


    PROC_sm-no_b_mass_0/bin/madevent
    PROC_sm-no_b_mass_0> combine_events
    PROC_sm-no_b_mass_0> store_events
    PROC_sm-no_b_mass_0> create_gridpack

-   NLO mode: Just tar the PROC directory.

### Running in multi-core mode

You can use multi-core MadGraph by doing

    export ATHENA_CORE_NUMBER=8

before running athena. Please keep in mind that this triggers the use of
8 cores, which while perfectly allowed interactively is not allowed on
the grid unless multi-core resources are **explicitly** requested in the
production request - in which case that variable will be automatically
exported and set to the number of cores your job is assigned to. You
will need to return to "serial" mode for showering (Pythia cannot handle
multi-core mode), like this:

    # Helper for resetting process number
    from MadGraphControl.MadGraphUtils import check_reset_proc_number
    check_reset_proc_number(opts)

An example implementation is provided in
[mc.MGPy8EG_ttbar_MCOREtest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950112/mc.MGPy8EG_ttbar_MCOREtest_valid.py)
jobOptions for testing, one can run the following command:

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=950112

#### Recovering a failed multicore run at NLO

Some ad-hoc recipes for recovering large scale multicore integrations
was developed during the Run 3 V+jets FxFx production. See these
[slides](https://indico.cern.ch/event/1211764/contributions/5097386/attachments/2529478/4351685/FxVjets_int.pdf)
for details.

### Using the Reweight module

Method consists in using a sample of events generated under a certain
theoretical hypothesis and in associating with those events an
additional weight that corresponds to a new theoretical hypothesis. It
is described in more detail [here](doc/reweight.md).
Ultimately, as is the case for MadSpin running, the appropriate reweight card needs to be placed in the process directory.

#### LO example

For an LO example see the jobOptions
[mc.MG_ttbar_ReweightTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950113/mc.MG_ttbar_ReweightTest_valid.py)
and run:


    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950113

#### NLO example

For an NLO example see the jobOptions
[mc.aMC_ttbar_ReweightTest_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950114/mc.aMC_ttbar_ReweightTest_valid.py)
and run:

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputTXTFile=test_lhe_events --jobConfig=950114

### Using the Bias module

The bias module allows to introduce a bias function that affects the
distribution of generated events. It is described
[here](https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/LOEventGenerationBias).

### Adding lifetimes

If you need to add lifetimes to some of the particles in your LHE file,
there are two mechanisms to do so. The first and easiest is to set
`time_of_flight` in your run card (the units are mm for the minimum time
of flight to be added); this can also be done by passing
`time_of_flight` in the extras dictionary to `build_run_card()`. The
second is to use the separate function:

    add_lifetimes(process_dir=a_proc_dir,threshold=1E-2)

Note that this function must be called **after** `generate` and
**before** `arrange_output` in order to take effect.

## Showering LHE files

It is possible to shower LO files with Pythia8 and it is possible to
shower NLO with either Pythia8 or Herwig7. Instructions for generating
OTF MadGraph events and then showering them are given below.

Note that when matching to a parton shower some events generated by MadGraph
will be vetoed by the parton shower, for various reasons.
In order to account for this inefficiency you should
always ask MadGraph to generate at least 10% more events than what you want to
get out from in the end. This is to ensure that the the job doesn't "run
out of events" in central production, causing the grid jobs to crash for
instance.

You can generate samples and let the shower take care of all additional
QCD radiation inclusively or generate samples with multiple additional
partons in the matrix element and then apply a merging/matching
procedure to remove overlaps. Instructions for both cases are outlined
in the following.

### LO Pythia8 Showering (inclusive)

To generate and shower LO MadGraph inclusively (i.e. simply adding radition generated with Pythia to the core process) use the following
[mc.aMCPy8EG_A14NNPDF23_ttbar_Incl_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950116/mc.MGPy8EG_A14NNPDF23_ttbar_Incl_valid.py)
jobOptions. This includes adding an extra line from MadGraphControl to
move the MadGraph output into the right naming convention and format
(`arrange_output()`) and then adding a few standard lines for the the
metadata and configuration and then finally including the relevant
Pythia8 shower jobOption fragments:


    #### Shower                                                                                                                                                             
    include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
    include("Pythia8_i/Pythia8_MadGraph.py")

Run with the usual Generate_tf.py command:

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=950116

If you are starting from external LHE files the above snippet can also
be used, one simply needs to replace the `runName` variable with a
string that is the "physics short" part of the LHE dataset name.
**IMPORTANT FOR OFFICIAL PRODUCTION:** If you are using external inputs
you need to add the following line in your top jobOptions to tell the
production system where to check for the LHE files -
`evgenConfig.inputfilecheck\="physics short"`

Note that Pythia can also decay unstable particles but it will not take into account spin correlations, so that MadGraph decays (or simply generating the full process, e.g., l+ l- instead of Z) should be used where possible.

### LO Pythia8 Showering with CKKW-L kT-Merging

Merging allows to combine samples with different jet-multplicity at matrix element level, e.g., Z+0jet and Z+1jet. This

    generate p p > l+ l-
    add process p p > l+ l- j

will generate a sample that contains a mixture of events with and without jets (jets are always defined with kinematic cuts, most importantly on ptj), according to the respective cross-sections
Generating the extra jet at matrix element level can improve the modelling, in particular of jets and the recoil of the remaining system against jets (it is part of the NLO correction).
However, we still need to add soft jets, whose modelling at matrix element level would not appropriate (in fact the ME diverges if we set ptj->0).
Furthermore, if we naively pass the above mixture of events to a parton shower, the parton shower will also extra jets. The `p p > l+ l-` would thus also become events with extra jets, leading to a double counting with `l+ l- j`

A merging procedure is thus required to define which parts of the phase space are modelled with parton shower (generally those with soft jets) and which with the matrix element generator (i.e. MadGraph).
If configured correctly, this improves the modelling of hard jets (and the recoiling particles), combining the advantages of matrix element generator (good for hard jets) and parton shower (needed for soft jets).

**The first thing you should do before trying to setup a new
CKKW-L-merged sample is look at the CKKW-L merging section of the
Pythia8 manual
[here](https://pythia.org/latest-manual/CKKWLMerging.html),
where the documentation is quite thorough.**

CKKW-L is the MG5_aMC+Py8 LO-multileg merging scheme of choice for Run2.
This is an update wrt MLM.
MLM should be used for loop-induced processes only, for which CKKW-L is not appropriate.

CKKW-L is not supported by the ickkw switches
like MLM, so you have to be a bit more careful with the settings. You
should **not** set xqcut at all with CKKW-L merging (if you set xqcut a
reasonable amount lower than ktdurham, then it’s unlikely to have any
affect, but to be safe, just keep xqcut unset).

Or these jobOptions for the combined Np0-Np1 all in one sample:
[mc.MGPy8EG_A14NNPDF23_ttbar_Merge_Np01_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950117/mc.MGPy8EG_A14NNPDF23_ttbar_Merge_Np01_valid.py)  

This requires changing a few parameters in the MadGraph `run_card.dat`,
e.g.:

    0 = ickkw
    0.0 = drjj
    30 = ktdurham
    0.4 = dparameter

and adding a number of arguments to the Pythia8 command:

    PYTHIA8_nJetMax=nJetMax
    PYTHIA8_Dparameter=dparameter
    PYTHIA8_Process=process
    PYTHIA8_TMS=ktdurham
    PYTHIA8_nQuarksMerge=4
    include("Pythia8_i/Pythia8_CKKWL_kTMerge.py")

Where `nJetMax` is the maximum number of jets in the matrix element (= 2
in example) , `process` is the hard process without additional partons
(= "pp>tt\~" in example), `ktdurham` is the same as in the MadGraph
`run_card.dat` (= 30 GeV in example) and `dparameter` is the same as in
the MadGraph `run_card.dat` (= 0.4 in example).

Run with the usual Gen_tf.py command:

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=950117


### NLO Pythia8 Showering

The inclusive Pythia8 showering settings are described in this section
for NLO matrix element samples. 

####  NLO Pythia8 Showering Inclusive samples

To generate NLO MadGraph5_aMC@NLO inclusively and shower with Pythia8
use the
[mc.aMCPy8EG_A14NNPDF23_ttbar_Incl_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/blob/master/950xxx/950118/mc.aMCPy8EG_A14NNPDF23_ttbar_Incl_valid.py)
jobOptions. This includes adding an extra line from MadGraphControl to
move the LHE output into the right naming convention and format
(`arrange_output()`) and then adding a few standard lines for the the
metadata and configuration and then finally including the relevant
Pythia8 shower jobOption fragments:

    include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")                                                                                                                  
    include("Pythia8_i/Pythia8_aMcAtNlo.py")  

Note that `Pythia8_aMcAtNlo.py` includes the author recommended setting
(from [here](http://amcatnlo.web.cern.ch/amcatnlo/list_detailed2.htm#showersettings)):

Run with the usual Generate_tf.py command:

    Gen_tf.py --ecmEnergy=13000. --maxEvents=-1 --firstEvent=1 --randomSeed=123456 --outputEVNTFile=EVNT.root --jobConfig=950118

**For the decay of resonances (which we have in most processes) the use of matrix element corrections, which are implemented in [Pythia8_aMcAtNlo_decayMEC.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/Generators/Pythia8_i/share/common/Pythia8_aMcAtNlo_decayMEC.py), might be more impropriate, see [2308.06389](https://arxiv.org/abs/2308.06389) .** This configuration should then be used instead of `Pythia8_aMcAtNlo.py`.

### NLO merged samples with FxFx merging

FxFx merging is the merging-scheme of choice for NLO samples. It is described [here](http://amcatnlo.web.cern.ch/amcatnlo/FxFx_merging.htm).

An example job option can be found here:

[mc.MGPy8EG_Zll_FxFx_1jets_valid.py](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions/-/blob/master/950xxx/950632/mc.MGPy8EG_Zll_FxFx_1jets_valid.py)

As for LO merging, both MG5_aMC@NLO and the parton shower need to be configured.
In MG we set:

    'ickkw'         : 3,
    'jetradius'     : 1.0,
    'jetalgo'       : 1, # this is the default, so this parameter is sometimes omitted
    'ptj'           : 8, # Needs to be <=qCut/2. Note that this is currently hard-coded to 8 in the Pythia config
    'etaj'          : 10,

While we need to tell Pythia about the max jet multiplicity (in addition to the core process):

    PYTHIA8_nJetMax=nJetMax
    PYTHIA8_qCut=qCut

The appropriate Pythia configuration is set in [Pythia8_FxFx_A14mod.py](https://gitlab.cern.ch/atlas/athena/-/blob/21.6/Generators/Pythia8_i/share/common/Pythia8_FxFx_A14mod.py)

*Note that both qCutME (which should correspond to the MG ptj cut) and nQmatch (4 or 5 flavour scheme) are currently hardcoded and need to be updated, consistent with the MG values, if different values are to be used*


### LO Herwig7 Showering

LO showering with Herwig in Athena is in principle straightforward. Instead of the Pythia includes, we include the ones for the Herwig parton shower.
Please note that the Herwig knowledge in ATLAS is not at the same level as for Pythia and some things are in flux, so it's good to double check the most recent recommendation with the Herwig experts.

    # Include ATLAS config   
    include("Herwig7_i/Herwig72_LHEF.py")
    # configure Herwig7
    Herwig7Config.me_pdf_commands(order="LO", name="NNPDF30_nlo_as_0118")
    Herwig7Config.tune_commands()
    Herwig7Config.lhef_mg5amc_commands(lhe_filename="tmp_LHE_events.events", me_pdf_order="NLO")
    # add EvtGen
    include("Herwig7_i/Herwig71_EvtGen.py")
    # run
    Herwig7Config.run()

### NLO Herwig7 Showering


For NLO (and NLO only) this includes changing one parameter in `run_card.dat`, setting

    parton_shower = 'HERWIGPP'

and thena adding the parton shower similar to LO running:

    # initialize Herwig7 generator configuration for showering of LHE files
    include("Herwig7_i/Herwig72_LHEF.py")
    # configure Herwig7
    Herwig7Config.me_pdf_commands(order="NLO", name="NNPDF30_nlo_as_0118")
    Herwig7Config.tune_commands()
    Herwig7Config.lhef_mg5amc_commands(lhe_filename="tmp_LHE_events.events", me_pdf_order="NLO")
    # add EvtGen
    include("Herwig7_i/Herwig71_EvtGen.py")
    # run
    Herwig7Config.run()

## Extra models

A list of available models with documentation is
[here](https://gitlab.cern.ch/atlas-generators-team/MadGraphModels/blob/master/model_list.txt).
The models themselves are
[here](http://gitlab.cern.ch/atlas-generators-team/MadGraphModels). If
you would like to request a new model, please open a merge request in
the
[MadGraphModels](https://gitlab.cern.ch/atlas-generators-team/MadGraphModels)
repository in GIT. To keep the repo tidy, we follow a
fork-and-merge-upstream model like athena; you can follow the
[instructions
here](https://atlassoftwaredocs.web.cern.ch/athena/git/gitlab-fork/).
Note that every model needs a corresponding info `.txt` file. These
should include a link to the model, a description, and links to the
relevant websites or papers describing the model. Please include the
model origin (who you got the code from, or how it was prepared) so that
proper attribution can be given by those using the model. 

When you request a new model, it is installed into
`/cvmfs/atlas.cern.ch/repo/sw/Generators/madgraph/models/latest` . At
that point, it will be available for you to run (this distribution
usually takes an hour or so). There is no need to wait for a new
production cache before submitting your request.

To test locally before requesting installation, put your model into a
directory other than the one in which you are running the job, and do:

    export PYTHONPATH=/path/to/my/model:$PYTHONPATH

The job should then be able to pick up your model.

## Guidelines for official samples

### JobOptions structure

ATLAS job options can be found in [this repository](https://gitlab.cern.ch/atlas-physics/pmg/mcjoboptions).
They are located in a folder whose name is the dataset ID (DSID).

In the case of larger signal grids that might differ, e.g., only in the mass of a particle,
the "top" jobOptions file (e.g. `mc.MGPy8EG_myProcess1.py`)
should not contain all the gory details but instead should 
include a "control" jobOptions file (e.g.`include('MadGraphControlMadGraphControl_myProcesses.py')`).
This file is added to one of the job option directories and linked from the other ones.
It is recommended that the top job option is configured based on its name,
which can be used like so:

      from MadGraphUtilsHelpers import get_physics_short
      phys_short=get_physics_short() # e.g. MGPy8EG_myProcess1

Particularly for signal requests, that allows quick additions of new mass points without
any modification to the central file. It also helps ensure that the
physics short of the job options captures the important part of the
process definition.


### Configuration Cards

We strongly encourage you to build your param and run
cards on the fly based on the default cards. We have found that this is
much less error prone in terms of hidden behaviors and unknown
parameters. To do that, you can use the `modify_param_card` and
`modify_run_card` functions in MadGraphControl. If you wish to include a
restriction card (very similar, conceptually, to a template param card,
but included directly with the model and thus more performant),
that can also be done. If the model is external to MG5_aMC (e.g. not `loop_sm`),
then you can open a merge request in
[MadGraphModels](https://gitlab.cern.ch/atlas-generators-team/MadGraphModels).
If the model is internal to MG5_aMC, then please open an
[AGENE](https://its.cern.ch/jira/browse/AGENE) ticket, and we will work
with the Genser team to get the restrict card installed. For restrict
cards in external models, generally the card can be available in a few
hours; for restrict cards in models within MG5_aMC we require a new
release to be built.

### Naming conventions for official production

#### (Top) JobOption names

As MG5_aMC@NLO can run in both LO and NLO modes the naming convention
for samples is not entirely obvious. So we make the choice to use the
following convention:

-   LO process (MadGraph or MG5_aMC@NLO) = "MadGraph" or short "MG"
-   NLO process (aMC@NLO or MG5_aMC@NLO) = "aMcAtNlo" or short "aMC"

This convention should be maintained at every stage of the MC production
chain - i.e LHE file inputs, JOs, evgenConfig.generators, etc.  
But it should be noted that the generator should still formally be
referred to as MadGraph5_aMC@NLO, the sample naming convention is for
our internal ATLAS convenience, to easily distinguish between LO and NLO
samples.

For example `mc.MGPy8EG_A14NNPDF23LO_ttbar_Merge_valid.py`.

#### Control JobOptions

Control jobOptions should follow the naming convention
`MadGraphControl_<PHYSICS NAME>.py`, note that the start of the name
should be `MadGraphControl` regardless of whether the process is LO or
NLO.

It is recommended to avoid if at all possible copy-pasting of code and
'dictionary-style' lists of DSIDs and how they map to models. Far better
is to encode the relevant options in the job options name, and then
decode the options in the control job options file. Particularly for
signal requests, that allows quick additions of new mass points without
any modification to the central file. It also helps ensure that the
physics short of the job options captures the important part of the
process definition.

#### Card files

If you need a specialised run_card.dat or param_card.dat for your
process the naming convention should be as follows:

    [MadGraph/aMcAtNlo]_[run/param]_card_<PHYSICS NAME>.dat 

e.g. MadGraph_run_card_pMSSM.dat

Where the choice of `MadGraph` vs `aMcAtNlo` is defined above.

#### SLHA files

The naming convention for SLHA files should be for e.g. SUSY group
samples:

    susy.<DSID>.<parameters>.slha 


## Feature requests

If you would like to request new features in MadGraphControl please send
an email to the mailing list <atlas-generators-madgraphcontrol@cern.ch>
or directly create an AGENE JIRA ticket
[here](https://its.cern.ch/jira/browse/AGENE/) **and assign the "Epic
Link" to AGENE-987**.

  

# **Contact persons**


**MadGraphControl** <atlas-generators-madgraphcontrol@cern.ch>

-   Zach Marshall
-   Hannes Mildner

**<MadGraph5_aMC@NLO> - Next-to-leading order also**

-   Dominic Hirschbuehl
-   Francesco Giuli

**<MadGraph5_aMC@NLO> - SUSY(control) and BSM models mainly**

-   Zach Marshall

  

# **FAQ**

## Differences between MG5_aMC 3.X and 2.X

3.X supports various new features, most importantly EW loops and the full EW (and EW+QCD) NLO expansion.
For simple NLO QCD proceses some differences are also expected.

3.X allows mixed nlo expansions (https://arxiv.org/pdf/1804.10017.pdf), which is by default activated with nlo_mixed_expansion=True.
For some EW processes (single top production, VBF production) this can lead to different diagrams being generated in v2 and v3.
This can lead to "poles do not cancel" (e.g. for some single top processes) and take a long time for these proceses but is *actually* NLO, in contast to the old syntax, which is only an approximation of the full NLO.

Another difference is coupling orders, if we now set one but not all coupling orders, higher orders are potentially included up to order 99 (if they lead to the final state of course).
Example:

    MG5_aMC>generate p p > z j j  QCD=0 [QCD]
    QED order is missing in the process definition. It will be set to "default unser couplings": 99
    If this is not what you need, please regenerate with the correct orders.
    Order QCD is not constrained as squared_orders. Using: QCD^2=0
    Order QED is not constrained as squared_orders. Using: QED^2=198

This can lead to surprises when, e.g., new physics couplings are non-zero for job option that had zero new physics in MG 2.X releases.

## Fail to reach target number of events

If the desired number of events cannot be generated, this hints at problems with the integration.

Please follow one of the following suggestions to fix the issue:
-  set in the run_card.dat 'sde_strategy' to 2
-  set in the run_card.dat  'hard_survey' to 1 or 2
-  reduce the number of requested events (if set too high)
-  check that you do not have -integrable- singularity in your amplitude

Singularities are generally present if we allow for soft (pt->0) or collinear (dr->0) particles.
Make sure that, for massless particles, the relevant ptj, drjj, drll, dral, etc cuts are set.
For photons, Frixione isolation should be used at NLO (at least to isolate from jets but one can also use it to separate leptons from photons).

On the integration parameters, see also [2102.00773](https://arxiv.org/abs/2102.00773)
For VBF/VBS processes sde_strategy 2 should be preferred. 
The integrator also seems to struggle with some EFT operators (in particular contact operators like four-fermion or four-boson operators) and sometimes with interference calculations.

## Warning: Non-trivial updates in 21.6

We have made an effort in release 21.6 to update, clean up, and clarify
quite a few functions within MadGraphControl. We've also added an
extensive set of tests, with job options (linked on this page) showing
how we think things should look. Particularly because of the interface
changes **it is expected that job options from older releases will not run in 21.6**. If you need help with the
migration, please [write to the list](mailto:atlas-generators-madgraphcontrol@cern.ch).


## Implicit type error

Frequently, users encounter an error that looks like this:

    14:54:17        run_card.inc:165.16:
    14:54:17            Included at setrun.f:41:
    14:54:17 
    14:54:17              CUT_DECAYS = ' F   '
    14:54:17                        1
    14:54:17        Error: Symbol 'cut_decays' at (1) has no IMPLICIT type

Perhaps with some other name in the place of "cut_decays" in the error
message. These errors mean that there is some option defined in the run
card (here named "cut_decays"; it may be defined in the extras
dictionary of the job options) that is not allowed as a part of the
generation you are running. This is common when trying to naively move
between NLO and LO, which allow different options. The options must be
removed from the run card in order for the job to complete. When doing
so, consider whether the options were supposed to do something in
particular and, if so, try to find a different way to do the same thing
(or take a careful look at the variables that are allowed in the run
card, which will be listed in the default run card for your process).

## Problems with run_card.dat in newer releases

Since MG5_aMCv2.30 there has been a different strategy for setting the
default `run_card.dat` parameters. This required a change in
MadGraphControl that is not backwards compatible. So if you see either
of the following errors using old jobOptions in a new release that is
most likely the cause:  
1)

    RuntimeError: Cannot find default run_card.dat or run_card_default.dat! I was looking here:...

or 2)

    TypeError: get_default_runcard() got an unexpected keyword argument 'isNLO'

or 3)

    ValueError: could not convert string to float: %(ktdurham)s

The fix is to move this line:

    process_dir = new_process()

above the `build_run_card()` function call, and then you need to pass
the `process_dir` from above to `get_default_runcard()`, so the
`build_run_card()` function looks something like this:

    build_run_card(run_card_old=get_default_runcard(proc_dir=process_dir),run_card_new='run_card.dat', xqcut=0, 
                   nevts=nevents,rand_seed=runArgs.randomSeed,beamEnergy=beamEnergy,extras=extras)

Note that if you previously passed `isNLO=True` to
`get_default_runcard()` then you also need to remove that.  

## run_card doesn't change

Make sure you run in a clean directory **every** time you change
something. MadGraphControl is designed to work in an empty directory (as
would be the case for official production).

## LHAPDF and fastjet warnings

You can ignore the following warnings when `new_process()` is executed,
the correct paths are set in the next stage (when `generate()` is
executed):

    lhapdf-config does not seem to correspond to a valid lhapdf-config executable.
    Please set the 'lhapdf' variable to the (absolute) /PATH/TO/lhapdf-config (including lhapdf-config).
    ...
    fastjet-config does not seem to correspond to a valid fastjet-config executable (v3+). We will use fjcore instead.
    Please set the 'fastjet'variable to the full (absolute) /PATH/TO/fastjet-config (including fastjet-config).

## Generation of EFT MC

The generation of EFT samples typically requires the generation of a
large number of samples. For this is can be useful to either use the
reweighting module described at [reweight](doc/reweight.md) . Samples
corresponding to SM-EFT interference as well as samples corresponding to
purely new-physics contributions can also be generated directly with the
method described at [MadGraphEFTMCdecomposition](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MadGraphEFTMCdecomposition).

## Using a private MG5_aMC Version

There are some cases where it might be useful to run with a privately
patched version of MadGraph. Note that this cannot be done in
production, and usually shouldn't be done on the grid; it's better to
get your patch into an official release as soon as it is clear that it
is correct. In order to run with a private version:

    # Set up your favorite release
    asetup 23.6,AthGeneration,here,latest
    # Copy our central MG installation
    cp -r $MADPATH LocalMadGraph
    # Make any necessary modifications to that installation
    # Reset things to point to the new installation
    export MADPATH=`pwd`/LocalMadGraph
    # Change the PYTHONPATH to add the local MG first
    export PYTHONPATH=$MADPATH:$PYTHONPATH

Note that the `PYTHONPATH` will still contain the "central" MadGraph
installation. If you wish to be super safe, you should replace the
central MadGraph installation with yours in `PYTHONPATH` rather than
just adding yours to the beginning of the patch. Nevertheless, for most
situations, the above will suffice.

## What is the default scale, how do I set other scales?

The default scale is a non-trivial dynamic scale, a few words on it here: https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/FAQ-General-13
One can choose other dynamic scales, see here: https://answers.launchpad.net/mg5amcnlo/+faq/2014
By changing setscales.f you can implement arbitrary scales and select them with the dynamical scale choice of usually 0 or 10.
In 3.5.X there is also a new way to set dynamic scales: https://answers.launchpad.net/mg5amcnlo/+faq/3325
There is also the option to use fixed scales, but in most cases this is not recommended.

## Particle width

### The total width (why is the default width different from the PDG value, why should I set it)

By default, the total width of particles is, in LO MadGraph, the particle width at LO, which can be quite different from the PDG value.
In general, but especially if one is sensitive to the lineshape (due to a large width), it can be important to update the width of particles.
This is however not the default as it might lead to unexpected behaviour in some cases. If, e.g., the W width is increased, the sum of

    p p > l+ vl

and

    p p > uptype downtype~

Will no longer add up to the total (LO) W cross-section but be rescaled by a factor LO_width/new_width

In particular for BSM models it might be required to update the particle width in the param card, too, if masses or couplings are changed.
This could either be done using an external formula (which might include higher-order corrections) or by setting the value of the width in the param card to "auto". In the latter case the particle width will be calculated dynamically by MG at LO.

### The role of branching ratios

Branching ratios are only relevant for MadSpin.
The syntax is a bit clunky, an examples of how to set the relevant "DECAY" block are [here](https://gitlab.cern.ch/atlas/athena/-/blob/21.6/Generators/MadGraphControl/python/MadGraphParamHelpers.py)
For standard event generation with, e.g.,

    p p > e+ ve

or even

    p p > w+, w+ > e+ ve

these BRs will not be considered and BRs will effectively be determined by (LO width)/(total width in param card).

## Do I need to use MadSpin to have spin correlations?

No, all particles that are decayed in MadGraph will be modelled correctly. You only need to use MadSpin if decays in MadGraph are not possible, for example for more complex NLO processes like ttbar production and decay.

## Usage of showers with MG5

MG5 samples can be generated using LO and NLO option. Various questions
come up in discussions all the time, in particular which shower can be
used and how. Please see below some advices and answers.

-   Q: aMC@NLO+Pythia8: can we use it?  A: **yes, with caution.** The
    main open question is at the moment related to few remaining
    discrepancies for ttbar processes using the recommended ``global
    recoil'' (as opposed to local recoil) for the interface between
    aMC@NLO and Pythia8.
    **Tuning**: in the context of the MG5_aMC@NLO tuning effort, small
    differences have been found between the parameter settings and the
    general A14. In general, A14 seems to be appropriate for Z processes as
    well but aMC@NLO+PYTHIA8 samples for Z have been also studied with the
    AZ tune (which probes that ISR and MPI): these are not affected by the
    global recoil, and can be used as alternative sample / set up. 
    Therefore: MG5_aMC@NLO+Pythia8 with global recoil and A14 tune is
    currently the recommended setup, but should be used with caution and
    comparisons with Herwig++ are recommended.
    
-   MG5 LO + Herwig++: can we use it? **A: only with extreme caution**
    The Herwig expertise in ATLAS is limited.

## Debugging outside of Athena<a name="write_test_script"></a>

Frequently we want to see if an issue can be reproduced in stand-alone
MadGraph5_aMC@NLO. In those sorts of cases, we have a helper function
that you can call from your job options, [write_test_script()](https://gitlab.cern.ch/atlas/athena/-/blob/master/Generators/MadGraphControl/python/MadGraphUtils.py#L198).
Just add to the relevant point in your job options:

    write_test_script()

and we will attempt to write a shell script into the log, as well as a
local copy (without the usual log decorations like times) for you to 
try to run. Sometimes this takes a little bit of attention (e.g. 
checking paths, or ensuring that changes you make in your job options
are being propagated to the script), but it should be a good starting
point for checking if things run outside of Athena.

Note that MadGraph5_aMC@NLO authors generally do not understand our
job options commands, so if you do need to open a ticket this is the
kind of script that you'll need to provide (along with the cards and so
on that you used). Sometimes the Athena log is sufficient for them
because we print all your cards and configuration there.

## I'm Really Stuck!

The [mailing list](mailto:atlas-generators-madgraphcontrol@cern.ch) is a
great place to go for help. Don't be shy!

If you are pretty sure that your issue is in MadGraph5_aMC@NLO itself,
you can check the
[questions](https://answers.launchpad.net/mg5amcnlo/+addquestion) and
[bugs](https://bugs.launchpad.net/mg5amcnlo/+filebug) sections of their
Launchpad page. Remember that the MadGraph5_aMC@NLO authors are not
ATLAS people; we need to translate what we're doing into a language they
can understand in order to ask the right question and get appropriate
help. The mailing list should be able to help identify whether you're
struggling with a MadGraph5_aMC@NLO issue or an "ATLAS issue" (or Athena
issue), and if it's really a MadGraph5_aMC@NLO issue the list can help
give you instructions to extract your job from Athena. The
[write_test_script()](#write_test_script)
function can be extremely helpful for creating a stand-alone bash script
that can be run to reproduce your issue without any ATLAS or job
transform wrappers.

-------

# **Release history**

For a history of releases and guidelines on what caches can and can't be
used for MadGraph5_aMC@NLO, please see the [PmgMcSoftware twiki](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/PmgMcSoftware).  
 

# Known issues in MadGraphControl
 

## Default weight not always the nominal weight

**Please note that we have
identified an issue with some MadGraph samples with on-the-fly weight
variations where the default weight is not the nominal weight.**

If you are using the
[PMGTruthWeightTool](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PmgEventWeights)
to access the weights in your sample(s), this should not affect you (you
can find out more about this tool and others supported by the PMG
[here](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PhysicsModellingGroup#PMG_Recommended_Supported_tools)).

If not, please double check that your samples are not affected by this.
You can do so by downloading an EVNT file and using
`checkMetaSG.py` eg:

    checkMetaSG.py mc16_13TeV.311388.MadGraphPy8EG_A14NNP23LO_monoSbb_zp2500_dm200_dh70.merge.EVNT.e7346_e5984/EVNT.17392141._000001.pool.root.1 | grep HepMCWeightNames

which will produce a line like this:

           /Generation/Parameters | HepMCWeightNames          | dict    | {'': 85, ' PDF=263000 MemberID=70 ': 18, ' MUR=2.0 MUF=2.0  ': 80, '...

The result is a dictionary of weights names and their index in the
weight vector. In particular, if you find the 0th weight you can tell if
this is nominal or a variation:

    ...
    ' MUR=0.5 MUF=0.5  ': 0,
    ...
    '': 85,
    ...

In this case, you can see that the 0th weight is not nominal but a
downward MUR/MUF variation. While the empty string weight (usually the
nominal) is in the 85th position. This is the one which should be used
as nominal.

*Please ensure you are using the
correct weight - the difference in cross section can be quite a lot
(sometimes up to 5-10% or more) depending on the phase space, and if you
are using the wrong weight in your signal then this could affect your
results by a similar amount.*

## Wrong Pythia cross section in LO samples with negative-weight events bug

In rare cases, there is a bug in the calculation of cross sections by
Pythia when using MadGraph LHE events. The main case where this will
have an effect is interference type calculations (where one requires
quadratic coupling constrains, e.g. NP^2==1 or QED^2=10) in combination
with systematics weights. However this bug will likely only be triggered
if you

-   Set 'event_norm = sum' (this will be a problem at NLO and LO)
-   Perform an interference type calculation (LO) and explicitly set
    'use_syst = True' (which will set 'event_norm = sum')
-   Perform an interference type calculation and use base fragments
    (which have recently become the recommend way to steer PDF and
    systematic settings)

In such a case the cross section reported by Pythia will be off by a
factor that is larger the larger the fraction of negative events is. If
you think you might be affected you can:

-   Compare the cross section reported by MadGraph and Pythia (which
    should be approximately the same if there is no filtering / merging
    involved).
-   Check whether the parameter event_norm in the run_card is set to
    "sum" or "average" (samples with 'event_norm = average ' should be
    fine).

More details can be found here:
<https://its.cern.ch/jira/browse/AGENE-1725>

## Parameter not found when updating the parameter card

In certain circumstances you might see an error that looks like:

    RuntimeError: Did not find any of the parameters for block <block name> in param_card

Please check to see if you are referring in your job options to the parameters by a
name in the comments (e.g. mW or mH) and, if so, that the comment also appears in the
existing param_card in the process directory. There are certain situations in which
MG5_aMC might remove comments from parameters. In these cases, you will need to refer
to the parameter according to its index (the number or numbers before the value), which
will always be present.

# To do list

See [AGENE-1614](https://its.cern.ch/jira/browse/AGENE-1614)