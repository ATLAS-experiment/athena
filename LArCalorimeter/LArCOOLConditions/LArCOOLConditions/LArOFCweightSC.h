/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-
#ifndef LARCOOLCONDITIONS_OFCWEIGHTSC_H
#define LARCOOLCONDITIONS_OFCWEIGHTSC_H

#include "LArCOOLConditions/LArSingleFloatBlob.h"
#include "LArCOOLConditions/LArCondSuperCellBase.h"

class CondAttrListCollection;

class LArOFCweightSC:
		   public LArCondSuperCellBase,
		   public LArSingleFloatBlob {

public:
  LArOFCweightSC(); 
  LArOFCweightSC(const CondAttrListCollection* attrList);

  virtual ~LArOFCweightSC() = default;

  bool good() const { return m_isInitialized && m_nChannels>0; }
  
  // retrieving weight using online ID  
  const float& getW(const HWIdentifier& chid) const;

private:
};

#include "AthenaKernel/CondCont.h"
CLASS_DEF( LArOFCweightSC , 224728897, 1 )
CONDCONT_DEF( LArOFCweightSC, 81787215 );

#endif 
