//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloClusterDeleter.h"


StatusCode CaloClusterDeleter::execute (const EventContext &, xAOD::CaloClusterContainer * cluster_collection) const
{
  cluster_collection->clear();

  return StatusCode::SUCCESS;

}

