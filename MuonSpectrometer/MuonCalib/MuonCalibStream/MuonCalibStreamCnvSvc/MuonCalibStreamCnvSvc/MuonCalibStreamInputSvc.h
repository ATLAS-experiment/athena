//===============================================================
//     MuonCalibStreamInputSvc.h
//===============================================================
//
// Description: Interface class for MuonCalibStream Input
//
//              The concrete class can be provide Calib event from
//              a file, transient store, or through network.
//---------------------------------------------------------------
#ifndef MUONCALIBSTREAMCNVSVC_MUONCALIBSTREAMINPUTSVC_H
#define MUONCALIBSTREAMCNVSVC_MUONCALIBSTREAMINPUTSVC_H

#include "GaudiKernel/IInterface.h"
#include "MuCalDecode/CalibEvent.h"

class MuonCalibStreamInputSvc : virtual public IInterface {
public:
    DeclareInterfaceID(MuonCalibStreamInputSvc, 1, 0);

    virtual const LVL2_MUON_CALIBRATION::CalibEvent *nextEvent() = 0;
    virtual const LVL2_MUON_CALIBRATION::CalibEvent *currentEvent() const = 0;
};
#endif
