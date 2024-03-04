[[_TOC_]]

# SFGen_i: An interface between SFGen Monte Carlo event generator for Athena version in release 21.6

Author:
 
* Lorenzo Primomo (lprimomo@cern.ch).

# Introduction

SFGen1.03 Monte Carlo event generator for central exclusive production.

This interface package runs SFGen Monte Carlo Generator (v1.03) within
Athena framework, and stores the events output into the transient store
in HepMC format.

SFGen Monte Carlo event generator is dedicated for lepton pair
production and lepton-lepton scattering within the Structure Function
approach, including initial-state $\gamma$, $Z$ and mixed $Z/\gamma + q$
contributions. This documentation gives you some details about setting
up the SFGen interface (which is prepared using the latest available
SFGen version 1.03) and activating the module using the JobOptions.

The code makes use of structure function grids in the LHAPDF format, as
calculated using APFEL. These structure function grids are provided with
the MC, and are calculated with MSHT20qe_nnlo PDFs in the high $Q^2$
region.

# Usage of Job Options

The example job option file can be found under the following path.

**SFGen_i/share/mc.SFGen.py**

Before running this script, first setup the Athena environment:

**asetup 23.6,latest,AthGeneration,slc6**

Above command sets up the latest AthGeneration cache.

The SFGen input parameters can be set from the job options service. The
default parameters initializes SFGen for proton-proton beams at
center-of-mass energy of 13 TeV for the scattering process
\[$\gamma \gamma  \rightarrow \mu^+ \mu^-$ (process n. 2)\].

**All the parameters passed to SFGen are in the units specified in the
SFGen manual https://sfgen.hepforge.org/SFGen1.03.pdf**

The default mc.SFGen.py file can be copied to your test run directory.
The initialization parameters can be changed via the following line in
the python file.

    SFGen.Initialize = ["parameter_1 value_1", "parameter_2 value_2"]

Each quoted string sets one parameter value in the fortran variable
format. You can set all the input parameters separated by commas,
however, the important ones are listed below.

**parameter_1:** must be one of the following variable names, an error
message is returned if the specified variable is not in the input
parameter list.

**value_1:** is the value of the input parameter.\
JO Example:\
The following command generates 10 events for proton-proton collisions
at 13 TeV center-of-mass energy along with important input parameters
for process 2 i.e. $\gamma \gamma  \rightarrow \mu^+ \mu^-$.\
Running the Job Option to produce events:

    Gen_tf.py --ecmEnergy=13000.0 --maxEvents=10  --firstEvent=1  --randomSeed=13  --outputEVNTFile=test.pool.root --jobConfig=mc.SFGen.py

Example of the initialization of the input parameters in the JO is shown
below.

    genSeq.SFGen.Initialize = \
    ["rts 13d3",                     # set the COM collision energy (in fortran syntax)
    "diff 'tot'",            # elastic ('el'), single/double dissociation ('sd'/'dd')
    "PDFname 'SF_MSHT20qed_nnlo'",      # PDF set name
    "PDFmember 0",                   # PDF member
    "proc 2",                        # Process number (1:gg->ee,2:gg->mumu,3:gg->tautau)
    "outtag 'out'",                  # for output file name
    "SFerror .false."        # Include error from SF input - increases run time
    ]


One can also add the kinematical cuts to the outgoing particles or the
system, details can be found in the manual.

# Running SFGen in Standalone way

One can directly carry out following commands in the fresh terminal to
produce SFGen events in standalone way.

    source /cvmfs/sft.cern.ch/lcg/releases/LCG_88b/MCGenerators/SFGen/1.03p0/x86_64-centos7-gcc62-opt/sfgenenv-genser.sh
    cp -rf /cvmfs/sft.cern.ch/lcg/releases/LCG_88b/MCGenerators/SFGen/1.03p0/x86_64-centos7-gcc62-opt/bin/tmp/cern_username/
    cd /tmp/cern_username/bin/
    export LHAPATH=/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current/
    ./SFGen < input.DAT

You can change the input.DAT file to change the C.O.M collision energy
for the particular process, process no., number of events etc. Above
commands will produce output stored under the directory **'evrecs'** .
