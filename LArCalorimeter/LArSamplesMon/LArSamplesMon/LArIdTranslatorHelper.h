/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LArIdTranslatorHelper
*/

#ifndef LArSamples_LArIdTranslatorHelper_H
#define LArSamples_LArIdTranslatorHelper_H


#include <vector>
#include <cstdlib>
#include <string>
#include <memory>
#include "RtypesCore.h" //ROOT data types, Int_t etc
#include "CxxUtils/checker_macros.h"

class TString;
class TCanvas;
class TH1;
class TH2;
class TTree;
class TFile;
class TH2I;


class ATLAS_NOT_THREAD_SAFE LArIdTranslatorHelper
{
  public:

    LArIdTranslatorHelper(const TString& inputfile);
    ~LArIdTranslatorHelper();
    
    // variables
    Int_t bec,side,ft,sl,ch,sa,part;
    Float_t eta,phi,x,y,z;
    ULong64_t onlid,offlid,ttid;
    std::vector<int>* hvid;
    Long_t entry;
    Int_t binetadqm,binphidqm1,binphidqm2;

    // functions
    int GetPartitionLayerIndex(const int partition, const int layer) const;
    bool FindChannel(const int index, float eta, float phi);
    bool LoadIdTranslator(const TString& file);

    bool IsInitialized(){ return m_kInitialized; }
    TCanvas* CaloPartitionLayerDisplay(TH2** h,const Char_t* title="",bool kLogz=true);
    TH2* GetCaloPartitionLayerMap(const int index,bool kProfile=false);
    const Char_t* GetPartitonLayerName(const int index);
    void MakeTranslatorMapping(const char* inputtreefile,const char* inputhistfile,const int run);
    bool IsHVLine(const int hvline) const;

    Int_t GetNpl(){ return m_nPartitionLayers; }

    TTree* Tree() { return m_tree; }
    
  private:

    bool m_kInitialized;
    TTree* m_tree;
    TFile* m_file;
    Int_t m_ntotal,m_extrabins;

    Int_t m_canvas_counts,m_clonemap_counts;
    Char_t m_namebuf[128]{};

    Int_t m_nPartitionLayers; // number of partition layers (nominally 30, from CaloCell monitoring)
    std::vector<std::string> m_PartitionLayers;
    Int_t m_nHistCategories; // categories used for mapping performances
    std::vector<std::string> m_HistCategories;
    std::vector<std::vector<std::unique_ptr<TH2I> > > m_HistCellmaps;  // mapping histograms loaded from input rootfile
};

#endif

