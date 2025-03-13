/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
#include <string>


class TestDriver {
public:
  TestDriver( const std::string& name,
              const std::string& coll_type = "RootCollection",
              const std::string& connection = "" );

  ~TestDriver() {}

  void write ATLAS_NOT_THREAD_SAFE ();

  void read ATLAS_NOT_THREAD_SAFE ();

private:
  std::string m_name;
  std::string m_type;
  std::string m_connection;
};
