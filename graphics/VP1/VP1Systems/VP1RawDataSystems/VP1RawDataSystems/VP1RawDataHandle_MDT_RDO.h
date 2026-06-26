/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class VP1RawDataHandle_MDT_RDO            //
//                                                            //
//  Description: Handle for MDT RDO's                         //
//                                                            //
//  Author: Riccardo Maria BIANCHI (riccardo.maria.bianchi@cern.ch) 
//  Initial version: January 2024       
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef VP1RawDataHandle_MDT_RDO_H
#define VP1RawDataHandle_MDT_RDO_H

#include "VP1RawDataSystems/VP1RawDataHandleBase.h"

#include "MuonReadoutGeometry/MdtReadoutElement.h"

class MdtDigit;

namespace InDetDD { class TRT_BaseElement; }

class VP1RawDataHandle_MDT_RDO : public VP1RawDataHandleBase {
public:

  VP1RawDataHandle_MDT_RDO(VP1RawDataCollBase*,const MdtDigit*);
  virtual ~VP1RawDataHandle_MDT_RDO();
  QStringList clicked(bool verbose) const;

protected:
  SoNode * buildShape();
  SoTransform * buildTransform();
  const MdtDigit* m_data;
  const MuonGM::MdtReadoutElement * element() const;//null in case of errors
};

#endif
