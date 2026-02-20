/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Includes
#include "AthContainers/AuxElement.h"

// Local include
#include "TrigT1Interfaces/CTPResultUtils.h"

// TDAQ include
#include "CTPfragment/CTPdataformatVersion.h"

// Gaudi include(s):
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"

namespace CTPResultUtils {

  // Initialise the object given the CTP version number, data words and the number of extra words
  void initialize(xAOD::CTPResult& ctpRes, const uint32_t ctpVersionNumber, std::vector<uint32_t>& data, const uint32_t nExtraWords) {

    // CTP version number
    CTPdataformatVersion ctpDataFormat(ctpVersionNumber);
    ctpRes.setCtpVersionNumber(ctpVersionNumber);

    // Number of bunches
    if (!data.size()) {
      ctpRes.setNumberOfBunches(0u);
    } else {
      uint32_t numberOfWords = static_cast<uint32_t>(data.size()) - ctpDataFormat.getNumberTimeWords() - nExtraWords;
      ctpRes.setNumberOfBunches(numberOfWords / ctpDataFormat.getDAQwordsPerBunch());
    }

    ctpRes.setTimeSec(data[ctpDataFormat.getTimeSecondsPos()]);
    ctpRes.setTimeNanoSec(data[ctpDataFormat.getTimeNanosecondsPos()]);

    // Create vectors of vectors for the trigger words for all bunches
    std::vector<std::vector<uint32_t>> tip(ctpRes.numberOfBunches());
    std::vector<std::vector<uint32_t>> tbp(ctpRes.numberOfBunches());
    std::vector<std::vector<uint32_t>> tap(ctpRes.numberOfBunches());
    std::vector<std::vector<uint32_t>> tav(ctpRes.numberOfBunches());

    // Fill vectors
    for( uint32_t bunch=0u; bunch != ctpRes.numberOfBunches(); ++bunch){

      // TIP words
      for(unsigned int tipIdx = 0; tipIdx < ctpDataFormat.getTIPwords(); ++tipIdx) {
        unsigned int index = ctpDataFormat.getTIPpos() + tipIdx + (bunch * ctpDataFormat.getDAQwordsPerBunch());
        if( index < data.size() ) {
          tip[bunch].push_back(data[index]);
        }
      }

      // TBP words
      for(unsigned int tbpIdx = 0; tbpIdx < ctpDataFormat.getTBPwords(); ++tbpIdx) {
        unsigned int index = ctpDataFormat.getTBPpos() + tbpIdx + (bunch * ctpDataFormat.getDAQwordsPerBunch());
        if( index < data.size() ) {
          tbp[bunch].push_back(data[index]);
        }
      }

      // TAP words
      for(unsigned int tapIdx = 0; tapIdx < ctpDataFormat.getTAPwords(); ++tapIdx) {
        unsigned int index = ctpDataFormat.getTAPpos() + tapIdx + (bunch * ctpDataFormat.getDAQwordsPerBunch());
        if( index < data.size() ) {
          tap[bunch].push_back(data[index]);
        }
      }

      // TAV words
      for(unsigned int tavIdx = 0; tavIdx < ctpDataFormat.getTAVwords(); ++tavIdx) {
        unsigned int index = ctpDataFormat.getTAVpos() + tavIdx + (bunch * ctpDataFormat.getDAQwordsPerBunch());
        if( index < data.size() ) {
          tav[bunch].push_back(data[index]);
        }
      }
    }
    ctpRes.setTIPWords(tip);
    ctpRes.setTBPWords(tbp);
    ctpRes.setTAPWords(tap);
    ctpRes.setTAVWords(tav);

    // Additional words
    std::vector<uint32_t> vec;
    vec.assign(data.begin() + (data.size()-nExtraWords), data.end());
    ctpRes.setAdditionalWords(vec);
  }

  // Print object content to default message stream
  void dumpData(xAOD::CTPResult& ctpRes) {
    SmartIF<IMessageSvc> msgSvc{Gaudi::svcLocator()->service("MessageSvc")};
    if ( !msgSvc ) {
      return;
    }
    MsgStream log(msgSvc, "xAOD::CTPResult");
    dumpData(ctpRes, log);
  }

