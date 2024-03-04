[[_TOC_]]

The *Athena SFGen interface* described here was developed by *Lorenzo Primomo*, who works for *ATLAS*. For interface questions please contact *Giancarlo Panizzo* or *Dominic Hirschbuehl*. For generator-specific questions, you might want to contact the *SFGen* author *Lucian Harland-Lang*.

List of responsibles:

* Lorenzo Primomo (lprimomo@cern.ch);
* Giancarlo Panizzo (giancarlo.panizzo@uniud.it);
* Dominic Hirschbuehl (dominic.hirschbuehl@cern.ch);
* Lucian Harland-Lang (l.harland-lang@ucl.ac.uk));

# Introduction

*SFGen1.03* ([ref](https://sfgen.hepforge.org/) and [manual](https://sfgen.hepforge.org/SFGen1.03.pdf)) is a Monte Carlo generator which provides a publicly available tool for lepton pair production and lepton-lepton scattering within the *Structure Function approach*, including initial-state γ, Z and mixed Z/+q contributions. Arbitrary distributions and unweighted events can be generated, and errors due to the experiment uncertainty on the structure function (the equivalent of *PDF* uncertainties in the photon *PDF* framework) can be calculated on-the-fly. The LO collinear photon-initiated predictions can also be calculated for comparison, for arbitrary factorization and renormalization scales. 

The standalone executables of a more recent parched version can be found at:

* `/cvmfs/sft.cern.ch/lcg/releases/LCG_88b/MCGenerators/SFGen/1.03p0/x86_64-centos7-gcc62-opt`;
* `/cvmfs/sft.cern.ch/lcg/releases/LCG_104d_ATLAS_4/MCGenerators/SFGen/1.03.atlas2/`;
* `/cvmfs/sft.cern.ch/lcg/releases/LCG_104d_ATLAS_3/MCGenerators/SFGen/1.03.atlas2/`;

It will be available in *ATLAS cvsmfs* soon. 

## Intro to standalone SFGen

When running independently, *SFGen* requires:
1) Folders named `outputs/` and `evrecs/` in the directory that you want to run from;
2) An input configuration file, named *input.DAT*. The format must follow that found in the directories with the standalone executables referenced above;
3) The packages *APFEL* and *LHAPDF* must also be in the `ROOT_INCLUDE_PATH` and `LD_LIBRARY_PATH` of your environment. This is taken care of if you source a recent AthGeneration 21.6.106 release or the more recent 23.6.24 release;

-----

Standalone SFGen is run in only one step:

The main MC code may be run with "./SFGen < input.DAT", using the same *input.DAT* file. This step generates an *LHE/HepMC/HEP-EVT* in the `evrecs/` folder that contains all of the generated events' information. This *LHE* file can then be fed into *Pythia* for showering.

-----

# SFGen Interface

The new *Athena SFGen interface* proceeds primarily as standalone *SFGen* would. It proceeds fairly linearly:
1) The necessary `outputs/` and `evrecs/` folders are created if they don't already exist;
2) An *input.DAT* file is generated using arguments that are given to "Init". The class type for "Init" is defined here: `/athena/Generators/SFGen_i/python/SFGenUtils.py`. You can see all of the various input variables that can be given. The file is written with the "writeInputDAT" function in that python macro;
3) "SFGenRun" also runs the SFGen executable, creating an *LHE* file. It is important that the output format be *LHE*, because the next step, which creates an *EVNT* file, is only compatible with the *LHE* format;
4) The *EVNT* file is filled using the `/athena/Generators/SFGen_i/python/EventFiller.py` macro. To use a format other than *LHE*, a different version of this macro would have to be written;

An example *jobOptions* file with *Pythia* showering can be found at `/athena/Generators/SFGen_i/share/mc.SFGenPy8_MuMu_SDA.py`. The key difference here is of course that *Pythia8* is added to the "genSeq", and that we have to be careful about what *Pythia* command options are included. The `/athena/Generators/SFGen_i/share/mc.SFGenPy8_MuMu_DD.py`, `/athena/Generators/SFGen_i/share/mc.SFGenPy8_MuMu_SDA.py`, `/athena/Generators/SFGen_i/share/mc.SFGenPy8_MuMu_SDB.py`, and `/athena/Generators/SFGen_i/share/mc.SFGenPy8_MuMu_El.py` contain the latest recommended options for the case of photon-induced di-lepton for the cases of double-dissociated production, single dissociative production on one side and the other, and elastic production, respectively.

## Some Notes

* The "Init.diff" should be either 'el', 'sda', 'sdb', or 'dd'. Be careful to make sure that the "BeamRemnants:unresolvedHadron = 0" component of the *Pythia* commands is adjusted accordingly below (el -> 3, sda -> 2, sdb ->1, and dd -> 0).
* The list of processes for "Init.proc" can be found in *Section 7* of the [manual](https://sfgen.hepforge.org/SFGen1.03.pdf).
* Output kinematics can of course be changed from what is in the samples. Two body decays use "a" and "b" particles (as in "pta" and "ptb").
  * Cuts that have "x" in them refer to the hard process as a whole, so e.g. the di-lepton system in a di-lepton event.
* When running without showering with *Pythia*, an important drawback of using *LHE* files should be noted: they do not use high enough precision.  What I mean is the following: In `EventFiller.py`, you can see that you enter in a four vector that contains every particle's px, py, pz, and E. The mass of the particles is then computed based on those values. Unfortunately, for highly boosted objects (i.e. those where E >> m) the calculation of four-vector mass with the precision given in the *LHE* files is slightly off. So in the output *EVNT* files, you will see protons with masses in the range of 900 - 1000 MeV. I have set the "generated_mass" of each particle to the right mass, but if you retrieve "m_m" for a truth-level particle, it will be slightly incorrect.

# Running locally

To run locally:
1) Make a folder to hold the *Athena release*, go inside it, and run: asetup 21.6,latest,AthGeneration (21.6.106 or any later release should do);
2) (Optional) Make a run folder and go inside it;
3) mkdir 999999;
4) Put a *jobOptions file* in `999999/`;
5) Run with e.g.:

   ```
   Gen_tf.py --ecmEnergy=13000.0 --maxEvents=1000 --firstEvent=1 --randomSeed=13 --outputEVNTFile=test.pool.root --jobConfig=999999
   ```

# Releases

The *SFGen* Athena interface will be soon incorporated into *AthGeneration release 23.6.25*.
