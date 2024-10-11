#ifndef MUONCALIBSTREAMCNVSVC_MUONCALIBSTREAMFILEINPUTSVC_H
#define MUONCALIBSTREAMCNVSVC_MUONCALIBSTREAMFILEINPUTSVC_H

//===================================================================
//     MuonCalibStreamFileInputSvc.h
//===================================================================
//
// Description: This class implements the interface MuonCalibStreamInputSvc for
//              event selector to read the events for Files.
//
//-------------------------------------------------------------------

// Include files.
#include <fstream>
#include <TROOT.h>

#include "MuonCalibStreamInputSvc.h"

#include "AthenaBaseComps/AthService.h"
#include "EventStorage/DataReader.h"
#include "MuCalDecode/CalibDataLoader.h"
#include "MuCalDecode/CalibEvent.h"
#include "MuCalDecode/CalibUti.h"
#include "MuCalDecode/DataBuffer.h"

class MuonCalibStreamFileInputSvc : public extends<AthService, MuonCalibStreamInputSvc> {
public:
    // Constructors:
    MuonCalibStreamFileInputSvc(const std::string &name, ISvcLocator *svcloc);
    // Destructor.
    virtual ~MuonCalibStreamFileInputSvc();
    // Implementation of the MuonCalibStreamInputSvc interface methods.
    virtual StatusCode initialize() override;
    virtual const LVL2_MUON_CALIBRATION::CalibEvent *nextEvent() override;
    virtual const LVL2_MUON_CALIBRATION::CalibEvent *currentEvent() const override;

private:
    Long64_t m_fileEventCounter{0};
    Long64_t m_totalEventCounter{0};
    Gaudi::Property<int> m_DumpStream{this, "DumpStream", 0};
    std::unique_ptr<DataReader>  m_reader{};
    bool m_EoF{false};
    std::unique_ptr<LVL2_MUON_CALIBRATION::CalibEvent> m_re{};
    std::unique_ptr<LVL2_MUON_CALIBRATION::CalibDataLoader> m_dataLoader{};
    DataBuffer m_dataBuffer;
    Gaudi::Property<std::vector<std::string>> m_inputFiles{this, "InputFiles", {} };
    std::vector<std::string>::iterator m_inputFilesIt{m_inputFiles.value().begin()};
};
#endif