  // Print object content to given message stream
  void dumpData(xAOD::CTPResult& ctpRes, MsgStream& log) {

    log << MSG::DEBUG << "*BEGIN* xAOD::CTPResult" << endmsg;

    // Check number of bunches
    if(ctpRes.numberOfBunches() == 0) {
      log << MSG::DEBUG << "xAOD::CTPResult empty" << endmsg;
      log << MSG::DEBUG << "*END* xAOD::CTPResult" << endmsg;
      return;
    }

    // Check number of bunches size
    if(ctpRes.numberOfBunches() != ctpRes.tipWords().size()) {
      log << MSG::ERROR <<"Mismatch: " << ctpRes.numberOfBunches() << " bunches, but the size of tipWords is " << ctpRes.tipWords().size() << endmsg;
    }

    log << MSG::DEBUG << "CTP version number: " <<  ctpRes.ctpVersionNumber() << endmsg;
    log << MSG::DEBUG << "Number of bunches: " <<  ctpRes.numberOfBunches() << endmsg;
    log << MSG::DEBUG << "L1A position: " << ctpRes.l1AcceptBunchPosition() << endmsg;

    // Print header info
    log << MSG::DEBUG << "Header information:  " << endmsg;
    log << MSG::DEBUG << "    Header marker           :  " << MSG::hex << ctpRes.headerMarker() << MSG::dec << endmsg;
    log << MSG::DEBUG << "    Header size             :  " << ctpRes.headerSize() << endmsg;
    log << MSG::DEBUG << "    Header format version   :  " << ctpRes.headerFormatVersion() << endmsg;
    log << MSG::DEBUG << "    Source ID               :  0x" << MSG::hex << ctpRes.sourceID() << MSG::dec << endmsg;
    log << MSG::DEBUG << "    Run number              :  " << ctpRes.runNumber() << endmsg;
    log << MSG::DEBUG << "    Ext. LVL1 ID            :  " << ctpRes.L1ID() << endmsg;
    log << MSG::DEBUG << "    BCID                    :  " << ctpRes.BCID() << endmsg;
    log << MSG::DEBUG << "    Trigger type            :  " << ctpRes.triggerType() << endmsg;
    log << MSG::DEBUG << "    Det. event type         :  " << ctpRes.eventType() << endmsg;

    // Print payload info
    log << MSG::DEBUG << "Payload information:  " << endmsg;
    log << MSG::DEBUG << "    Time " << ctpRes.timeSec() << "s "
        << std::setw(10) << std::setiosflags(std::ios_base::right) << std::setfill(' ')
        << ctpRes.timeNanoSec() << std::resetiosflags(std::ios_base::right)
        << "ns" << endmsg;

    // Print per-bunch information
    for(unsigned int i = 0; i<ctpRes.numberOfBunches(); ++i) {

      auto bunch = ctpRes.getBC(i);
      log << MSG::DEBUG << "    BC dump for bunch " << i << endmsg;

      // TIP words
      for(unsigned int j = 0; j<ctpRes.tipWords()[i].size(); ++j) {
        log << MSG::DEBUG << "        TIP word number " << j << ": " << ctpRes.tipWords()[i][j] << endmsg;
      }
      if (ctpRes.tipWords()[i].size() == 0) {
        log << MSG::DEBUG << "        No TIP words!" << endmsg;
      }

      // TBP words
      for(unsigned int j = 0; j<ctpRes.tbpWords()[i].size(); ++j) {
        log << MSG::DEBUG << "        TBP word number " << j << ": " << ctpRes.tbpWords()[i][j] << endmsg;
      }
      if (ctpRes.tbpWords()[i].size() == 0) {
        log << MSG::DEBUG << "        No TBP words!" << endmsg;
      }

      // TAP words
      for(unsigned int j = 0; j<ctpRes.tapWords()[i].size(); ++j) {
        log << MSG::DEBUG << "        TAP word number " << j << ": " << ctpRes.tapWords()[i][j] << endmsg;
      }
      if (ctpRes.tapWords()[i].size() == 0) {
        log << MSG::DEBUG << "        No TAP words!" << endmsg;
      }

      // TAV words
      for(unsigned int j = 0; j<ctpRes.tavWords()[i].size(); ++j) {
        log << MSG::DEBUG << "        TAV word number " << j << ": " << ctpRes.tavWords()[i][j] << endmsg;
      }
      if (ctpRes.tavWords()[i].size() == 0) {
        log << MSG::DEBUG << "        No TAV words!" << endmsg;
      }
    }

    // Additional words
    for(unsigned int i = 0; i<ctpRes.additionalWords().size(); ++i) {
      log << MSG::DEBUG << "    Additional word number " << i << ": " << ctpRes.additionalWords()[i] << endmsg;
    }
    if (ctpRes.additionalWords().size() == 0) {
      log << MSG::DEBUG << "    No additional words!" << endmsg;
    }

    // Print trailer info
    log << MSG::DEBUG << "Trailer information:  " << endmsg;
    log << MSG::DEBUG << "    Error status                :  " << ctpRes.errorStatus() << endmsg;
    log << MSG::DEBUG << "    Status info                 :  " << ctpRes.infoStatus() << endmsg;
    log << MSG::DEBUG << "    Number of status words      :  " << ctpRes.numStatusWords() << endmsg;
    log << MSG::DEBUG << "    Number of data words        :  " << ctpRes.numDataWords() << endmsg;
    log << MSG::DEBUG << "    Status information position :  " << ctpRes.statusPosition() << endmsg;
    log << MSG::DEBUG << "*END* xAOD::CTPResult" << endmsg;
  }


} // namespace CTPResultUtils
