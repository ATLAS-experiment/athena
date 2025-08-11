/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file AtlCopyBSEvent.cxx
 * $Author: hma $
 * $Revision: 1.3 $
 * $Date: 2008-10-01 18:26:56 $
 *
 */
 
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include <ctype.h>
#include <stdlib.h>

#include "eformat/eformat.h"
#include "eformat/old/util.h"
#include "EventStorage/pickDataReader.h"
#include "EventStorage/DataWriter.h" 

#include "CxxUtils/checker_macros.h"

void eventLoop(DataReader*, EventStorage::DataWriter*, unsigned&, const std::vector<uint64_t>*, uint32_t, bool, bool, bool, const std::vector<long long int>* = 0);

int main ATLAS_NOT_THREAD_SAFE (int argc, char *argv[]) {
  using namespace eformat;

  //Interpret arguments
  if(argc<3) {
    std::cerr << "usage: " << argv[0] 
	      << " [-d --deflate] -e [--event] <eventNumbers> [-r, --run <runnumber>] [-l, --listevents] [-t --checkevents] -o, --out outputfile inputfiles...." << std::endl;
    std::cerr << "eventNumbers is a comma-separated list of events" << std::endl;
    std::exit(1);
  }

  std::string fileNameOut("extractedEvents.data");
  std::vector<std::string> fileNames;
  std::vector<uint64_t> searchEvents;
  uint32_t searchRun=0; 
  bool searchRunSet=false;
  bool listEvents=false;
  bool checkEvents=false;
  bool compressEvents=false;
  unsigned nFound=0;
  for (int i=1; i<argc; i++) {
    const std::string& arg1(argv[i]);
    if (arg1=="-d" || arg1=="--deflate") {
      compressEvents=true;
    } else if (arg1=="-e" || arg1=="--event") {
      //try read the list of event number
      std::string arg2;
      if ((i+1) < argc) arg2=argv[i+1];
      if (arg2.size()>0 && isdigit(arg2[0])) {
	size_t p=0;
	while (p!=std::string::npos) {
	  searchEvents.push_back(atoll(arg2.c_str()+p));
	  p=arg2.find(',',p);
	  if (p!=std::string::npos) p++;
	}
      } else if (arg2=="all") {
	std::cout << "Copy all events" << std::endl;
      } else {
	std::cout << "ERROR: no numerical argument found after '" << arg1 << "'" << std::endl;
	return -1;
      }
      i++;
    } else if (arg1=="-o" || arg1=="--out") {
      std::string arg2;
      if ((i+1) < argc) arg2=argv[i+1];
      //set output file name
      if (arg2.size()>0) fileNameOut=arg2;
      else {
	std::cout << "ERROR: Expected output file name after '" << arg1 << "'" <<std::endl;
	return -1;
      }
      i++;
    } else if (arg1=="-r" || arg1=="--run") {
      std::string arg2;
      if ((i+1) < argc) arg2=argv[i+1];
      //try read event number
      if (arg2.size()>0 && isdigit(arg2[0])) searchRun=atoi(arg2.c_str());
      else {
	std::cout << "ERROR: no numerical argument found after '" << arg1 << "'" << std::endl;
	return -1;
      }
      i++;
      searchRunSet=true;
    } else if (arg1=="-t" || arg1=="--checkevents") {
      checkEvents=true;
    } else if (arg1=="-l" || arg1=="--listevents") {
      listEvents=true;
    } else {
      fileNames.push_back(arg1);
    }
  }// End loop over arguments

  std::sort(searchEvents.begin(),searchEvents.end());
  std::cout << "Events to copy: ";
  for (std::vector<uint64_t>::const_iterator itEvt1=searchEvents.begin(), itEvt2=searchEvents.end(); itEvt1!=itEvt2; ++itEvt1) {
    std::cout << *itEvt1 << " ";
  }
  std::cout << std::endl;

  std::unique_ptr<EventStorage::DataWriter> pDW;
  //start loop over files
  for (std::vector<std::string>::const_iterator it = fileNames.begin(), it_e = fileNames.end(); it != it_e; ++it) {
    const std::string& fName=*it;
    std::cout << "Checking file " << fName << std::endl;
    std::unique_ptr<DataReader> pDR(pickDataReader(fName));
    if (!pDR) {
      std::cout << "Problem opening or reading this file!\n";
      return -1;
    }

    if (!pDW) { //Create data writer when reading the first file
      //Create DataWriter
      //Copy run_parameters_pattern from first file:
      EventStorage::run_parameters_record runPara = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      runPara.run_number=pDR->runNumber();  
      runPara.max_events=pDR->maxEvents(); 
      runPara.rec_enable=pDR-> recEnable();    
      runPara.trigger_type=pDR-> triggerType();  
      std::bitset<64> word1;
      std::bitset<64> word2;
      for (unsigned int i=0; i<64; ++i) {
         word1[i] = pDR->detectorMask()[i];
         word2[i] = pDR->detectorMask()[i+64];
      }
      runPara.detector_mask_LS=word1.to_ulong();
      runPara.detector_mask_MS=word2.to_ulong();
      runPara.beam_type=pDR->beamType();
      runPara.beam_energy=pDR->beamEnergy();

      std::string shortFileNameOut=fileNameOut;
      const std::string project(pDR->projectTag());
      const std::string streamName("");
      const std::string streamType("");
      const std::string stream(pDR->stream());
      const uint32_t lbnbr(pDR->lumiblockNumber());

      std::string dirNameOut=".";
      size_t p = fileNameOut.rfind('/');
      if (p != std::string::npos) {
	dirNameOut = fileNameOut.substr(0, p);
	shortFileNameOut = fileNameOut.substr(p + 1);
      }

      EventStorage::freeMetaDataStrings metaStrings;
      if (compressEvents) {
        pDW=std::make_unique<EventStorage::DataWriter>(dirNameOut, shortFileNameOut, runPara, project, streamType,
	        streamName, stream, lbnbr, "AtlCopyBSEvent", metaStrings, EventStorage::ZLIB);
      } else {
        pDW= std::make_unique<EventStorage::DataWriter>(dirNameOut, shortFileNameOut, runPara, project, streamType,
	        streamName, stream, lbnbr, "AtlCopyBSEvent", metaStrings);
      }
      pDW->setMaxFileMB(10000); //Max 10 metric GByte files
      if (!pDW->good() ) { 
	std::cout << "ERROR  Unable to initialize file "<< std::endl;
	return -1;
      }    
      std::cout << "Created DataWriter for file " << shortFileNameOut << " in directory " << dirNameOut << std::endl;
    }
    if (!pDR->good() || pDR->endOfFile()) {
      std::cerr << "No events in file "<< fName << std::endl;
      continue;
    }
    eventLoop(pDR.get(), pDW.get(), nFound, &searchEvents, searchRun, searchRunSet, listEvents, checkEvents);
    if (nFound >= searchEvents.size() && nFound) break;
  }
  if (!nFound && searchEvents.size() > 0) {
    std::cout << "No events found!"  << std::endl;
    //return -1;  // Some use cases don't expect to find the events, just issue message
    return 0;
  } else if (nFound) {
    std::cout << "Wrote " << nFound << " events to file " << fileNameOut  << std::endl;
  } else {
    std::cout << "Copied all events to file " << fileNameOut  << std::endl;
  }
  return 0;
}

