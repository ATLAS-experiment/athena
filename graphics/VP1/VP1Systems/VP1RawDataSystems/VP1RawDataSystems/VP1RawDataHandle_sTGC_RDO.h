/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class VP1RawDataHandle_sTGC_RDO           //
//                                                            //
//  Description: Handle for sTGC RDO's                         //
//                                                            //
//  Author: Riccardo Maria BIANCHI (riccardo.maria.bianchi@cern.ch) 
//  Initial version: December 2024       
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef VP1RawDataHandle_sTGC_RDO_H
#define VP1RawDataHandle_sTGC_RDO_H

#include "VP1RawDataSystems/VP1RawDataHandleBase.h"

#include "MuonReadoutGeometry/sTgcReadoutElement.h"

class sTgcDigit;

// namespace InDetDD { class TRT_BaseElement; }

class VP1RawDataHandle_sTGC_RDO : public VP1RawDataHandleBase {
public:

  VP1RawDataHandle_sTGC_RDO(VP1RawDataCollBase*,const sTgcDigit*);
  virtual ~VP1RawDataHandle_sTGC_RDO();
  QStringList clicked(bool verbose) const;

  // VP1RawDataFlags::InDetPartsFlags inInDetParts() const;

  // bool highThreshold() const;
  // double timeOverThreshold() const;

protected:
  SoNode * buildShape();
  SoTransform * buildTransform();
  const sTgcDigit* m_data;
  const MuonGM::sTgcReadoutElement * element() const;//null in case of errors
  // int strawID() const;//-1 in case of errors

  int m_channelType{-1};
  std::string m_channelTypeStr{""};

};

#endif
