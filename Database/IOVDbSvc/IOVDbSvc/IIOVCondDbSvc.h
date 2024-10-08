/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __IIOVCONDDBSVC_H__
#define __IIOVCONDDBSVC_H__

/*****************************************************************************
 *
 *  IIOVCondDbSvc.h
 *  AthenaKernel
 *
 *  Author: RD Schaffer, Antoine Perus
 *
 *  Abstract interface to IOVDbSvc to access ConditionsDB
 *
 *****************************************************************************/

#include "GaudiKernel/IInterface.h"

#include "CoolKernel/types.h"
#include "CoolKernel/IDatabase.h"


class IIOVCondDbSvc : virtual public IInterface  {
public:
    /// Declare interface ID
    DeclareInterfaceID(IIOVCondDbSvc, 1, 0);

    /// COOL access
    virtual cool::IDatabasePtr getDatabase(bool readOnly = false ) = 0;

};

#endif
