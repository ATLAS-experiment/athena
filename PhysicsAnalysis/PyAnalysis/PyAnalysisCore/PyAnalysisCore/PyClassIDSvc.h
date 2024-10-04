/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PYANALYSISCORE_PYCLASSIDSVC_H
#define PYANALYSISCORE_PYCLASSIDSVC_H

/**
   A wrapper for ClassIDSvc

    @author Tadashi Maeno
*/
 
#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/errorcheck.h"

struct PyClassIDSvc
{
  /// get type name associated with clID (if any)
  static std::string getTypeNameOfID (const unsigned int id)
  {
    std::string name;
    SmartIF<IClassIDSvc> clidSvc{ Gaudi::svcLocator()->service("ClassIDSvc") };
    CHECK_WITH_CONTEXT( clidSvc.isValid(), "PyClassIDSvc", name );

    CHECK_WITH_CONTEXT( clidSvc->getTypeNameOfID (id, name), "PyClassIDSvc", name );

    return name;
  }
};

#endif

  