void eventLoop(DataReader* pDR, EventStorage::DataWriter* pDW, unsigned& nFound, const std::vector<uint64_t>* pSearchEvents, uint32_t searchRun, bool searchRunSet, bool listEvents, bool checkEvents, const std::vector<long long int>* pOffsetEvents) {
  using namespace eformat;
  // the event loop
  uint32_t eventCounter=0;
  std::vector<long long int>::const_iterator offIt, offEnd;
  if (pOffsetEvents != 0) {
    offIt = pOffsetEvents->begin();
    offEnd = pOffsetEvents->end();
  }
  while (pDR->good()) {
    unsigned int eventSize;    
    char *buf=nullptr;
    DRError ecode;
    if (pOffsetEvents != 0) {
      if (offIt == offEnd) break;
      ecode = pDR->getData(eventSize, &buf, *offIt);
      ++offIt;
    } else {
      ecode = pDR->getData(eventSize,&buf);
    }
    std::unique_ptr<uint32_t[]> fragment(reinterpret_cast<uint32_t*>(buf));
    if (DROK != ecode) {
      std::cout << "Can't read from file!" << std::endl;
      break;
    }
    ++eventCounter;
    
    // make a fragment with eformat 3.0 and check it's validity
    try {
      if ((eformat::HeaderMarker)(fragment[0])!=FULL_EVENT) {
        std::cout << "Event doesn't start with full event fragment (found " 
                << std::ios::hex << fragment[0] << ") ignored." <<std::endl;
        continue;
      }
      const uint32_t formatVersion = eformat::helper::Version(fragment[3]).major_version();
      //convert to new version if necessary
      if (formatVersion != eformat::MAJOR_DEFAULT_VERSION) {
        // 100 for increase of data-size due to header conversion
        uint32_t newEventSize = eventSize + 1000;
        auto newFragment=std::make_unique<uint32_t[]>(newEventSize);
	      eformat::old::convert(fragment.get(),newFragment.get(),newEventSize);
	      // set new pointer
	      fragment = std::move(newFragment);
      }
      FullEventFragment<const uint32_t*> fe(fragment.get());
      if (checkEvents) fe.check_tree();
      
      uint64_t eventNo=fe.global_id();
      uint32_t runNo=fe.run_no();
      if (listEvents)
        std::cout << "Index=" << eventCounter <<" Run=" << runNo << " Event=" << eventNo 
                << " LB=" <<  fe.lumi_block() <<std::endl;
      
      if ((!searchRunSet || (runNo==searchRun)) && std::binary_search(pSearchEvents->begin(),pSearchEvents->end(),eventNo)) {
        nFound++;
        std::cout << std::endl;
        std::cout << "File:" <<  pDR->fileName() << std::endl;
        std::cout << "Event index: " << eventCounter << std::endl;
        std::cout << "Run:         " << runNo << std::endl;   
        std::cout << "Event ID:    " << eventNo << std::endl;
        std::cout << "LumiBlock:   "  << fe.lumi_block() << std::endl;
      
        //Write event to file
        uint32_t size = fe.fragment_size_word(); 
        pDW->putData(sizeof(uint32_t)*size, reinterpret_cast<void *>(fragment.get()));
      } else if (pSearchEvents->size() == 0) {
        //Write event to file
        uint32_t size = fe.fragment_size_word(); 
        pDW->putData(sizeof(uint32_t)*size, reinterpret_cast<void *>(fragment.get()));
      }
    } catch (eformat::Issue& ex) {
      std::cerr << "Uncaught eformat issue: " << ex.what() << std::endl;
    } catch (ers::Issue& ex) {
      std::cerr << "Uncaught ERS issue: " << ex.what() << std::endl;
    } catch (std::exception& ex) {
      std::cerr << "Uncaught std exception: " << ex.what() << std::endl;
    } catch (...) {
      std::cerr << std::endl << "Uncaught unknown exception" << std::endl;
    }
    // end event processing 
  }
}
