/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef LARCOOLCONDITIONS_LARDSPTHRESHOLDSFLAT_H
#define LARCOOLCONDITIONS_LARDSPTHRESHOLDSFLAT_H

#include "LArCOOLConditions/LArCondFlatBase.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

class IdentifierHash;
class AthenaAttributeList;
class HWIdentifier;

class LArDSPThresholdsFlat: public LArCondFlatBase {
  
private: 
  LArDSPThresholdsFlat(); //private default constructor
  
public:
  LArDSPThresholdsFlat(const AthenaAttributeList* attrList);
  bool good() const { return m_isInitialized && m_nChannels>0; }

  
  // retrieving DSPThresholds using online ID
  
  float tQThr(const HWIdentifier&  CellID) const;  
  float samplesThr(const HWIdentifier&  CellID) const;
  float trigSumThr(const HWIdentifier&  CellID) const;


  float tQThrByHash(const IdentifierHash& h) const;  
  float samplesThrByHash(const IdentifierHash& h) const;  
  float trigSumThrByHash(const IdentifierHash& h) const;  


private:

  void readBlob(const AthenaAttributeList* attr);

  unsigned m_nChannels{};
  const float* m_ptQThr{};
  const float* m_psamplesThr{};
  const float* m_ptrigSumThr{};

};

CLASS_DEF( LArDSPThresholdsFlat, 194681315 ,1  )

CONDCONT_DEF( LArDSPThresholdsFlat, 148006985 );
#endif 
