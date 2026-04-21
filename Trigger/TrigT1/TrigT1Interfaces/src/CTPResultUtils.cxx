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
  const std::string print(const xAOD::CTPResult& ctpRes) {

    std::ostringstream s;

    s << "\n*BEGIN* xAOD::CTPResult" << std::endl;

    // Check number of bunches
    if(ctpRes.numberOfBunches() == 0) {
      s << "xAOD::CTPResult empty" << std::endl;
      s << "*END* xAOD::CTPResult" << std::endl;
      return s.str();
    }

    s << "CTP version number: " <<  ctpRes.ctpVersionNumber() << std::endl;
    s << "Number of bunches: " <<  ctpRes.numberOfBunches() << std::endl;
    s << "L1A position: " << ctpRes.l1AcceptBunchPosition() << std::endl;

    // Print header info
    s << "Header information:  " << std::endl;
    s << "    Header marker           :  " << std::hex << ctpRes.headerMarker() << std::dec << std::endl;
    s << "    Header size             :  " << ctpRes.headerSize() << std::endl;
    s << "    Header format version   :  " << ctpRes.headerFormatVersion() << std::endl;
    s << "    Source ID               :  0x" << std::hex << ctpRes.sourceID() << std::dec << std::endl;
    s << "    Run number              :  " << ctpRes.runNumber() << std::endl;
    s << "    Ext. LVL1 ID            :  " << ctpRes.L1ID() << std::endl;
    s << "    BCID                    :  " << ctpRes.BCID() << std::endl;
    s << "    Trigger type            :  " << ctpRes.triggerType() << std::endl;
    s << "    Det. event type         :  " << ctpRes.eventType() << std::endl;

    // Print payload info
    s << "Payload information:  " << std::endl;
    s << "    Time " << ctpRes.timeSec() << "s "
        << std::setw(10) << std::setiosflags(std::ios_base::right) << std::setfill(' ')
        << ctpRes.timeNanoSec() << std::resetiosflags(std::ios_base::right)
        << "ns" << std::endl;

    // Print per-bunch information
    for(unsigned int i = 0; i<ctpRes.numberOfBunches(); ++i) {

      auto bunch = ctpRes.getBC(i);
      s << "    BC dump for bunch " << i << std::endl;

      // TIP words
      for(unsigned int j = 0; j<ctpRes.tipWords()[i].size(); ++j) {
        s << "        TIP word number " << j << ": 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ctpRes.tipWords()[i][j] << std::dec << std::endl;
      }
      if (ctpRes.tipWords()[i].size() == 0) {
        s << "        No TIP words!" << std::endl;
      }

      // TBP words
      for(unsigned int j = 0; j<ctpRes.tbpWords()[i].size(); ++j) {
        s << "        TBP word number " << j << ": 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ctpRes.tbpWords()[i][j] << std::dec << std::endl;
      }
      if (ctpRes.tbpWords()[i].size() == 0) {
        s << "        No TBP words!" << std::endl;
      }

      // TAP words
      for(unsigned int j = 0; j<ctpRes.tapWords()[i].size(); ++j) {
        s << "        TAP word number " << j << ": 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ctpRes.tapWords()[i][j] << std::dec << std::endl;
      }
      if (ctpRes.tapWords()[i].size() == 0) {
        s << "        No TAP words!" << std::endl;
      }

      // TAV words
      for(unsigned int j = 0; j<ctpRes.tavWords()[i].size(); ++j) {
        s << "        TAV word number " << j << ": 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ctpRes.tavWords()[i][j] << std::dec << std::endl;
      }
      if (ctpRes.tavWords()[i].size() == 0) {
        s << "        No TAV words!" << std::endl;
      }
    }

    // Additional words
    for(unsigned int i = 0; i<ctpRes.additionalWords().size(); ++i) {
      s << "    Additional word number " << i << ": 0x" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << ctpRes.additionalWords()[i] << std::dec << std::endl;
    }
    if (ctpRes.additionalWords().size() == 0) {
      s << "    No additional words!" << std::endl;
    }

    // Print trailer info
    s << "Trailer information:  " << std::endl;
    s << "    Error status                :  " << ctpRes.errorStatus() << std::endl;
    s << "    Status info                 :  " << ctpRes.infoStatus() << std::endl;
    s << "    Number of status words      :  " << ctpRes.numStatusWords() << std::endl;
    s << "    Number of data words        :  " << ctpRes.numDataWords() << std::endl;
    s << "    Status information position :  " << ctpRes.statusPosition() << std::endl;
    s << "*END* xAOD::CTPResult" << std::endl;

    return s.str();
  }


} // namespace CTPResultUtils
