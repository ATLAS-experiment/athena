/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCCablingServerSvc_H
#define TGCCablingServerSvc_H

#include "AthenaBaseComps/AthService.h"
#include "TGCcablingInterface/ITGCcablingServerSvc.h"

namespace Muon {

class TGCCablingServerSvc : public AthService, 
                            virtual public ITGCcablingServerSvc {
 public:
  // Constructor and other Service methods
  TGCCablingServerSvc(const std::string& name, ISvcLocator* svc);
  virtual ~TGCCablingServerSvc() = default;

  virtual StatusCode queryInterface(const InterfaceID& riid,void** ppvIF);

  // Interface implementation
  virtual StatusCode giveCabling( const ITGCcablingSvc*&) const;
  virtual bool isAtlas(void) const;

 private:
  BooleanProperty m_atlas{this, "Atlas", true, "Controls whether using ATLAS cabling, or from testbeams etc"};
};

}

#endif  // TGCCablingServerSvc_H
