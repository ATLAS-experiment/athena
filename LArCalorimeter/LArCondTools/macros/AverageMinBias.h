/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Macro to average MinBias hits energies in the EMB strips


#ifndef AverageMinBias_h
#define AverageMinBias_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

class AverageMinBias {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   static const unsigned  nindex=1833;
   // Declaration of leaf types
   Int_t           ncell;
   Int_t           nevt_total;
   Int_t           identifier[nindex];   //[ncell]
   Int_t           layer[nindex];   //[ncell]
   Int_t           region[nindex];   //[ncell]
   Int_t           ieta[nindex];   //[ncell]
   Float_t         eta[nindex];   //[ncell]
   Float_t         phi[nindex];   //[ncell]
   Double_t        nevt[nindex];   //[ncell]
   Double_t        average[nindex];   //[ncell]
   Double_t        rms[nindex];   //[ncell]
   Double_t        reference[nindex];   //[ncell]

   // List of branches
   TBranch        *b_ncell;   //!
   TBranch        *b_nevt_total;   //!
   TBranch        *b_identifier;   //!
   TBranch        *b_layer;   //!
   TBranch        *b_region;   //!
   TBranch        *b_ieta;   //!
   TBranch        *b_eta;   //!
   TBranch        *b_phi;   //!
   TBranch        *b_nevt;   //!
   TBranch        *b_average;   //!
   TBranch        *b_rms;   //!
   TBranch        *b_reference;   //!

   AverageMinBias(TTree *tree=0);
   virtual ~AverageMinBias();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);

   AverageMinBias(const AverageMinBias&) = delete;
   AverageMinBias& operator=(const AverageMinBias&) = delete;
};

#endif

#ifdef AverageMinBias_cxx
AverageMinBias::AverageMinBias(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("ntuple.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("ntuple.root");
      }
      f->GetObject("m_tree",tree);

   }
   Init(tree);
}

AverageMinBias::~AverageMinBias()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t AverageMinBias::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t AverageMinBias::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void AverageMinBias::Init(TTree *tree)
{
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("ncell", &ncell, &b_ncell);
   fChain->SetBranchAddress("nevt_total", &nevt_total, &b_nevt_total);
   fChain->SetBranchAddress("identifier", identifier, &b_identifier);
   fChain->SetBranchAddress("layer", layer, &b_layer);
   fChain->SetBranchAddress("region", region, &b_region);
   fChain->SetBranchAddress("ieta", ieta, &b_ieta);
   fChain->SetBranchAddress("eta", eta, &b_eta);
   fChain->SetBranchAddress("phi", phi, &b_phi);
   fChain->SetBranchAddress("nevt", nevt, &b_nevt);
   fChain->SetBranchAddress("average", average, &b_average);
   fChain->SetBranchAddress("rms", rms, &b_rms);
   fChain->SetBranchAddress("reference", reference, &b_reference);
   Notify();
}

Bool_t AverageMinBias::Notify()
{
   return kTRUE;
}

void AverageMinBias::Show(Long64_t entry)
{
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t AverageMinBias::Cut(Long64_t entry)
{
   return 1;
}
#endif // #ifdef AverageMinBias_cxx
