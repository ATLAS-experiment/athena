// Include files.
#include "MuonCalibStreamCnvSvc/MuonCalibStreamDataProviderSvc.h"

#include "GaudiKernel/ISvcLocator.h"
#include "MuonCalibStreamCnvSvc/MuonCalibRunLumiBlockCoolSvc.h"

// Constructor.
MuonCalibStreamDataProviderSvc::MuonCalibStreamDataProviderSvc(const std::string &name, ISvcLocator *svcloc) :
    AthService(name, svcloc),
    m_lumiBlockCoolSvc("MuonCalibRunLumiBlockCoolSvc", "MuonCalibRunLumiBlockCoolSvc"),
    m_run_number_from_cool(false),
    m_lumi_block_number_from_cool(false),
    m_event(nullptr),
    m_evtN(0),
    m_runN(0),
    m_fake_evtN(0),
    m_fake_runN(-1),
    m_fake_lumiB(200),
    m_pt(0),
    m_timeStamp(0) {
    declareProperty("RunNumber", m_fake_runN, "run number to be used for DB access");
    declareProperty("FirstEventNumber", m_fake_evtN, "reset fake event number to this value");
    declareProperty("LumiBlockCoolSvc", m_lumiBlockCoolSvc);
    declareProperty("RunNumberFromCool", m_run_number_from_cool);
    declareProperty("LumiBlockNumberFromCool", m_lumi_block_number_from_cool);
}

// Destructor.
MuonCalibStreamDataProviderSvc::~MuonCalibStreamDataProviderSvc() {}

// Initialization
StatusCode MuonCalibStreamDataProviderSvc::initialize() {
    if (m_run_number_from_cool || m_lumi_block_number_from_cool) { ATH_CHECK(m_lumiBlockCoolSvc.retrieve()); }
    return StatusCode::SUCCESS;
}

/// Query interface
StatusCode MuonCalibStreamDataProviderSvc::queryInterface(const InterfaceID &riid, void **ppvInterface) {
    if (IMuonCalibStreamDataProviderSvc::interfaceID().versionMatch(riid)) {
        *ppvInterface = (IMuonCalibStreamDataProviderSvc *)this;
    } else {
        return AthService::queryInterface(riid, ppvInterface);
    }

    addRef();
    return StatusCode::SUCCESS;
}

void MuonCalibStreamDataProviderSvc::setNextEvent(const LVL2_MUON_CALIBRATION::CalibEvent *re) {
    m_event = re;
    m_evtN = m_event->lvl1_id();
    m_runN = m_event->run_number();
    m_pt = m_event->pt();
    m_timeStamp = m_event->timestamp();
    m_fake_evtN++;
    if (m_run_number_from_cool || m_lumi_block_number_from_cool) {
        int runN(-1);
        if (!m_run_number_from_cool) {
            if (m_fake_runN >= 0)
                runN = m_fake_runN;
            else
                runN = m_runN;
        }
        int lb_nr = 200;
        if (!m_lumiBlockCoolSvc->GetRunEventNumber(m_timeStamp, runN, lb_nr).isSuccess()) {
            ATH_MSG_FATAL("Failed to get run event number!");
            return;
        }
        if (m_run_number_from_cool) { m_fake_runN = runN; }
        if (m_lumi_block_number_from_cool) { m_fake_lumiB = lb_nr; }
    }
    return;
}

const LVL2_MUON_CALIBRATION::CalibEvent *MuonCalibStreamDataProviderSvc::getEvent() { return m_event; }

int MuonCalibStreamDataProviderSvc::fakeEventN() {
    if (m_fake_runN < 0) {
        return m_evtN;
    } else {
        return m_fake_evtN;
    }
}

int MuonCalibStreamDataProviderSvc::fakeRunN() {
    if (m_fake_runN < 0) {
        return m_runN;
    } else {
        return m_fake_runN;
    }
}

int MuonCalibStreamDataProviderSvc::fakeLumiBlock() { return m_fake_lumiB; }

float MuonCalibStreamDataProviderSvc::LVL2_pt() {
    return m_pt;
}

int MuonCalibStreamDataProviderSvc::timeStamp() {
    return m_timeStamp;
}
