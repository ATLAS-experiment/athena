/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARELECCALIB_ILARHVMAPTOOL_H
#define LARELECCALIB_ILARHVMAPTOOL_H 1

// FrameWork includes
#include "GaudiKernel/IAlgTool.h"

// Forward declaration
class Identifier;
class HWIdentifier;
class CaloDetDescrManager;
class MsgStream;

static const InterfaceID IID_ILArHVMapTool("ILArHVMapTool", 1, 0);

class ILArHVMapTool
  : virtual public ::IAlgTool
{ 
 public: 

  /** Destructor: 
   */
  virtual ~ILArHVMapTool() {};
  static const InterfaceID& interfaceID();

  virtual void GetHVLines(const Identifier& id, const CaloDetDescrManager *calodetdescrmgr,  
                     std::vector<int> &hvLineVec) const=0;
  virtual void GetHVLines(const Identifier& id, const CaloDetDescrManager *calodetdescrmgr,  
                     std::vector<HWIdentifier> &hvLineId) const=0;

}; 

inline const InterfaceID& ILArHVMapTool::interfaceID() 
{ 
   return IID_ILArHVMapTool; 
}


#endif 
