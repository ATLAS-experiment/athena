# PhysVal MET athena code

This code was written by Daniel Buescher <daniel.buescher@cern.ch> & Philipp Mogg <philipp.mogg@cern.ch> in 2022. The current version was modified by Owen Darragh Aug 2026. Contact owendarragh@cmail.carleton.ca if you have questions.

The following code is used for the production of MET histograms for NTUP_PHYSVAL.root files used for physics validation. 

For the production of these MET histograms the code collects the particles (electron, photon, muon, & tau) alongside jets. Particles undergo a pt and eta selection. After these selections the particles are registered into METMaker, in which they undergo OR and NNJVT with a selection applied to jets as well. The elements for the particles and jets are then retrieved from METMaker. It is these elements for the electrons, photons, muons, tau, and jets we receive from METMaker which are used to fill the histograms for final the NTUP_PHYSVAL file.

The histograms produced are saved within the NTUPLE under the folder named MET. Within this folder the histograms are divided between subfolders, some of which also have folders for each particle type examined.

For the production of NTUPLES with MET PhysVal histograms produced locally do the following:
1. Setup the appropriate athena systems (Tutorial can be found here: https://atlas-software.docs.cern.ch/athena/git/)
2. Setup your build and compile the code
```
asetup main,Athena,latest 
cmake ../athena/Projects/WorkDir 
make -j8 
source x86*/setup.sh
```
3. Within a folder you want to run this use the following command to produce your ntuple. Below is a sample using the DAOD_PHYSVAL.48599043._000003.pool.root.1 to produce NTUP_PHYSVAL.48599045._000002.pool.root.1, there files can be swapped out for your choice.
```
Derivation_tf.py --inputDAOD_PHYSVALFile="DAOD_PHYSVAL.48599043._000003.pool.root.1" --sharedWriter="True" --runNumber="601229" --AMITag="p7134" --CA="all:True" --outputNTUP_PHYSVALFile="NTUP_PHYSVAL.48599045._000002.pool.root.1" --validationFlags doMET
```
Note: its is the doMET validation flag which used the MET PhysVal code. Other flags may also be used with a "," as spacing.

For validation the NTUP_PHYSVAL.root files produced using this will be used for the weekly physics validations. These validations produce comparison histograms between a pair of NTUP_PHYSVAL.root files and upload them to the PhysVal page: https://atlas-physval.web.cern.ch/ for examination.
