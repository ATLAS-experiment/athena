/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDINTERFACE_ITGCCABLINGDBTOOL_H
#define MUONCONDINTERFACE_ITGCCABLINGDBTOOL_H

// Includes for Gaudi
#include "GaudiKernel/IAlgTool.h"

#include <string>
#include <vector>


class ITGCCablingDbTool : virtual public extend_interfaces<IAlgTool> {
public:
    DeclareInterfaceID(ITGCCablingDbTool, 1, 0);

    virtual StatusCode readASD2PP_DIFF_12FromText() = 0;

    virtual std::vector<std::string>* giveASD2PP_DIFF_12(void) = 0;

    virtual std::string getFolderName(void) const = 0;
};

#endif  // MUONCONDINTERFACE_ITGCCABLINGDBTOOL_H
