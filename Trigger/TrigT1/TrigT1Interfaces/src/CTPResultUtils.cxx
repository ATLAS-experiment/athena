/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Includes
#include "AthContainers/AuxElement.h"

// Local include
#include "TrigT1Interfaces/CTPResultUtils.h"

// TDAQ include
#include "CTPfragment/CTPdataformatVersion.h"

namespace CTPResultUtils {

  // Initialize takes the number of BCs of the readout window as argument
  void initialize(xAOD::CTPResult& ctpRes, uint32_t ctpVersionNumber, const uint32_t nBCs, uint32_t nExtraWords) {   
    CTPdataformatVersion ctpDataFormat(ctpVersionNumber);
    ctpRes.setCtpVersionNumber(ctpVersionNumber);
    setNumberOfBunches(ctpRes, nBCs);
    setNumberOfAdditionalWords(ctpRes, nExtraWords);

    // create correct length, zero filled data member
    std::vector<uint32_t> words;
    words.resize(ctpDataFormat.getNumberTimeWords()+(nBCs*ctpDataFormat.getDAQwordsPerBunch() )+nExtraWords);
    ctpRes.setDataWords(words);
  }

  // Initialize taking the data words of one or several BCs as argument
  void initialize(xAOD::CTPResult& ctpRes, uint32_t ctpVersionNumber, const std::vector<uint32_t>& data, uint32_t nExtraWords) {
    CTPdataformatVersion ctpDataFormat(ctpVersionNumber);
    ctpRes.setCtpVersionNumber(ctpVersionNumber);
    ctpRes.setDataWords(data);
    setNumberOfAdditionalWords(ctpRes, nExtraWords);

    //additional initialisation
    if (!ctpRes.dataWords().size()) {
      setNumberOfBunches(ctpRes, 0u);
    } else {
      uint32_t words = static_cast<uint32_t>(ctpRes.dataWords().size()) - ctpDataFormat.getNumberTimeWords() - nExtraWords;
      setNumberOfBunches(ctpRes, words / ctpDataFormat.getDAQwordsPerBunch());
    }
  }

  // get the time in sec
  uint32_t getTimeSec(const xAOD::CTPResult& ctpRes) {
    CTPdataformatVersion ctpDataFormat(ctpRes.ctpVersionNumber());
    if(ctpRes.dataWords().size() <= ctpDataFormat.getTimeSecondsPos()){
      return 0;
    } 
    return ctpRes.dataWords().at(ctpDataFormat.getTimeSecondsPos());
  }

  // set the time in sec
  void setTimeSec(xAOD::CTPResult& ctpRes, const uint32_t sec) {
    CTPdataformatVersion ctpDataFormat(ctpRes.ctpVersionNumber());
    std::vector<uint32_t> words = ctpRes.dataWords();
    if (ctpDataFormat.getTimeSecondsPos() < words.size()) {
      words[ctpDataFormat.getTimeSecondsPos()] = sec;
      ctpRes.setDataWords(words);
    }
  }

  // get the time in nanosec
  uint32_t getTimeNanoSec(const xAOD::CTPResult& ctpRes) {
    CTPdataformatVersion ctpDataFormat(ctpRes.ctpVersionNumber());
    if(ctpRes.dataWords().size() <= ctpDataFormat.getTimeNanosecondsPos()) {
      return 0; 
    } 
    return (ctpRes.dataWords().at(ctpDataFormat.getTimeNanosecondsPos() ) >> ctpDataFormat.getTimeNanosecondsOffset() ) * ctpDataFormat.getTimeNanosecondsTicks();
  }

  // set the time in nanosec
  void setTimeNanoSec(xAOD::CTPResult& ctpRes, const uint32_t nano) {
    CTPdataformatVersion ctpDataFormat(ctpRes.ctpVersionNumber());
    std::vector<uint32_t> words = ctpRes.dataWords();
    if(ctpDataFormat.getTimeNanosecondsPos() < words.size()) {
      words[ctpDataFormat.getTimeNanosecondsPos()] = ((nano/ctpDataFormat.getTimeNanosecondsTicks()) << ctpDataFormat.getTimeNanosecondsOffset());
      ctpRes.setDataWords(words);
    }
  }

