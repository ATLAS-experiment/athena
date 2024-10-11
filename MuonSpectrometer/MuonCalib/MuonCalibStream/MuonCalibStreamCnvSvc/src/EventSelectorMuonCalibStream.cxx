/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//====================================================================
//	EventSelectorMuonCalibStream.cxx
//      Event loop for calibration stream
//====================================================================
//
// Include files.
#include "MuonCalibStreamCnvSvc/EventSelectorMuonCalibStream.h"
#include "MuonCalibStreamCnvSvc/EventContextMuonCalibStream.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/StatusCode.h"

#include "xAODEventInfo/EventInfo.h"

// Constructor.
EventSelectorMuonCalibStream::EventSelectorMuonCalibStream(const std::string &name, ISvcLocator *svcloc) :
    base_class(name, svcloc),
    m_beginIter(nullptr),
    m_endIter(nullptr)
{
    ATH_MSG_DEBUG("EventSelectorMuonCalibStream constructor");
}

// Destructor.
EventSelectorMuonCalibStream::~EventSelectorMuonCalibStream() {
    if (m_beginIter) delete m_beginIter;
    if (m_endIter) delete m_endIter;
}

// EventSelectorMuonCalibStream::initialize().
StatusCode EventSelectorMuonCalibStream::initialize() {
    ATH_MSG_INFO("EventSelectorMuonCalibStream::initialize");

    // Check MuonCalibStreamCnvSvc
    ATH_CHECK( m_eventSource.retrieve() );

    ATH_CHECK( m_dataProvider.retrieve() );

    // Create the begin and end iterators for this selector.
    m_beginIter = new EventContextMuonCalibStream(this);
    // increment to get the new event in.
    //     ++(*m_beginIter);   ???
    m_endIter = new EventContextMuonCalibStream(0);

    return StatusCode::SUCCESS;
}

StatusCode EventSelectorMuonCalibStream::createContext(IEvtSelector::Context *&it) const {
    it = new EventContextMuonCalibStream(this);
    return (StatusCode::SUCCESS);
}

// Implementation of IEvtSelector::next().
StatusCode EventSelectorMuonCalibStream::next(IEvtSelector::Context &it) const {
    ATH_MSG_DEBUG(" EventSelectorMuonCalibStream::next m_NumEvents=" << m_NumEvents);
    for (;;) {
        const LVL2_MUON_CALIBRATION::CalibEvent *pre = m_eventSource->nextEvent();
        if (!pre) {
            // End of file
            it = *m_endIter;
            return StatusCode::FAILURE;
        }
        ++m_NumEvents;

        // Check if we are skipping events
        if (m_NumEvents > m_SkipEvents) {
            break;
        } else {
            ATH_MSG_DEBUG(" Skipping event " << m_NumEvents - 1);
        }
    }

    return StatusCode::SUCCESS;
}

// Implementation of IEvtSelector::next() with a "jump" parameter
// (to skip over a certain number of events?)
StatusCode EventSelectorMuonCalibStream::next(IEvtSelector::Context &ctxt, int jump) const {
    ATH_MSG_DEBUG(" EventSelectorMuonCalibStream::next skipping events ==" << jump);
    if (jump > 0) {
        for (int i = 0; i < jump; ++i) {
            StatusCode status = next(ctxt);
            if (!status.isSuccess()) { return status; }
        }
        return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::previous(IEvtSelector::Context & /*it*/) const {
    ATH_MSG_ERROR("EventSelectorMuonCalibStream::previous() not implemented");
    return (StatusCode::FAILURE);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::previous(IEvtSelector::Context &it, int /*jump*/) const { return (previous(it)); }

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::last(IEvtSelector::Context &it) const {
    if (it.identifier() == m_endIter->identifier()) {
        ATH_MSG_DEBUG("last(): Last event in InputStream.");
        return (StatusCode::SUCCESS);
    }
    return (StatusCode::FAILURE);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::resetCriteria(const std::string & /*criteria*/, IEvtSelector::Context & /*ctxt*/) const {
    return (StatusCode::SUCCESS);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::rewind(IEvtSelector::Context & /*it*/) const {
    ATH_MSG_ERROR("EventSelectorMuonCalibStream::rewind() not implemented");
    return (StatusCode::FAILURE);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::createAddress(const IEvtSelector::Context & /*it*/, IOpaqueAddress *&iop) const {
    ATH_MSG_DEBUG("EventSelectorMuonCalibStream::createAddress");
    const LVL2_MUON_CALIBRATION::CalibEvent *pre = m_eventSource->currentEvent();
    m_dataProvider->setNextEvent(pre);
    ATH_MSG_DEBUG("Calib Event cached in Data Provider ");

    iop = new MuonCalibStreamAddress(ClassID_traits<xAOD::EventInfo>::ID(), "EventInfo", ""); // change to xAOD::EventInfo key
    //iop = new MuonCalibStreamAddress(ClassID_traits<xAOD::EventInfo>::ID(), "MuonCalibStreamEventInfo", ""); // old key which need the conversion afterwards
    ATH_MSG_DEBUG("MuonCalibStreamAddress for MuonCalibStreamEventInfo created ");

    return (StatusCode::SUCCESS);
}

//________________________________________________________________________________
StatusCode EventSelectorMuonCalibStream::releaseContext(IEvtSelector::Context *& /*it*/) const { return (StatusCode::SUCCESS); }
