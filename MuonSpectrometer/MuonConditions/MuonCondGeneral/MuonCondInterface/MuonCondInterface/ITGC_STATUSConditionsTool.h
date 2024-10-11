/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDINTERFACE_ITGC_STATUSCONDITIONSTOOL_H
#define MUONCONDINTERFACE_ITGC_STATUSCONDITIONSTOOL_H

// Includes for Gaudi
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include "GaudiKernel/IAlgTool.h"
#include "Identifier/Identifier.h"

//**********************************************************
//* Author Monica Verducci monica.verducci@cern.ch
//*
//* Tool to retrieve the MDT DCS Info from COOL DB
//* retrieving of tables from DB virtual interface
//*********************************************************

class ITGC_STATUSConditionsTool : virtual public extend_interfaces<IAlgTool, IAddressProvider> {
public:
    DeclareInterfaceID(ITGC_STATUSConditionsTool, 1, 0);

    virtual std::string FolderName() const = 0;

    virtual StatusCode loadParameterStatus(IOVSVC_CALLBACK_ARGS) = 0;

    virtual StatusCode loadTgcDqStatus(IOVSVC_CALLBACK_ARGS) = 0;

    virtual const std::vector<Identifier>& deadStationsId() = 0;
};

#endif
