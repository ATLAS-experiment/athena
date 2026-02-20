/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELINTERFACES_IGEOMODELTOOL_H
#define GEOMODELINTERFACES_IGEOMODELTOOL_H

#include "GaudiKernel/IAlgTool.h"

class IGeoModelTool : public virtual IAlgTool {
public:

    DeclareInterfaceID( IGeoModelTool, 1, 0 );

    // Abstract interface method(s)
    virtual StatusCode create() = 0;

    // This method is designed to perform following tasks:
    //    1. Release detector manager from the Detector Store
    //    2. Do any extra clean up tasks if necessary
    virtual StatusCode clear() = 0;

    // Function to apply alignments in Simulation jobs
    virtual StatusCode align() = 0;
};

#endif // GEOMODELINTERFACES_IGEOMODELTOOL_H
