[[_TOC_]]

The Athena SuperChic interface described here was developed by Patrick McCormack (william.patrick.mc.cormack.iii@cern.ch), 
who now works for CMS.  For interface questions please contact  
- Aleksandra Dimitrievska (aleksandra.dimitrievska@cern.ch) 
- Simone Pagan Griso (simone.pagan.griso@cern.ch)  
- Andrii Verbytskyi (andrii.verbytskyi@mpp.mpg.de)

For generator-specific questions, you might want to contact 
[Lucian Harland-Lang](lucian.harland-lang@physics.ox.ac.uk) or [Marek Tasevsky](tasevsky@mail.cern.ch).

# Introduction

([SuperChic 5 ](https://github.com/LucianHL/SuperChic) is a generator for exclusive and photon-induced physics.  The current (as of January 2024) version of the athena interface 
of `SuperChic` has been strongly reworked from prior releases (those preceeding April 2021), now relying primarily on 
the standalone executables from LCG, rather than the Fortran code that works behind the scenes for SuperChic.  
The standalone executables can be found at e.g./cvmfs/sft.cern.ch/lcg/releases/LCG_104d_ATLAS_3/MCGenerators/superchic/5.0/x86_64-el9-gcc13-opt/.
The bugreports, requests and issues concerning the SuperChic should be filled at [github](https://github.com/LucianHL/SuperChic).
 The latest `SuperChic` manual is available on  [github](https://github.com/LucianHL/SuperChic/blob/v.5.0/doc/superchic.tex)) 

## Standalone execution of SuperChic

The standalone execution of `SuperChic` requires:
1) Folders named outputs/, inputs/, and evrecs/ in the directory that you want to run from.
2) An input configuration file, probably named `input.DAT`.  The format must follow that found in the 
directories with the standalone executables referenced above.
3) The libraries of the `lhapdf`  packages ( and `apfel` for Superchic 5.0) must also be in the `LD_LIBRARY_PATH` of your environment.  
This is taken care of if you source a recent 21.6,AthGeneration release (21.6.70 or later).
4) To run the code for semi-exclusive production you will need to have the `SF_MSHT20qed` structure function grid file in LHAPDF format placed in the appropriate (i.e. visible to LHAPDF) directory.  Those can be downloaded from [HepForge](https://superchic.hepforge.org/code.html)
For the `SuperChic` version 5.1 the tables are installed into the `${prefix}/share/SF` directory.


Standalone `SuperChic` is run in two steps:
1) First, you would run "init < input.DAT", which generates files in the `inputs/` directory.  This step performs some 
initial integrations that get referenced in the next step.
2) Next, you would run "superchic < input.DAT", using the same `input.DAT` file.  This step generates a LHE or HepMC output 
in the `evrecs/` directory that contains all of the generated events' information.  This LHE file can then be passed to `pythia8` 
for showering and/or hadronization.

# SuperChic Interface

The new athena SuperChic interface proceeds primarily as standalone SuperChic would.  An example of SuperChic without 
showering can be found at /athena/Generators/Superchic_i/share/jobOptions.SuperChic_ALP2.py.  It proceeds fairly linearly:
1) The necessary inputs/, outputs/, and evrecs/ folders are created if they don't already exist
2) An input.DAT file is generated using arguments that are given to "Init".  The class type for Init is defined here: 
/athena/Generators/Superchic_i/python/SuperChicUtils.py.  You can see all of the various input variables that can be given.  
The file is written with the writeInputDAT function in that python macro.
3) SuperChicRun also runs the init and superchic executables, creating an LHE file.  It is important that the output format 
be LHE, because the next step, which creates an EVNT file is only compatible with the LHE format.
4) The EVNT file is filled using the /athena/Generators/Superchic_i/python/EventFiller.py macro.  To use a format other than LHE, 
a different version of this macro would have to be written.

An example jobOptions file with pythia showering can be found at /athena/Generators/Superchic_i/share/jobOptions.SuperChicPy8_yyMuMu_SDA.py.  
The key difference here is of course that Pythia8 is added to the genSeq, and that we have to be careful about what pythia 
command options are included.  The /athena/Generators/Superchic_i/share/jobOptions.SuperChicPy8_yyMuMu_DD.py, 
/athena/Generators/Superchic_i/share/jobOptions.SuperChicPy8_yyMuMu_SDA.py, 
/athena/Generators/Superchic_i/share/jobOptions.SuperChicPy8_yyMuMu_SDB.py, and 
/athena/Generators/Superchic_i/share/jobOptions.SuperChicPy8_yyMuMu_El.py contain the latest recommended options for the case of 
photon-induced di-lepton for the cases of double-dissociated production, single dissociative production on one side and the other, 
and elasic production, respectively.

## Some Notes
* The S^2 model can be changed by adjusting the Init.isurv value
* The Init.diff should be either 'el', 'sda', 'sdb', or 'dd'.  Be careful to make sure that the "BeamRemnants:unresolvedHadron = 0" 
component of the pythia commands is adjusted accordingly below (el -> 3, sda -> 2, sdb ->1, and dd -> 0)
* The list of processes for Init.proc can be found in section 7 of the [manual](https://github.com/LucianHL/SuperChic/blob/v.5.0/doc/superchic.tex)
* Output kinematics can of course be changed from what is in the samples.  Two body decays use "a" and "b" particles (as in pta and ptb).  
Four body decays use "a", "b", "c", "d", and so forth.
  * Cuts that have "x" in them refer to the hard process as a whole, so e.g. the di-lepton system in a di-lepton event
* When running without showering with pythia, an important drawback of using LHE files should be noted: they do not use high enough precision.  
What I mean is the following.  In EventFiller.py, you can see that you enter in a four vector that contains every particle's px, py, pz, and E.  
The mass of the particles is then computed based on those values.  Unfortunately, for highly boosted objects (i.e. those where E >> m), such 
as the final state protons, the calculation of four-vector mass with the precision given in the LHE files is slightly off.  So in the 
output EVNT files, you'll see protons with masses in the range of 900 - 1000 MeV.  I have set the "generated_mass" of each particle to the 
right mass, but if you retrieve "m_m" for a truth-level particle, it will be slightly incorrect.

# Running locally

To run locally:
1) Make a folder to hold the athena release, go inside it, and run: asetup 21.6,latest,AthGeneration (21.6.70 or any later release should do)
2) (optional) make a run folder and go inside it
3) mkdir 999999
4) put a joboptions file in 999999/.  You may need to change the prefix from "jobOptions." to "mc.".
5) Run with e.g.:
   ```
   Gen_tf.py --ecmEnergy=13000.0 --maxEvents=1000 --firstEvent=1 --randomSeed=14 --outputEVNTFile=test.pool.root --jobConfig 999999
   ```

# Releases

A functional version of the SuperChic interface was first incorporated into the nightly release 21.6,AthGeneration,r2021-04-30T0933.  
Any full release more recent than 21.6.70 should contain a functional interface.  However, a slightly tweak was implemented in EventFiller.py 
to put the initial state protons into the EVNT files in the unshowered case.  This will likely be in release 21.6.72 and onward.
