/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCCONDDATA_H
#define ATHEXRPCLOOP_RPCCONDDATA_H

/**
 * @file RpcCondData.h
 * @brief A trivial conditions payload, to prove the synthetic-event recipe
 *        keeps conditions access working.
 *
 * The whole premise of this POC is that the server side is a *full* Athena job
 * -- conditions, geometry, magnetic field -- rather than a bare model server.
 * Nothing enforces that until something actually reads a conditions object out
 * of a synthetic event, which is what this exists for. It carries an integer so
 * the tests can assert exact arithmetic rather than a float comparison.
 */

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

#include <cstdint>

namespace AthExRpc {

class RpcCondData {
public:
  RpcCondData() = default;
  explicit RpcCondData( int64_t offset ) : m_offset( offset ) {}

  int64_t offset() const { return m_offset; }

private:
  int64_t m_offset = 0;
};

}  // namespace AthExRpc

CLASS_DEF( AthExRpc::RpcCondData, 213558423, 1 )
CONDCONT_DEF( AthExRpc::RpcCondData, 213558424 );

#endif  // ATHEXRPCLOOP_RPCCONDDATA_H
