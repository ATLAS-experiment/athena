/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-
#ifndef LARCOOLCONDITIONS_LARNOISEFLAT_H
#define LARCOOLCONDITIONS_LARNOISEFLAT_H

#include "LArElecCalib/ILArNoise.h" 
#include "LArCOOLConditions/LArSingleFloatBlob.h"
#include "LArCOOLConditions/LArCondFlatBase.h"


class CondAttrListCollection;

class LArNoiseFlat: public ILArNoise,
		    public LArCondFlatBase,
		    public LArSingleFloatBlob {

public:
  LArNoiseFlat(); 
  LArNoiseFlat(const CondAttrListCollection* attrList);

  virtual ~LArNoiseFlat();

  bool good() const { return m_isInitialized && m_nChannels>0; }
  
  // retrieving LArNoise using online ID  
  virtual const float& noise(const HWIdentifier& chid, int gain) const;

private:
  //static const float errorcode;

};
#include "AthenaKernel/CondCont.h"
CLASS_DEF( LArNoiseFlat ,56185552 , 1 )
CONDCONT_DEF( LArNoiseFlat, 123133640, ILArNoise );

#endif 
