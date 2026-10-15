/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AssociationUtils/FEMatcher.h"

namespace {

  static const SG::ConstAccessor<ElementLink<xAOD::IParticleContainer>> ACC_partA("FE_ParticleA");
  static const SG::ConstAccessor<ElementLink<xAOD::IParticleContainer>> ACC_partB("FE_ParticleB");
  static const SG::ConstAccessor<int> ACC_typeA("FE_ParticleA_Type");
  static const SG::ConstAccessor<int> ACC_typeB("FE_ParticleB_Type");
  static const SG::ConstAccessor<float> ACC_sharedEc("FE_SharedEc");
  static const SG::ConstAccessor<float> ACC_sharedEn("FE_SharedEn");
  static const SG::ConstAccessor<float> ACC_totAc("FE_TotAc");
  static const SG::ConstAccessor<float> ACC_totAn("FE_TotAn");
  static const SG::ConstAccessor<float> ACC_totBc("FE_TotBc");
  static const SG::ConstAccessor<float> ACC_totBn("FE_TotBn");

}

namespace ADD_CMODE(ORUtils) 
{

  //---------------------------------------------------------------------------
  // FEMatcher constructor
  //---------------------------------------------------------------------------
  FEMatcher::FEMatcher(){};

  //---------------------------------------------------------------------------
  // Check object match
  //---------------------------------------------------------------------------
  bool FEMatcher::objectsMatch(const xAOD::IParticle* tPar, const xAOD::IParticle* rPar, xAOD::MissingETAssociationMap feMap, bool /*swapArgs*/) const {

    // Find whether FE energy fraction is above a threshold
    // Loop over metmap and do the calculation
    for (auto assoc : feMap) {
      const xAOD::IParticle* pA = *(ACC_partA(*assoc));
      const xAOD::IParticle* pB = *(ACC_partB(*assoc));

      if (pA == tPar && pB == rPar) {
        double sharedE = ACC_sharedEc(*assoc) + ACC_sharedEn(*assoc);
        double totEA = ACC_totAc(*assoc) + ACC_totAn(*assoc);
        double frac_A = totEA != 0 ? sharedE / totEA : 0.0;
        if (frac_A > 0.8) return true;
        else return false;
      }
      else if (pB == tPar && pA == rPar) {
          double sharedE = ACC_sharedEc(*assoc) + ACC_sharedEn(*assoc);
          double totEB = ACC_totBc(*assoc) + ACC_totBn(*assoc);
          double frac_B = totEB != 0 ? sharedE / totEB : 0.0;
          if (frac_B > 0.8) return true;
          else return false;
      }
    }
    return false;
  }  
} // ADD_CMODE(ORUtils)