/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DBSTRING_H
#define DBSTRING_H 1

#include <string>

/// Declaration of DbString class
class DbString : public std::string  {
  public:
    /// Standard Constructor
    DbString() = default;
    /// Constructor with initializer
    DbString(const std::string& s) {  assign(s); }
    /// Standard destructor
    virtual ~DbString() = default;
};

#endif
