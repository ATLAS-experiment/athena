/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBSTREAMCNVSVC_EVENTINFOMUONCALIBSTREAMCNV_H
#define MUONCALIBSTREAMCNVSVC_EVENTINFOMUONCALIBSTREAMCNV_H

#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/Converter.h"
#include "MuCalDecode/CalibEvent.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"
#include "MuonCalibStreamCnvSvc/IMuonCalibStreamDataProviderSvc.h"

class IOpaqueAddress;
class DataObject;

#include <string>

class EventInfoMuonCalibStreamCnv : public Converter, public AthMessaging {
public:
    virtual StatusCode initialize();
    virtual StatusCode createObj(IOpaqueAddress *pAddr, DataObject *&pObj);
    virtual StatusCode createRep(DataObject *pObj, IOpaqueAddress *&pAddr);

    /// Storage type and class ID
    virtual long repSvcType() const { return MuonCalibStreamAddress::storageType(); }
    static long storageType() { return MuonCalibStreamAddress::storageType(); }
    static CLID classID();

    EventInfoMuonCalibStreamCnv(ISvcLocator *svcloc);

private:
    ServiceHandle<IConversionSvc> m_MuonCalibStreamCnvSvc;
    ServiceHandle<IMuonCalibStreamDataProviderSvc> m_dataProvider;
};
#endif
