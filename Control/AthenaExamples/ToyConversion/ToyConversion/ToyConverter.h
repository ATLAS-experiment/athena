/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TOYCONVERSION_TOYCONVERTER_H
#define TOYCONVERSION_TOYCONVERTER_H

#include "ToyConversion/ToyConversionSvc.h"

#include "GaudiKernel/Converter.h"
#include "GaudiKernel/StatusCode.h"

#include "AthenaKernel/ClassID_traits.h"
#include "AthenaKernel/StorableConversions.h"

template <class Cnv> class CnvFactory;

/** @class ToyConverter
 * @brief  a toy converter template that creates a default instance of DATA
 * @param  DATA the type to be "converted". Must have a default constructor
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */
template <typename DATA>
class ToyConverter : public Converter {
public:
  // tran->per
  //  StatusCode createRep(DataObject* pO, IOpaqueAddress*& pA);

  // per->tran
  virtual StatusCode createObj(IOpaqueAddress*, DataObject *& pO) override {
    //could alse get DATA (perhaps as std::any) from the IOA
    pO = SG::asStorable(new DATA);
    return StatusCode::SUCCESS;
  }

  static CLID classID() { return ClassID_traits<DATA>::ID(); }

  virtual long int repSvcType() const override;
  static long int storageType();

  ToyConverter(ISvcLocator* svcloc) :
    Converter(storageType(), classID(), svcloc) {}
};


template <typename DATA>
long int ToyConverter<DATA>::storageType() {
  return ToyConversionSvc::storageType();
}
template <typename DATA>
long int ToyConverter<DATA>::repSvcType() const {
  return storageType();
}

#endif // TOYCONVERSION_TOYCONVERTER_H
