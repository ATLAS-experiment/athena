# PhysVal MET athena code

Contributers: 
Daniel Buescher, Philipp Mogg - 2022
Owen Darragh - 2026

For questions please contect the JSV conveners: atlas-cp-jetetmiss-jsv-conveners@cern.ch


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


Types of histograms produced


For validation the NTUP_PHYSVAL.root files produced using this will be used for the weekly physics validations. These validations produce comparison histograms between a pair of NTUP_PHYSVAL.root files and upload them to the PhysVal page: https://atlas-physval.web.cern.ch/ for examination.

For more information on the full PhysVal procedure please check out this TWiki: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PhysValMonitoring

#Produced histograms

Outlined below are all histograms that should be produced and their sub folders. Within a produced ntuple these should all be under a folder called MET.

##MET_Calo

This folder contains the calo histograms. They are named as follows:
Calo 
Calo_x 
Calo_y 
Calo_phi 
Calo_sum 

##MET_Rebuilt_AntiKt4EMTopo &  MET_Rebuilt_AntiKt4EMPFlow

These are two different folders each has the same histograms. However one is for EMTopo and the other for EMPFlow. The following sections are the major histogram folders.

###Correlations

This folder contains correlation histograms. Within the EMTopo section these are labeled as such:
MET_Rebuilt_AntiKt4EMTopo_Muons_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_Muons_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_PVSoftTrk_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_RefEle_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_RefEle_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_RefGamma_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_RefGamma_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_RefJet_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_RefJet_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_RefTau_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_RefTau_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_SoftClus_FinalClus 

###Cumulative

This folder contains cumulative histograms. Within the EMTopo section these are labeled as such:
MET_Rebuilt_AntiKt4EMTopo_Cumulative_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_Cumulative_FinalTrk 

###Differences

This folder contains subfolders for each particle. These are called RefEle, RefMuons, RefGamma, RefJet, and RefTau. Within each of these folders we have difference histograms for the particles. Within a RefMuons folder in the EMTopo section these are labeled as such:
MET_Rebuilt_AntiKt4EMTopo_Diff_RefMuons 
MET_Rebuilt_AntiKt4EMTopo_Diff_RefMuons_phi 
MET_Rebuilt_AntiKt4EMTopo_Diff_RefMuons_sums 
MET_Rebuilt_AntiKt4EMTopo_Diff_RefMuons_x 
MET_Rebuilt_AntiKt4EMTopo_Diff_RefMuons_y 

###Residuals

This folder contains residual histograms. Within the EMTopo section these are labeled as such:
MET_Rebuilt_AntiKt4EMTopo_Resolution_FinalClus_x 
MET_Rebuilt_AntiKt4EMTopo_Resolution_FinalClus_y 
MET_Rebuilt_AntiKt4EMTopo_Resolution_FinalTrk_x 
MET_Rebuilt_AntiKt4EMTopo_Resolution_FinalTrk_y 

###Significance

This folder contains significance histograms. Within the EMTopo section these are labeled as such:
MET_Rebuilt_AntiKt4EMTopo_Significance_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_Significance_FinalTrk 

###Terms

This folder contains the subfolders FinalClus, FinalTrk, PVSoftTrk, SoftClus, RefEle, RefMuons, RefGamma, RefJet, and RefTau. Within each of these folders we have terms histograms for the particles. Within a RefMuons folder in the EMTopo section these are labeled as such:
MET_Rebuilt_AntiKt4EMTopo_RefMuons 
MET_Rebuilt_AntiKt4EMTopo_RefMuons_phi 
MET_Rebuilt_AntiKt4EMTopo_RefMuons_sum 
MET_Rebuilt_AntiKt4EMTopo_RefMuons_x
MET_Rebuilt_AntiKt4EMTopo_RefMuons_y 

###dPhi

This folder contains dPhi histograms. Within the EMTopo section these are labeled as such:
MET_Rebuilt_AntiKt4EMTopo_dPhi_leadJetMET_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_dPhi_leadJetMET_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_dPhi_subleadJetMET_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_dPhi_subJetMET_FinalClus 
MET_Rebuilt_AntiKt4EMTopo_dPhi_leadLepMET_FinalTrk 
MET_Rebuilt_AntiKt4EMTopo_dPhi_leadLepMET_FinalClus 

###Kinematics

This folder contains subfolders for each particle. These are called RefEle, RefMuons, RefGamma, RefJet, and RefTau. Within each of these folders we have kinematics histograms for the particles. Within a RefMuons folder in the EMTopo section these are labeled as such:
MET_Rebuilt_AntiKt4EMTopo_Kinematic_RefMuons_pt
MET_Rebuilt_AntiKt4EMTopo_Kinematic_RefMuons_eta
MET_Rebuilt_AntiKt4EMTopo_Kinematic_RefMuons_phi
MET_Rebuilt_AntiKt4EMTopo_Multi_RefMuons


