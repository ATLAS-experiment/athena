/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class VP1RawDataHandle_MDT_RDO            //
//                                                            //
//  Description: Handle for TRT RDO's                         //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: April 2008 (rewritten January 2009)      //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef VP1RawDataHandle_MDT_RDO_H
#define VP1RawDataHandle_MDT_RDO_H

#include "VP1RawDataSystems/VP1RawDataHandleBase.h"

class MdtDigit;

namespace InDetDD { class TRT_BaseElement; }

class VP1RawDataHandle_MDT_RDO : public VP1RawDataHandleBase {
public:

  VP1RawDataHandle_MDT_RDO(VP1RawDataCollBase*,const MdtDigit*);
  virtual ~VP1RawDataHandle_MDT_RDO();
  QStringList clicked(bool verbose) const;

  // VP1RawDataFlags::InDetPartsFlags inInDetParts() const;

  // bool highThreshold() const;
  // double timeOverThreshold() const;

protected:
  SoNode * buildShape();
  SoTransform * buildTransform();
  const MdtDigit* m_data;
  const Muon::MuonDetectorManager * element() const;//null in case of errors
  // int strawID() const;//-1 in case of errors

};

#endif
