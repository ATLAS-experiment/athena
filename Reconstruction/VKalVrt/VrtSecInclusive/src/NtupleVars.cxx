/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "VrtSecInclusive/NtupleVars.h"
#include "VrtSecInclusive/IntersectionPos.h"

#include "TTree.h"
#include <iostream>

namespace VKalVrtAthena {
  
  //____________________________________________________________________________________________________
  NtupleVars::NtupleVars() = default;

  //____________________________________________________________________________________________________
  NtupleVars::~NtupleVars() = default;

  //____________________________________________________________________________________________________
  void NtupleVars::setupNtupleVariables() {}
  
  
  //____________________________________________________________________________________________________
  void NtupleVars::deleteNtupleVariables() {}
  
  
  //____________________________________________________________________________________________________
  void NtupleVars::branchNtuple( TTree *tree ) {
    
    for( const auto& pair : m_varHolder ) {
      pair.second->branch_tree( pair.first, tree );
    }
      
    /*
    for( auto *layer : m_RecoTrk_Int ) {
      layer->setBranchAddress( tree, "RecoTrk_Int" );
    }
    */
      
    /*
    for( auto *layer : m_SVTrk_Int ) {
      layer->setBranchAddress( tree, "SVTrk" );
    }
    */
      

  }
  
  //____________________________________________________________________________________________________
  void NtupleVars::clear() {
    for( auto& pair : m_varHolder ) {
      pair.second->clear();
    }
    
    // for( auto *layer : m_RecoTrk_Int ) {
    //   layer->clear();
    // }
    
    // for( auto *layer : m_SVTrk_Int ) {
    //   layer->clear();
    // }
    
  }
  
}
