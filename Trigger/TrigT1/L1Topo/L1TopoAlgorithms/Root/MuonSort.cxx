/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//  MuonSort.cxx
//  TopoCore
//  Created by Joerg Stelzer on 11/10/12.
//  algorithm to create sorted lists for muons, et order applied
//
#include "L1TopoAlgorithms/MuonSort.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/MuonTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(MuonSort)

bool SortByEtLargestM(TCS::GenericTOB* tob1, TCS::GenericTOB* tob2)
{
  //Order the TOBs according to Et (high to low), then (in case of equal ET) side (first A, then C). Further ambiguity resolution depends on details of MUCTPI and are currently not taken into account (to be seen if necessary).
  
  //highest priority: ET
  if (tob1->Et() > tob2->Et()) return true;
  if (tob1->Et() < tob2->Et()) return false;
  //second criterion: A side before C side (here: emulated via signed eta coordinate)
  if (tob1->eta() > tob2->eta()) return true;
  if (tob1->eta() < tob2->eta()) return false; //explicitly indicate tob1 < tob2 in case additional criteria are added
  return false;
  
}


// constructor
TCS::MuonSort::MuonSort(const std::string & name) : SortingAlg(name) {
   defineParameter( "InputWidth", 32 ); // for FW
   defineParameter( "InputWidth1stStage", 16 ); // for FW
   defineParameter( "OutputWidth", 6 );
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 196 );
   defineParameter( "InnerCoinCut", 0 );
   defineParameter( "FullStationCut", 0 );
   defineParameter( "GoodMFieldCut", 0 );
}


// destructor
TCS::MuonSort::~MuonSort() {}


TCS::StatusCode
TCS::MuonSort::initialize() {
   m_numberOfMuons = parameter("OutputWidth").value();
   m_minEta = parameter("MinEta").value();
   m_maxEta = parameter("MaxEta").value();
   m_InnerCoinCut = parameter("InnerCoinCut").value();  
   m_FullStationCut = parameter("FullStationCut").value();
   m_GoodMFieldCut = parameter("GoodMFieldCut").value();
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TCS::MuonSort::sort(const InputTOBArray & input, TOBArray & output) {

   const MuonTOBArray & muons = dynamic_cast<const MuonTOBArray&>(input);

   // fill output array with GenericTOB built from muons
   for(MuonTOBArray::const_iterator muon = muons.begin(); muon!= muons.end(); ++muon ) {

      if (parType_t(std::abs((*muon)-> eta())) < m_minEta) continue; 
      if (parType_t(std::abs((*muon)-> eta())) > m_maxEta) continue;
      
    // Apply flag selection only for TGC muons. The flag selection is applied only if the corresponding parameter from the menu is 1.  
    if ( parType_t((*muon)->isTGC()) )
      {
	if(m_InnerCoinCut == 1 && ( ! ((int)parType_t((*muon)->innerCoin()) == (int)m_InnerCoinCut ) ) ) continue;
	if(m_FullStationCut == 1 && ( ! ((int)parType_t((*muon)->bw2or3()) == (int)m_FullStationCut ) ) ) continue;
	if(m_GoodMFieldCut == 1 && ( ! ((int)parType_t((*muon)->goodMF()) == (int)m_GoodMFieldCut ) ) ) continue;
      }

      const GenericTOB gtob(**muon);
      output.push_back( gtob );
   }

   // sort
   output.sort(SortByEtLargestM);
   

   // keep only max number of muons
   int par = m_numberOfMuons;
   unsigned int maxNumberOfMuons = std::clamp(par, 0, std::abs(par));
   if(maxNumberOfMuons>0) {
      while( output.size()> maxNumberOfMuons ) {
         if (output.size() == (maxNumberOfMuons+1)) {
            bool isAmbiguous = output[maxNumberOfMuons-1].EtDouble() == output[maxNumberOfMuons].EtDouble();
            if (isAmbiguous) { output.setAmbiguityFlag(true); }
         }
         output.pop_back();
      }
   }
   return TCS::StatusCode::SUCCESS;
}

