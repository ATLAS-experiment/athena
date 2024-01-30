# Pythia 8 in ATLAS

For any issues or advice, please contact the support team via atlas-generators-pythia@cern.ch.

Significant problems requiring code fixes should be submitted via the ATLAS
Generators JIRA: https://its.cern.ch/jira/projects/AGENE/issues/AGENE-1427


## Introduction

Pythia 8 can be run within Athena by using the `Pythia8_i` interface algorithm.
The resulting HepMC objects are put into the `StoreGate` data store, and may be
used by other packages (e.g full simulation, Atlfast, Rivet, etc.) in the usual
way. The `Pythia_i` interface code is located in [this
package](https://gitlab.cern.ch/atlas/athena/tree/21.6/Generators/Pythia8_i).


## Running Pythia 8 in Athena

`Pythia8_i` can be used from any MC production release, set up with
e.g. `setupATLAS; lsetup asetup; asetup 21.6.20,AthGeneration`.  Releases not
used for MC production will not have been tested, but may nevertheless work.

Pythia8_i should be run via the `Gen_tf.py` transform script in release 21
(formerly `Generate_tf.py` in MC15/releases 19-20, and `Generate_trf.py` in
MC12/release 17).  The transform can download standard "job option" run
configuration scripts automatically, given a job-config/dataset ID number, so if
you just want to run an existing MC process just run like this:

    Gen_tf.py --ecmEnergy=13000 --jobConfig=421113 --maxEvents=10 --outputEVNTFile=test_minbias_inelastic.EVNT.pool.root

If the process you want does not already exist, you will need to make your own
job option script and run it locally: this is covered in the following section.


## Writing job option files

Athena is steered using job option scripts (JOs), written in Python. To run
"just" Pythia8, i.e. internal hard-process simulation rather than external
partonic events via LHE format (for which special settings are often required)
you should start your JO by `include()`ing a base JO fragment from
https://gitlab.cern.ch/atlas-physics/pmg/infrastructure/mc15joboptions/-/tree/master/share

    include("MC15JobOptions/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")

This file itself builds upon a non-"user-facing" set of more fundamental setup
fragments, but you should use these versions with an MC tune (A14) and parton
density set (NNPDF23LO) configured, and usually with some dedicated particle
decays handled by the EvtGen afterburner program.

Any valid Pythia8 command, including those to set the process and generation cuts, may be passed in using

    genSeq.Pythia8.Commands += ['foo=bar']

The beam specifications are set on the command line by `Generate_tf.py` and do not need to be specified in the JO.


## Using a development version of Pythia8_i

To use a development version not already in a release, you will need to check
out and build the relevant packages from the ATLAS Git repository. Before doing
this, follow the ATLAS Git workflow instructions at
https://atlassoftwaredocs.web.cern.ch/gittutorial/workflow-quick/ to get a
personal fork of the ATLAS codebase. Then make and a sparse checkout of the
`Pythia8_i` package with your preferred Git branch or tag:

    setupATLAS; lsetup asetup; lsetup git
    git atlas init-workdir https://:@gitlab.cern.ch/USERNAME/athena.git
    cd athena
    git atlas addpkg Pythia8_i
    git checkout -b 21.6 release/21.6.20 --no-track   #< use your own preferred tag or branch
    cd ..

Now make a build directory separate from the source checkout, set up the release
to build against with your local modifications, and build & set up the run:

    mkdir athena-build && cd athena-build
    asetup 21.6.20,AthGeneration
    cmake -DATLAS_PACKAGE_FILTER_FILE=../package_filters.txt ../athena/Projects/WorkDir
    make
    source x86_64-*/setup.sh


## Using a custom libPythia8 library

Using a new copy of the Pythia8 library for testing can be a bit more involved,
because you will also need to check out the `Pythia8` externals package and, depending on the release, also
override some paths to point at the new version. Here is a full set of commands
to run, again starting from a new checkout of your Git
Athena fork:

    setupATLAS; lsetup asetup; lsetup git
    git atlas init-workdir https://:@gitlab.cern.ch/USERNAME/athena.git athena-py8custom
    cd athena-py8custom
    git atlas addpkg Pythia8 Pythia8_i #< or just Pythia8 if you don't plan to implement changes in Pythia8_i

From now on, the procedure depends on your release version: in all cases, be sure to have followed the above steps in a new terminal, where no other `asetup` commands have been called. Check also to work in a new, clean, `build` folder, where appropriate.

### Older releases (development releases in R21)

If you really still need to use older R21 releases, you now need to modify the project build instructions to pick up the external
library override, by editing `athena/Projects/WorkDir/CMakeLists.txt`:

1. Just after `find_package( AtlasCMake QUIET )` add, for example,

        set( PYTHIA8_LCGVERSION 301p3 )
        set( PYTHIA8_LCGROOT  /cvmfs/sft.cern.ch/lcg/releases/LCG_88/MCGenerators/pythia8/${PYTHIA8_LCGVERSION}/${LCG_PLATFORM} )
        set( PYTHIA8_VERSION ${PYTHIA8_LCGVERSION})
        set( PYTHIA8_ROOT    ${PYTHIA8_LCGROOT})

2. Just after `# Find the project that we depend on:` add

        find_package(AthGenerationExternals REQUIRED)

And in `athena/External/Pythia8/CMakeLists.txt`, add LCG version overrides just
before `find_package( Pythia8 )`:

    set( PYTHIA8_LCGVERSION 301p3 )
    set( PYTHIA8_LCGROOT /cvmfs/sft.cern.ch/lcg/releases/LCG_88/MCGenerators/pythia8/${PYTHIA8_LCGVERSION}/${LCG_PLATFORM})

3. Before building, you should also create a `package_filters.txt` file, perhaps
based on `athena/Projects/WorkDir/package_filters_example.txt`. This should contain the
following lines, specifying which packages to build:

        + External/Pythia8
        + Generators/Pythia8_i #< you may comment this if you don't plan to implement changes in Pythia8_i
        - .*

4. Finally, (re)build:

        mkdir athena-py8custom-build && cd athena-py8custom-build
        asetup 21.6.20,AthGeneration
        # or: asetup 21.6,latest,AthGeneration,slc6
        cmake -DATLAS_PACKAGE_FILTER_FILE=../package_filters.txt ../athena/Projects/WorkDir
        make
        source x86_64-*/setup.sh

You may wish to run a `make clean` before `make`, to ensure that everything is
definitely rebuilt as intended.

### Most recent releases (late R21, and R22/R23)
Skip steps 1 and 2 of the procedure described above. Follow step 3. Instead of step 4, do the following:

    mkdir athena-py8custom-build && cd athena-py8custom-build
    asetup 21.6.99,AthGeneration
    # or in R22/23: asetup AthGeneration,main,latest
    cmake -DATLAS_PACKAGE_FILTER_FILE=../package_filters.txt -DPYTHIA8_LCGROOT=/cvmfs/sft-nightlies.cern.ch/lcg/nightlies/dev4/Thu/MCGenerators/pythia8/309/x86_64-centos7-gcc11-opt ../athena/Projects/WorkDir/
    make
    source */setup.sh


where in the above example Pythia8.309 has been taken from a nightly build.

## MLM matching within Athena
Where CKKWL cannot be applied (notably, in all loop induced processes) it can be useful to run MLM matching.
The first efforts in implementing MLM matching within Athena are documented in this [JIRA ticket](https://its.cern.ch/jira/browse/AGENE-1879).
The MLM matching base fragment is Pythia8_i/Pythia8_ClassicalMLM_Match.py, which works in close analogy to the corresponding CKKWL base fragment.
This implement the classical, Madgraph inspired, MLM matching technique,  checking on an event-by-event basis the event jet multiplicity.

## Sacrifice standalone steering package

In addition to the Athena interface, James Monk has written a standalone package
that steers Pythia 8 without needing Athena, and provides Photos++, LHAPDF, LHEF
and HepMC interfaces and can be more easily steered from the command line.  The
package is available from the AGILe project on HepForge: https://agile.hepforge.org/

# Getting Differential Distribution Rates ( DJRs ) from CKKWL

In this tutorial, we will outline the procedures for extracting the internally computed DJRs from Pythia8, employing the mergingDJRs.cxx UserHook. We will focus on a straightforward scenario and utilize the Madgraph generator, although any other matrix element generator can be employed. Specifically, we will examine the production of a Z boson with 1-jet multiplicity.

This tutorial is the result of the following Qualification Task: 

https://its.cern.ch/jira/browse/AGENE-2069

# Introduction 

Parton showers (PS) are suitable in soft/collinear parton emissions regions, while they fail with hard and well-separated regions.
The latter are well modeled only by matrix elements (ME). We will have the hard jet of the process from the Matrix element generator whil the soft jets should be modelled by emissions generated by Pythia8. 
Consistently merging processes helps eliminate duplicate counts and overlaps between hard and soft partons by introducing a merging scale. One way to validate this approach is by directly extracting the N+1 → N jet clustering scales, also known as the Differential Jet Rates (DJRs), from Pythia8.

For more information about the CKKWL merging scheme, please check the following link:
https://pythia.org/latest-manual/CKKWLMerging.html

# The steps to use the UserHook and activate the merging procedure:

1) Create multiple JOs depending on the jet multiplicity of your hard process.
2) Start generating jet multiplicity and the merged sample such as: 

  a) Generate Seperated JOs for each jet multiplicity: 

