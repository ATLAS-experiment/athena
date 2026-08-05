The following code is used for the production of MET histograms for NTUP_PHYSVAL.root files used for physics validation. 

For the production of these MET histograms the code collects the particles (electron, photon, muon, & tau) alongside jets. Particles undergo a pt and eta selection. After these selections the particles are registered into METMaker, in which they undergo OR and NNJVT with a selection applied to jets as well. The elements for the particles and jets are then retrieved from METMaker. It is these elements for the electrons, photons, muons, tau, and jets we receive from METMaker which are used to fill the histograms for final the NTUP_PHYSVAL file.

The histograms produced are saved within the NTUPLE under the folder named MET. Within this folder the histograms are divided between subfolders, some of which also have folders for each particle type examined.

For validation the NTUP_PHYSVAL.root files produced using this will be used for the weekly physics validations. These validations produce comparison histograms between a pair of NTUP_PHYSVAL.root files and upload them to the PhysVal page: https://atlas-physval.web.cern.ch/ for examination.
