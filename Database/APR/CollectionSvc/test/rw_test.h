/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "StorageSvc/DbType.h"

#include "CxxUtils/checker_macros.h"
#include <string>


class TestDriver {
public:
  explicit TestDriver( const std::string& name,
              const pool::DbType& coll_type = pool::ROOT_StorageType.type(),
              const std::string& connection = "" );

  ~TestDriver() {}

  void write ATLAS_NOT_THREAD_SAFE ();

  void read ATLAS_NOT_THREAD_SAFE ();

private:
  std::string m_name;
  pool::DbType m_type;
  std::string m_connection;
};
