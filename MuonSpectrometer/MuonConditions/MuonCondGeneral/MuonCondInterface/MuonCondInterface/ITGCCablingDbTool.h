/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDINTERFACE_ITGCCABLINGDBTOOL_H
#define MUONCONDINTERFACE_ITGCCABLINGDBTOOL_H

// Includes for Gaudi
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include "GaudiKernel/IAlgTool.h"

// class TgcIdHelper;

#include <string>
#include <vector>



class ITGCCablingDbTool : virtual public extend_interfaces<IAlgTool, IAddressProvider> {
public:
    DeclareInterfaceID(ITGCCablingDbTool, 1, 0);

    virtual StatusCode loadParameters(IOVSVC_CALLBACK_ARGS) = 0;

    virtual StatusCode loadASD2PP_DIFF_12(IOVSVC_CALLBACK_ARGS) = 0;

    virtual StatusCode readASD2PP_DIFF_12FromText() = 0;

    virtual std::vector<std::string>* giveASD2PP_DIFF_12(void) = 0;

    virtual std::string getFolderName(void) const = 0;
};

#endif  // MUONCONDINTERFACE_ITGCCABLINGDBTOOL_H
