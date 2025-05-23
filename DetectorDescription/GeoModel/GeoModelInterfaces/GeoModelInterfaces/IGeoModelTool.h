/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELINTERFACES_IGEOMODELTOOL_H
#define GEOMODELINTERFACES_IGEOMODELTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include "CxxUtils/checker_macros.h"

class IGeoModelTool : public virtual IAlgTool {
public:

    DeclareInterfaceID( IGeoModelTool, 1, 0 );

    // Abstract interface method(s)
    virtual StatusCode create() = 0;

    // This method is designed to perform following tasks:
    //    1. Release detector manager from the Detector Store
    //    2. Do any extra clean up tasks if necessary
    virtual StatusCode clear() = 0;

    // Register callback function on ConDB object
    virtual StatusCode registerCallback ATLAS_NOT_THREAD_SAFE () = 0;

    // Callback function itself
    virtual StatusCode align ATLAS_NOT_THREAD_SAFE (IOVSVC_CALLBACK_ARGS) = 0;

};

#endif // GEOMODELINTERFACES_IGEOMODELTOOL_H
