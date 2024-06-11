/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TOYCONVERSION_TOYCONVERSIONSVC_H
#define TOYCONVERSION_TOYCONVERSIONSVC_H

#include "GaudiKernel/ConversionSvc.h"

/** @class ToyConversionSvc 
 * @brief  a toy that implements IConversionSvc
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */
class ToyConversionSvc : public ConversionSvc {
public:
  static long int storageType();
  virtual long int repSvcType() const override;

  ToyConversionSvc(const std::string &name, ISvcLocator* svc);
  virtual ~ToyConversionSvc();
};

#endif
