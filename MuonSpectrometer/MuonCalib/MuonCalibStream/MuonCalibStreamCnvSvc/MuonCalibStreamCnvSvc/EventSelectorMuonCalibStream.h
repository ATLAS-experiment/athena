/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIBSTREAMCNVSVC_EVENTSELECTORMUONCALIBSTREAM_H
#define MUONCALIBSTREAMCNVSVC_EVENTSELECTORMUONCALIBSTREAM_H
//  EventSelectorMuonCalibStream

// Include files.
#include <map>

#include "MuonCalibStreamInputSvc.h"
#include "IMuonCalibStreamDataProviderSvc.h"

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuCalDecode/CalibEvent.h"

// Forward declarations.
class ISvcLocator;
class EventContextMuonCalibStream;

// Class EventSelectorMuonCalibStream.
class EventSelectorMuonCalibStream : public extends<AthService, IEvtSelector> {
public:
    // Standard Constructor.
    EventSelectorMuonCalibStream(const std::string &name, ISvcLocator *svcloc);

    // Standard Destructor.
    ~EventSelectorMuonCalibStream();

    // Implementation of Service base class methods.
    virtual StatusCode initialize();

    // Implementation of the IEvtSelector interface methods.
    virtual StatusCode createContext(Context *&it) const;
    virtual StatusCode next(Context &it) const;
    virtual StatusCode next(Context &it, int jump) const;
    virtual StatusCode previous(Context &it) const;
    virtual StatusCode previous(Context &it, int jump) const;

    virtual StatusCode last(Context &it) const;
    virtual StatusCode rewind(Context &it) const;

    virtual StatusCode createAddress(const Context &it, IOpaqueAddress *&iop) const;
    virtual StatusCode releaseContext(Context *&it) const;
    virtual StatusCode resetCriteria(const std::string &criteria, Context &context) const;

private:
    // property
    Gaudi::Property<int> m_SkipEvents{this, "SkipEvents", 0, "Number of events to skip at the beginning"};
    ServiceHandle<MuonCalibStreamInputSvc> m_eventSource{ this, "MuonCalibStreamInputSvc", "MuonCalibStreamFileInputSvc"};
    ServiceHandle<IMuonCalibStreamDataProviderSvc> m_dataProvider{this, "DataProvider", "MuonCalibStreamDataProviderSvc"};

    EventContextMuonCalibStream *m_beginIter;
    EventContextMuonCalibStream *m_endIter;

    mutable std::atomic<long> m_NumEvents{0};  // Number of Events read so far.
};

#endif