```
   p p > z @0 

   p p > z j @1 
 
   p p > z j j @2
```

  b) generate a sample for the merged process such as: 

```
  generate p p > z 
  add process p p > z j 
  add process p p > z j j 
```

2) Add to your JO the fragment code which activates the merging procedure and configures all the essential settings for the merging process, such as: 

```
PYTHIA8_nJetMax=2
PYTHIA8_Process='pp>e-e+'
PYTHIA8_Dparameter=0.4
PYTHIA8_TMS=30.0
PYTHIA8_nQuarksMerge=4

include("Pythia8_i/Pythia8_CKKWL_kTMerge.py")
``` 

3) Include the fragment code responsible for computing the DJRs: 

```
include("Pythia8_i/Pythia8_mergingDJRs.py")
```

4) You run the JO with Gen_tf.py.

5) The output will be a ROOT file called `hist-DJR.root` that contains histograms fro the first and second DJR. The user then should stack the plots for the different jet multiplicity and the merged sample manually.

# How to know the merging is working ?

**1)  Make sure that the merging is activate:**

 If all the parameters are properly configured, especially with  `Merging:doKTMerging = on `  the merging process should be operational. Consequently, you can verify this by examining your log file, where you should find  `MEPS Merging Initialization ` printout that contains the configurations you have specified.

