/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_TEST_SHAREDIOCNVSVCSTUB_H
#define ATHENASERVICES_TEST_SHAREDIOCNVSVCSTUB_H

#include "AthenaBaseComps/AthCnvSvc.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/StorableConversions.h"
#include "GaudiKernel/GenericAddress.h"

#include <string>
#include <vector>

/// Object created by SharedIOCnvSvcStub; carries the address it was read from.
struct SharedIOStubObject {
  std::string key;
  unsigned long num{0};
};
CLASS_DEF( SharedIOStubObject, 245100004, 1 )

/**
 * @brief Conversion service stub for the MetaDataSvc::shmProxy path in MetaDataSvc_test.
 *
 * MetaDataSvc only accepts an AddressCreator called AthenaPoolSharedIOCnvSvc
 * for shmProxy. Configured under that name, and registered with
 * EventPersistencySvc, this stub creates GenericAddresses and turns them into
 * SharedIOStubObjects, whatever the CLID. It records the parameters of every
 * address it created as "<par0>|<par1>|<par2>|<ipar0>".
 */
class SharedIOCnvSvcStub : public AthCnvSvc {
public:
  static constexpr long storageType = 0x5A5A;

  SharedIOCnvSvcStub( const std::string& name, ISvcLocator* svcLoc )
    : AthCnvSvc( name, svcLoc, storageType ) {}

  using AthCnvSvc::createAddress;
  virtual StatusCode createAddress( long svcType, const CLID& clid,
                                    const std::string* par, const unsigned long* ipar,
                                    IOpaqueAddress*& addr ) override {
    m_addresses.push_back( par[0] + "|" + par[1] + "|" + par[2] + "|" + std::to_string( ipar[0] ) );
    addr = new GenericAddress( svcType, clid, par[0], par[1], ipar[0], ipar[1] );
    return StatusCode::SUCCESS;
  }

  virtual StatusCode createObj( IOpaqueAddress* addr, DataObject*& obj ) override {
    ++m_created;
    obj = SG::asStorable( new SharedIOStubObject{ addr->par()[1], addr->ipar()[0] } );
    return StatusCode::SUCCESS;
  }

  const std::vector<std::string>& addresses() const { return m_addresses; }
  int created() const { return m_created; }

private:
  std::vector<std::string> m_addresses;
  int m_created{0};
};

#endif
