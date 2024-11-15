///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// ILArNoisyROTool.h 
// Header file for class ILArNoisyROTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef LARELECCALIB_ILARNOISYROTOOL_H
#define LARELECCALIB_ILARNOISYROTOOL_H 1

// STL includes
#include <memory>

// FrameWork includes
#include "GaudiKernel/IAlgTool.h"

// Forward declaration
class CaloCellContainer;
class LArNoisyROSummary;
class HWIdentifier;
class EventContext;
class LArHVNMap;
class CaloDetDescrManager;
class LArHVIdMapping;

static const InterfaceID IID_ILArNoisyROTool("ILArNoisyROTool", 2, 0);

class ILArNoisyROTool
  : virtual public ::IAlgTool
{ 
 public: 

  /** Destructor: 
   */
  virtual ~ILArNoisyROTool() {};
  static const InterfaceID& interfaceID();

  virtual 
  std::unique_ptr<LArNoisyROSummary> process(const EventContext& ctx, const CaloCellContainer*, const std::set<unsigned int>* knownBadFebs, const std::vector<HWIdentifier>* knownMNBFebs, const LArHVNMap* hvmap=nullptr, const CaloDetDescrManager* cddm=nullptr, const LArHVIdMapping* hvid=nullptr) const =0;

}; 

inline const InterfaceID& ILArNoisyROTool::interfaceID() 
{ 
   return IID_ILArNoisyROTool; 
}


#endif //> !LARELECCALIB_ILARNOISYROTOOL_H
