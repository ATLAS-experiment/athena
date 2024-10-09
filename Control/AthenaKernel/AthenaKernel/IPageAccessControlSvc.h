/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IPAGEACCESSCONTROLSVC_H
#define ATHENAKERNEL_IPAGEACCESSCONTROLSVC_H

#include "GaudiKernel/IInterface.h"

class PageAccessControl;

/**
 * @class  IPageAccessControlSvc
 * @brief  Interface to a service that monitors memory page accesses
 * @author Paolo Calafiura
 *
 * This interface allows to start and stop the monitoring and to print a report
 */
class IPageAccessControlSvc: virtual public IInterface {
public:
  /// Declare interface ID
  DeclareInterfaceID(IPageAccessControlSvc, 1, 0);

  /// Virtualize D'tor
  virtual ~IPageAccessControlSvc() {}

  /// In baseline implementation, protect pages and install a SEGV handler
  /// that counts the number of accesses to a protected address.
  virtual bool startMonitoring() = 0;
  virtual bool stopMonitoring() = 0;
  /// has this pointer been accessed (read/written)
  virtual bool accessed(const void* address) const =0;
  /// In baseline implementation, controlled via PageAccessControlSvc.OutputLevel
  virtual void report() const = 0;

  ///control access to the page containing address
  virtual bool controlPage(const void* address) = 0;

  ///FIXME Access to the underlying @class PageAccessControl, used to 
  ///protect/restore page access
  //hopefully not necessary  virtual PageAccessControl* pac() = 0;
};

#endif // ATHENAKERNEL_IPAGEACCESSCONTROLSVC_H
