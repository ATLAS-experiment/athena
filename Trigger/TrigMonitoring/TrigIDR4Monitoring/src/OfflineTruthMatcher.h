/** emacs: this is -*- c++ -*- 
 **
 **     @file    OfflineTruthMatcher.h        
 **                   
 **     @author  sutt
 **     @date    Fri  2 Oct 2026 18:14:07 BST
 **
 **     $Id: OfflineTruthMatcher.h, v0.0   Fri  2 Oct 2026 18:14:07 BST sutt $
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration **
 **/


#ifndef  IDTPM2_OFFLINETRUTHMATCHER_H
#define  IDTPM2_OFFLINETRUTHMATCHER_H

#include <iostream>
#include <cmath>

#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "xAODCore/AuxStoreAccessorMacros.h"
#include "AthLinks/ElementLink.h"

#include "TrigInDetAnalysis/TrackAssociator.h"
#include "TrigInDetAnalysisUtils/BestMatcher.h"

namespace IDTPM2 {

  
/// decorates types with the truth match information 
template<typename T>    
class decorator {
    
  inline static const SG::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> m_acc = {"truthParticleLink"};
  inline static const SG::ConstAccessor<float> m_probAcc = {"truthMatchProbability"};

public:

  template<typename Tp>
  explicit decorator( const Tp* t ) : m_t(t->template as<T>()) { }
  
  explicit operator bool() const { return m_t!=0; }
  
  const xAOD::TruthParticle* truthMatch() const {
    if ( m_t && m_acc.isAvailable(*m_t) ) { 
      const ElementLink<xAOD::TruthParticleContainer>& link = m_acc(*m_t);
      if (link.isValid()) return *link;
    }
    return 0; 
  }
  
  double discriminant() const {
    if ( m_t && m_probAcc.isAvailable(*m_t) ) {
      float prob = m_probAcc(*m_t);
      if ( prob ) return std::log10(1.0/prob);
    }
    return std::numeric_limits<double>::max();
  }
  
private:
  
  const T* m_t;
  
};



  
template<typename T, typename R=T>
class TOfflineTruthMatcher : public BestMatcher<T,R> {
  
public:

  TOfflineTruthMatcher( const std::string& name, double d )  :
    BestMatcher<T,R>(name, std::log10(1.0/d)) {
  }

  virtual ~TOfflineTruthMatcher() { } 

  virtual std::unique_ptr<TIDA::Associator<T,R>> uclone() const override { return std::make_unique<TOfflineTruthMatcher<T,R>>(*this); }

  virtual double distance( const T* t0, const R* t1 ) const override {
    decorator<xAOD::TrackParticle> tp(t1);
    if ( tp && tp.truthMatch() == t0->template as<xAOD::TruthParticle>() ) return tp.discriminant();
    return std::numeric_limits<double>::max();
  }
      
};

  
}
  

#endif  // IDTPM2_OFFLINETRUTHMATCHER_H 










