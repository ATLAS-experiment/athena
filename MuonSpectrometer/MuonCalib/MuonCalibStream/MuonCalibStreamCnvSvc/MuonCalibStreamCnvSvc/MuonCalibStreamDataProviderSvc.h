#ifndef MUONCALIBSTREAMCNVSVCBASE_MUONCALIBSTREAMDATAPROVIDERSVC_H
#define MUONCALIBSTREAMCNVSVCBASE_MUONCALIBSTREAMDATAPROVIDERSVC_H

#include <map>
#include <vector>

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/StatusCode.h"
#include "MuCalDecode/CalibEvent.h"
#include "MuonCalibStreamCnvSvc/IMuonCalibStreamDataProviderSvc.h"

class MuonCalibRunLumiBlockCoolSvc;

class MuonCalibStreamDataProviderSvc : public extends<AthService, IMuonCalibStreamDataProviderSvc> {

public:
    MuonCalibStreamDataProviderSvc(const std::string &name, ISvcLocator *svcloc);
    virtual ~MuonCalibStreamDataProviderSvc();
    virtual StatusCode initialize();
    virtual void setNextEvent(const LVL2_MUON_CALIBRATION::CalibEvent *re);
    virtual const LVL2_MUON_CALIBRATION::CalibEvent *getEvent();

    int fakeEventN();
    int fakeRunN();
    int fakeLumiBlock();
    float LVL2_pt();
    int timeStamp();

private:
    ServiceHandle<MuonCalibRunLumiBlockCoolSvc> m_lumiBlockCoolSvc;
    bool m_run_number_from_cool;
    bool m_lumi_block_number_from_cool;
    const LVL2_MUON_CALIBRATION::CalibEvent *m_event;
    int m_evtN;
    int m_runN;
    int m_fake_evtN;
    int m_fake_runN;
    int m_fake_lumiB;
    float m_pt;
    int m_timeStamp;
};
#endif