  // set the number of bunches
  void setNumberOfBunches(xAOD::CTPResult& ctpRes, const uint32_t nBCs) {
    if(nBCs > ctpRes.numberOfBunches()) {
      static const SG::AuxElement::Accessor< uint32_t > acc("numberOfBunches");
      acc(ctpRes) = nBCs;
      CTPdataformatVersion ctpDataFormat(ctpRes.ctpVersionNumber());
      std::vector<uint32_t> words = ctpRes.dataWords();
      words.resize(ctpDataFormat.getNumberTimeWords()+(nBCs*ctpDataFormat.getDAQwordsPerBunch())+ctpRes.numberOfAdditionalWords());
      ctpRes.setDataWords(words);
    }
  }

  // set the number of additional words
  void setNumberOfAdditionalWords(xAOD::CTPResult& ctpRes, const uint32_t nExtraWords) {
    if(nExtraWords > ctpRes.numberOfAdditionalWords()) {
      CTPdataformatVersion ctpDataFormat(ctpRes.ctpVersionNumber());
      std::vector<uint32_t> words = ctpRes.dataWords();
      words.resize(ctpRes.dataWords().size()+nExtraWords-ctpRes.numberOfAdditionalWords());
      ctpRes.setDataWords(words);
    }
    static const SG::AuxElement::Accessor< uint32_t > acc("numberOfAdditionalWords");
    acc(ctpRes) = nExtraWords;
  }

  // get TIP words
  std::vector<uint32_t> getTIPWords(const xAOD::CTPResult& ctpRes) {return getWords(ctpRes, WordType::TIP);}
  
  // get TBP words
  std::vector<uint32_t> getTBPWords(const xAOD::CTPResult& ctpRes) {return getWords(ctpRes, WordType::TBP);}
  
  // get TAP words
  std::vector<uint32_t> getTAPWords(const xAOD::CTPResult& ctpRes) {return getWords(ctpRes, WordType::TAP);}
  
  // get TAV words
  std::vector<uint32_t> getTAVWords(const xAOD::CTPResult& ctpRes) {return getWords(ctpRes, WordType::TAV);}
  
  // get Extra words
  std::vector<uint32_t> getExtraWords(const xAOD::CTPResult& ctpRes) {return getWords(ctpRes, WordType::Extra);}

  // helper function to get words
  std::vector<uint32_t> getWords(const xAOD::CTPResult& ctpRes, CTPResultUtils::WordType type) {
    unsigned int nWords = 0;
    unsigned int offset = 0;
    std::vector<uint32_t> vec;
    vec.clear();
    CTPdataformatVersion ctpDataFormat(ctpRes.ctpVersionNumber());
    switch (type) {
    case WordType::TIP:
        nWords = ctpDataFormat.getTIPwords();
        offset = ctpDataFormat.getTIPpos();
        break;
    case WordType::TBP:
        nWords = ctpDataFormat.getTBPwords();
        offset = ctpDataFormat.getTBPpos();
        break;
    case WordType::TAP:
        nWords = ctpDataFormat.getTAPwords();
        offset = ctpDataFormat.getTAPpos();
        break;
    case WordType::TAV:
        nWords = ctpDataFormat.getTAVwords();
        offset = ctpDataFormat.getTAVpos();
        break;
    case WordType::Extra:
        vec.assign(ctpRes.dataWords().begin() + (ctpRes.dataWords().size()-ctpRes.numberOfAdditionalWords()), ctpRes.dataWords().end());
        return vec;
        break;
    default:
        break;
    }
    for(unsigned int bunch = 0 ; bunch < ctpRes.numberOfBunches() ; ++bunch) {
      for(unsigned int tbp = 0; tbp < nWords; ++tbp) {
          // take offset of TBPwords into account
          unsigned int index = offset + tbp;
          // go to the correct bunch
          index += (bunch * ctpDataFormat.getDAQwordsPerBunch());
          if( index < ctpRes.dataWords().size() ) {
            vec.push_back(ctpRes.dataWords()[index]);
          }
      }
    }
    // this is now a list of consecutive data words for all bunches
    return vec;
  }


} // namespace CTPResultUtils