**2) The merging scale choice:**
  
 One crucial configuration in the CKKWL merging scheme is known as  `the merging scale`, which can be specified through the  `Merging:TMS` setting. This parameter serves as a threshold for the jet multiplicities included in our process. 
 Its selection should be linked to the process's hard scale, such as the mass of the produced particles, HT cut etc. The threshold is ideally chosen within a range of  `~ 1/6 to 1/3 of the hard scale`. We systematically vary the scale within this range and assess the results based on the DJRs plot to determine the optimal setting. 
 For the SUSY process, it is advisable to utilize the mass of the final particles as the preferred hard scale.

**3) Analyzing the DJRs Plots:**
   
 The interpretation of DJRs plots is pivotal in determining the effectiveness of the chosen merging scale. Several key observations can guide the user in assessing the suitability of the merging scale:
 -  The DJRs histograms should be continuous. 
 -  The combined jet-multiplicity samples' sum should align with the merged sample.
 -  In the initial DJR, there should be a clear distinction between the 0-jet sample and the 1-jet sample. Similarly, in the second DJR, the separation between the 1-jet sample and the 2-jet sample should be evident. A more distinct separation indicates a better choice of the merging scale.
 -  The point where the 1-jet sample becomes dominant and the 0-jet multiplicity reaches zero signifies the initiation of the merging scale. The higher the value for our merging scale is, the greater the corresponding value of DJR at this juncture should be.
 -  In the case of the first DJR, when dealing with small DJR values, the 0-jet multiplicity should closely resemble the merged sample. As DJR values increase, the 1-jet multiplicity should approximately match the merged sample.
