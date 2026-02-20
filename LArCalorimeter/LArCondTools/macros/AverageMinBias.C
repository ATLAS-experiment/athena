/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#define AverageMinBias_cxx
#include "AverageMinBias.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

void AverageMinBias::Loop()
{

   if (fChain == 0) return;

   Long64_t nentries = fChain->GetEntriesFast();

   TFile *fout=new TFile("ntuple_av.root","RECREATE");
   TTree* tout=new TTree("m_tree","Averaged cells in region=0, layer=1");
         
   const unsigned nstripgrp=448;
   Double_t av8[nstripgrp], rms8[nstripgrp];
   for(int j=0; j<nstripgrp; ++j) {av8[j]=0.; rms8[j]=0.;}

   tout->Branch("ncell", &ncell, "ncell/I");
   tout->Branch("nevt_total", &nevt_total, "nevt_total/I");
   tout->Branch("identifier", &identifier, "identifier[ncell]/I");
   tout->Branch("layer", &layer, "layer[ncell]/I");
   tout->Branch("region", &region, "region[ncell]/I");
   tout->Branch("ieta", &ieta, "ieta[ncell]/I");
   tout->Branch("eta", &eta, "eta[ncell]/F");
   tout->Branch("phi", &phi, "phi[ncell]/F");
   tout->Branch("nevt", &nevt, "nevt[ncell]/D"); 
   tout->Branch("average", &average, "average[ncell]/D");
   tout->Branch("rms", &rms, "rms[ncell]/D");
   tout->Branch("reference", &reference, "reference[ncell]");

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      for(int icell=0; icell<ncell; ++icell) {
         if(layer[icell] != 1 || (layer[icell]==1 && region[icell] !=0)) continue;
         int inum=ieta[icell]/8;
         int imin=0; 
         if(inum==0) imin=1;  
         for(int j=imin; j<8; ++j) {
            av8[j+inum*8] += average[icell];
            rms8[j+inum*8] += rms[icell];
         }
      }
      for(int j=1; j<8; ++j) {av8[j]/=7; rms8[j]/=7;}
      for(int j=8; j<nstripgrp; ++j)  {av8[j]/=8; rms8[j]/=8;}
      // now replace layer=1 region=0 cells by average and save
      for(int icell=0; icell<ncell; ++icell) {
         if(layer[icell]==1 && region[icell]==0){
           cout<<ieta[icell]<<" "<<average[icell]<<" "<<av8[ieta[icell]]<<endl;
           average[icell] = av8[ieta[icell]];
           rms[icell] = rms8[ieta[icell]];
         }
      }
      tout->Fill();
      fout->cd();
      tout->Write();
      fout->Save();
      fout->Close();
   }
}
