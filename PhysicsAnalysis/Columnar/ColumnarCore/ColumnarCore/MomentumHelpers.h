/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_MOMENTUM_HELPERS_H
#define COLUMNAR_CORE_MOMENTUM_HELPERS_H

#include <ColumnarCore/MomentumAccessors.h>
#include <FourMomUtils/xAODP4Helpers.h>

namespace columnar
{
  /// @file helpers for use with columnar momentum accessors
  ///
  /// Essentially this is just a copy of what we have for xAOD types,
  /// translated to columnar momentum accessors.

  /** delta Phi in range [-pi,pi[ */
  inline
  double deltaPhi( double phiA, double phiB )
  {
    return  -remainder( -phiA + phiB, 2*M_PI );
  }



  /// Computes efficiently @f$ \Delta{y} @f$
  template<ContainerIdConcept CI1,ContainerIdConcept CI2,typename CM>
  double deltaRapidity( const MomentumAccessors<CI1,CM>& momAcc1, ObjectId<CI1,CM> p1, const MomentumAccessors<CI2,CM>& momAcc2, ObjectId<CI2,CM> p2, bool useRapidity=true )
  {
    if (useRapidity)
      return momAcc1.rapidity(p1) - momAcc2.rapidity(p2);
    else
      return momAcc1.eta(p1) - momAcc2.eta(p2);
  }



  /// Check if 2 particles are in a @f$ \Delta{R} @f$ cone
  /// @param dR [in] @f$ \Delta{R} @f$
  /// @return true if they are
  template<ContainerIdConcept CI1,ContainerIdConcept CI2,typename CM>
  bool isInDeltaR( const MomentumAccessors<CI1,CM>& momAcc1, ObjectId<CI1,CM> p1, const MomentumAccessors<CI2,CM>& momAcc2, ObjectId<CI2,CM> p2,
                    double dR, bool useRapidity=true )
  {
    const double dPhi = std::abs( xAOD::P4Helpers::deltaPhi(momAcc1.phi(p1),momAcc2.phi(p2)) ); // in [-pi,pi)
    if ( CxxUtils::fpcompare::greater(dPhi,dR) ) return false;        // <==
    const double dRapidity = std::abs( deltaRapidity(momAcc1,p1,momAcc2,p2,useRapidity) );
    if ( CxxUtils::fpcompare::greater(dRapidity,dR) ) return false;        // <==
    const double deltaR2 = dRapidity*dRapidity + dPhi*dPhi;
    if ( CxxUtils::fpcompare::greater(deltaR2,dR*dR) ) return false;  // <==
    return true;
  }
}

#endif