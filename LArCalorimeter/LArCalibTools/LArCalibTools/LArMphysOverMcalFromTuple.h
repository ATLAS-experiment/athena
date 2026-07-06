/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARMPHYSOVERMCALFROMTuple_H
#define LARMPHYSOVERMCALFROMTuple_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"



#include <vector>
#include <string>

/** @class LArMphysOverMcalFromTuple

This algorithm allows to build a LArMphysOverMcalComplete container from the 
content of a root ntuple. 

The root tree should be named "outfit" and contain the following branches :
- BarAC :  int:  0 = barrel C   1 = barrel A
- FT :  int: feedthrough number (0-31) 
- Channel :  int:  channel number (0-14)
- Slot :  int: slot number (1-14)
- PhysOCal :  float :  value of Mphy/Mcal
 */


class LArMphysOverMcalFromTuple : public AthReentrantAlgorithm
{
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual ~LArMphysOverMcalFromTuple();

  //standard algorithm methods

  /// implements IAlgorithm::execute()  : Does nothing
  virtual StatusCode execute(const EventContext&) const override {return StatusCode::SUCCESS;}

  /// IAlgorithm::stop() : Where the action takes place...
  virtual StatusCode stop() override;
 
 private:
  /// list of input ntuple file names 
  StringArrayProperty m_root_file_names { this, "FileNames", {} };

  /// key of the PhysWave collection in StoreGate
  StringProperty m_store_key { this, "StoreKey", "FROMTUPLE" };

};

#endif
