/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef TRT_GEOMODEL_TRTSTRAWSTATUSACCESSOR_H
#define TRT_GEOMODEL_TRTSTRAWSTATUSACCESSOR_H

#include "Identifier/Identifier.h"
#include <map>

class TRTStrawStatusAccessor {
public:
  TRTStrawStatusAccessor() = default;
  ~TRTStrawStatusAccessor() = default;

  // Read the map from the input ASCII file
  void fill(const std::string& path);

  // Get the status
  int status(const Identifier& id) const;
private:
  using Key = Identifier::value_type;
  std::map<Key,int> m_statusMap;
};

#endif
