/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARONLINE_SUPERCELLID_H
#define LARONLINE_SUPERCELLID_H

#include "LArIdentifier/LArOnlineID_Base.h"
#include "AthenaKernel/BaseInfo.h"
#include "string.h"
#include <vector>
#include <algorithm>

#include <iostream>


class IdentifierHash;
class Range;

class LArOnline_SuperCellID : public LArOnlineID_Base
{
 public:        

  typedef Identifier::size_type  size_type;
  
  /** 
   * @brief Default constructor
   */
  LArOnline_SuperCellID();
  /** 
   * @brief Default destructor
   */
  ~LArOnline_SuperCellID();


  int  initialize_from_dictionary (const IdDictMgr& dict_mgr) override final;

  bool  isHECchannel     (const HWIdentifier id) const override final; // differs for Maini and DT
  bool  isEMECchannel    (const HWIdentifier id) const override final; // differs for Maini and DT
  bool  isEMECIW         (const HWIdentifier id) const override final; // differs for Main and DT
  bool  isEMECOW         (const HWIdentifier id) const override final; // differs for Main and DT
};

CLASS_DEF( LArOnline_SuperCellID , 115600394 , 1 )
SG_BASES( LArOnline_SuperCellID, LArOnlineID_Base );

#endif // LARONLINE_ID_H

