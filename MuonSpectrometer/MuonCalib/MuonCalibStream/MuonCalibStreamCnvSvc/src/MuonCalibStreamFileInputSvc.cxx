//===================================================================
//  Implementation of MuonCalibStreamFileInputSvc
//===================================================================
//

// Include files.
#include "MuonCalibStreamCnvSvc/MuonCalibStreamFileInputSvc.h"

#include <iostream>

#include "EventStorage/pickDataReader.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "MuCalDecode/CalibUti.h"
//#include "EventStorage/DataReaderBase.h"
//#include "EventStorage/DataReader.h"
#include "EventStorage/EventStorageRecords.h"
#include "EventStorage/fRead.h"
#include "EventStorage/loadfRead.h"

// Constructor.
MuonCalibStreamFileInputSvc::MuonCalibStreamFileInputSvc(const std::string& name, ISvcLocator* svcloc) :
    base_class(name, svcloc) {}

// Destructor.
MuonCalibStreamFileInputSvc::~MuonCalibStreamFileInputSvc() {}

// Open the first input file and read the first event.
StatusCode MuonCalibStreamFileInputSvc::initialize() {

    // Check that the vector of input file names is not empty.
    if (m_inputFiles.empty()) {
        ATH_MSG_ERROR(" initialize: No input event files ");
        return StatusCode::FAILURE;
    }

    // Set the iterator to the first input file.
    m_inputFilesIt = m_inputFiles.begin();

    // instantiate the data loader
    m_dataLoader = std::make_unique<LVL2_MUON_CALIBRATION::CalibDataLoader>(m_inputFilesIt->c_str(), false, 0xffffffff, 0, DEFAULT_BUFFER_SIZE);

    m_EoF = false;

    return StatusCode::SUCCESS;
}  // MuonCalibStreamFileInputSvc::initialize()

// Read the next event.
const LVL2_MUON_CALIBRATION::CalibEvent* MuonCalibStreamFileInputSvc::nextEvent() {

    if (m_EoF) {  // EOF reached, need to change input file
        ATH_MSG_INFO("nextEvent: end of file reached ");
        m_totalEventCounter += m_fileEventCounter;

        ATH_MSG_INFO("nextEvent: finished with file  " << *m_inputFilesIt);
        ATH_MSG_INFO(" Number of Events in this file " << m_fileEventCounter);
        ATH_MSG_INFO(" Total number of Events        " << m_totalEventCounter);

        // Reinitialize the file event counter.
        m_fileEventCounter = 0;

        ++m_inputFilesIt;

        if (m_inputFilesIt == m_inputFiles.end()) {
            ATH_MSG_INFO("no more file to read  ");
            return 0;
        }

        // do the next File
        m_dataLoader = std::make_unique<LVL2_MUON_CALIBRATION::CalibDataLoader>(m_inputFilesIt->c_str(), false, 0xffffffff, 0, DEFAULT_BUFFER_SIZE);

        m_EoF = false;

        return nextEvent();
    }

    ++m_fileEventCounter;

    // try to read next event
    // add TGC fragment and move the print out of the fragment to DEBUG outputLevel
    if (m_dataLoader->next(m_dataBuffer)) {
        m_re = std::make_unique<LVL2_MUON_CALIBRATION::CalibEvent>(m_dataBuffer);

        if (m_re) {
            if (m_re->mdt() != 0) {
                if (m_DumpStream != 0)
                    ATH_MSG_DEBUG(" MuonCalibStreamFileInputSvc::next event -- eta=" << m_re->eta() << " mdt " << *(m_re->mdt()));
            }
            if (m_re->rpc() != 0) {
                if (m_DumpStream != 0)
                    ATH_MSG_DEBUG(" MuonCalibStreamFileInputSvc::next event -- eta=" << m_re->eta() << " rpc " << *(m_re->rpc()));
            }
            if (m_re->tgc() != 0) {
                if (m_DumpStream != 0)
                    ATH_MSG_DEBUG(" MuonCalibStreamFileInputSvc::next event -- eta=" << m_re->eta() << " tgc " << *(m_re->tgc()));
            }
        }
    } else {
        // file must be over
        m_EoF = true;
        return nextEvent();
    }

    // Return
    return m_re.get();
}  // MuonCalibStreamFileInputSvc::nextEvent()

///  Get a pointer to the current event.
const LVL2_MUON_CALIBRATION::CalibEvent* MuonCalibStreamFileInputSvc::currentEvent() const {
    // Return a pointer to the  raw event.
    return m_re.get();
}
