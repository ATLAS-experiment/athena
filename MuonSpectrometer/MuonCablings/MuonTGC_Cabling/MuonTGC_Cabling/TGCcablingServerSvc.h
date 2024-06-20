/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCcablingServerSvc_H
#define TGCcablingServerSvc_H

#include "AthenaBaseComps/AthService.h"
#include "TGCcablingInterface/ITGCcablingServerSvc.h"

namespace Muon {

class TGCcablingServerSvc : public AthService, 
                            virtual public ITGCcablingServerSvc {
 public:
    TGCcablingServerSvc(const std::string& name, ISvcLocator* svc);
    virtual ~TGCcablingServerSvc() = default;

    virtual StatusCode queryInterface(const InterfaceID& riid,void** ppvIF);

    // Interface implementation
    virtual StatusCode giveCabling( const ITGCcablingSvc*&) const;
    virtual bool isAtlas(void) const;

 private:
    BooleanProperty m_atlas{this, "Atlas", true, "Controls whether using ATLAS cabling"};
};

}

#endif  // TGCcablingServerSvc_H
