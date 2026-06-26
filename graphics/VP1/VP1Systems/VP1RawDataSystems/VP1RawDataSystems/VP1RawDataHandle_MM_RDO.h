/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class VP1RawDataHandle_MM_RDO            //
//                                                            //
//  Description: Handle for MM RDO's                         //
//                                                            //
//  Author: Riccardo Maria BIANCHI (riccardo.maria.bianchi@cern.ch) 
//  Initial version: December 2024       
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef VP1RawDataHandle_MM_RDO_H
#define VP1RawDataHandle_MM_RDO_H

#include "VP1RawDataSystems/VP1RawDataHandleBase.h"

#include "MuonReadoutGeometry/MMReadoutElement.h"

class MmDigit;

class VP1RawDataHandle_MM_RDO : public VP1RawDataHandleBase {
public:

  VP1RawDataHandle_MM_RDO(VP1RawDataCollBase*,const MmDigit*);
  virtual ~VP1RawDataHandle_MM_RDO();
  QStringList clicked(bool verbose) const;

protected:
  SoNode * buildShape();
  SoTransform * buildTransform();
  const MmDigit* m_data;
  const MuonGM::MMReadoutElement * element() const;//null in case of errors
};

#endif
