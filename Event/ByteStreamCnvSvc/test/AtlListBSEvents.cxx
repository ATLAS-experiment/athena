/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
 
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <iomanip>
#include <ctype.h>
#include <stdlib.h>
#include <format>

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# pragma GCC diagnostic ignored "-Wparentheses"
#endif
#include "eformat/eformat.h"
#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
#include "eformat/old/util.h"
#include "eformat/index.h"
#include "EventStorage/pickDataReader.h"
#include <time.h>

#include "CxxUtils/checker_macros.h"

int main ATLAS_NOT_THREAD_SAFE (int argc, char *argv[])
{
  using namespace eformat;

  if(argc<2) {
    std::cerr << std::format("usage: {} [-s, --showsize] [-c, --checkevents] [-l, --listevents] [-m, --maxevents] files ...\n", argv[0]);
    return 1;
  }

  uint64_t totalSize=0;
  std::vector<uint64_t> totalSizePerSubDet(10,0);
  std::vector<std::string> fileNames;
  unsigned eventCounter=0;
  unsigned maxEvents=std::numeric_limits<unsigned>::max();
  bool listevents=false;
  bool checkevents=false;
  bool showSizes=false;
  for (int i=1;i<argc;i++) {
    const std::string& arg1(argv[i]);
    std::string arg2;
    if ((i+1) < argc) 
      arg2=argv[i+1];

    if (arg1=="-l" || arg1=="--listevents") {
      listevents=true;
    }
    else if (arg1=="-c" || arg1=="--checkevents") {
      checkevents=true;
    }
    else if (arg1=="-s" || arg1=="--showsize") {
      showSizes=true;
    } else if (arg1 == "-m" || arg1 == "--maxevents") {
      if (arg2.size() && isdigit(arg2[0]))
        maxEvents = atoi(arg2.c_str());
      else {
        std::cout << std::format(
            "ERROR: no numerical argument found after '{}'\n", arg1);
        return -1;
      }
      i++;
    }
    else
      fileNames.push_back(arg1);
  }// End loop over arguments

  if (!fileNames.size()) {
    std::cout << "ERROR: No file names set\n";
    return 1;
  }

  //start loop over files
  for (const std::string& fName : fileNames) {
    std::cout << std::format("Checking file {}\n", fName);
    std::unique_ptr<EventStorage::DataReader> pDR(pickDataReader(fName));

    if (!pDR) {
      std::cerr << "Problem opening or reading this file!\n";
      return 1;
    }

    if (!pDR->good()) {
      std::cout << std::format("No events in file {}\n", fName);
    }

    // Print file summary
    const std::vector<std::string> fmds = pDR->freeMetaDataStrings();
    std::cout << "File Metadata:\n";
    std::cout << std::format("         GUID: {}\n", pDR->GUID());
    std::cout << std::format("   Start time: {}\n", pDR->fileStartTime());
    std::cout << std::format("   Start date: {}\n", pDR->fileStartDate());
    std::cout << std::format("  Project Tag: {}\n", pDR->projectTag());
    std::cout << std::format("   Stream Tag: {}\n", pDR->stream());
    std::cout << std::format("   Lumi Block: {}\n", pDR->lumiblockNumber());
    std::cout << std::format("   Run Number: {}\n", pDR->runNumber());
    std::cout << " Free Strings: ";

    if (fmds.size() == 0)
      std::cout << "None\n";
    else {
      std::cout << fmds[0] << '\n';
      for (std::size_t i_fmds = 1; i_fmds < fmds.size(); ++i_fmds)
        std::cout << std::format("               {}\n", fmds[i_fmds]);
    }
    std::cout << "Start loop through events\n";

    // the event loop
    while (pDR->good() && eventCounter <= maxEvents) {
      unsigned int eventSize;
      char* buf = nullptr;

      DRError ecode = pDR->getData(eventSize, &buf);
      std::unique_ptr<uint32_t[]> fragment(reinterpret_cast<uint32_t*>(buf));
      if (DROK != ecode) {
        std::cerr << "Can't read from file!\n";
        return 1;
      }

      // make a fragment with eformat 3.0 and check it's validity
      try {
        if ((eformat::HeaderMarker)(fragment[0]) != FULL_EVENT) {
          std::cout << std::format(
              "Event doesn't start with full event fragment (found 0x{:x}) ignored.\n",
              fragment[0]);
          ++eventCounter;
          continue;
        }
        const uint32_t formatVersion = eformat::helper::Version(fragment[3]).major_version();
        // convert to new version if necessary
        if (formatVersion != eformat::MAJOR_DEFAULT_VERSION) {
          // 1000 for increase of data-size due to header conversion
          uint32_t newEventSize = eventSize + 1000;
          auto newFragment = std::make_unique<uint32_t[]>(newEventSize);
          eformat::old::convert(fragment.get(), newFragment.get(), newEventSize);
          // set new pointer
          fragment = std::move(newFragment);
        }
        FullEventFragment<const uint32_t*> fe(fragment.get());

        if (checkevents) {
          if (!fe.check_tree()) {
            std::cerr << std::format("Event {} failed check_tree\n",
                                     eventCounter);
            return 1;
          }
        }
        totalSize += fe.readable_payload_size_word() * sizeof(uint32_t);
        const uint64_t eventNo = fe.global_id();
        const uint32_t runNo = fe.run_no();
        const time_t sec = fe.bc_time_seconds();
        if (listevents) {
          std::cout << std::setprecision(2) << std::fixed;
          std::cout << "Index=" << eventCounter << " Run=" << runNo
                    << " Event=" << eventNo << " LB=" << fe.lumi_block()
                    << " Size="
                    << fe.fragment_size_word() * sizeof(uint32_t) / 1024.
                    << "kB (uncompr:"
                    << fe.readable_payload_size_word() * sizeof(uint32_t) /
                           1024.
                    << "kB)"
                    << " Offset=" << pDR->getPosition() << " "
                    << std::put_time(std::gmtime(&sec), "%Y-%m-%d:%H:%M:%S")
                    << " UTC\n";
        }
        if (showSizes) {
          std::map<eformat::SubDetectorGroup, std::vector<const uint32_t*>> robIndex;
          eformat::helper::build_toc(fe, robIndex);
          for (const auto& [sd, robs] : robIndex) {
            if (sd >= totalSizePerSubDet.size()) {
              totalSizePerSubDet.resize(1 + sd, 0);
            }
            uint64_t& thisSDSize = totalSizePerSubDet[sd];
            for (const auto& rob : robs) {
              ROBFragment<const uint32_t*> robFrag(rob);
              const unsigned robsize = robFrag.fragment_size_word() * sizeof(uint32_t);
              thisSDSize += robsize;
            }  // end loop over ROB fragments
          }  // end loop over subdets
        }  // end if showSizes
      } catch (eformat::Issue& ex) {
        std::cerr << std::format("Uncaught eformat issue: {}\n", ex.what());
        return 1;
      } catch (ers::Issue& ex) {
        std::cerr << std::format("Uncaught ERS issue: {}\n", ex.what());
        return 1;
      } catch (std::exception& ex) {
        std::cerr << std::format("Uncaught std exception: {}\n", ex.what());
        return 1;
      } catch (...) {
        std::cerr << "Uncaught unknown exception\n";
        return 1;
      }

      // end event processing
      ++eventCounter;
    }
  }

  //Print summary:
  std::cout.setf(std::ios::right | std::ios::fixed);
  std::cout.width(10);
  std::cout.precision(2);
  if (showSizes) {
    constexpr std::array<std::string_view,10> detnames{"    ANY (0x0)",
                                                       "  PIXEL (0x1)",
                                                       "    SCT (0x2)",
                                                       "    TRT (0x3)",
                                                       "    LAR (0x4)",
                                                       "TILECAL (0x5)",
                                                       "   MUON (0x6)",
                                                       "   TDAQ (0x7)",
                                                       "FORWARD (0x8)",
                                                       " L1Calo (0x9)"};

    std::cout << "\nAverage fragment size per subdetector:\n";
    uint64_t sum=0;
    for (unsigned sd=0;sd<totalSizePerSubDet.size();++sd) {
      std::string name;
      if (sd <detnames.size()) {
        name=detnames[sd];  
      }
      else {
        name = std::format("UNKNOWN (0x{:x})", sd);
      }

      const uint64_t s=totalSizePerSubDet[sd];
      if (s==0) continue; //Ignore if size is exactly 0
      sum+=s;
      double sPerEv=0;
      if (eventCounter>0) sPerEv=s/(1024.0*eventCounter); //In kB

      double fraction=0;
      if (totalSize>0) fraction=s/double(totalSize);
      std::cout << name << " :" << sPerEv << " kB/event (" << 100*fraction << "%)\n";
    }
    const int64_t overhead=totalSize-sum;
    double ohPerEv=0;
    double fraction=0;
    if (totalSize>0) fraction=overhead/double(totalSize);  
    if (eventCounter>0) ohPerEv=overhead/(double)eventCounter;
    std::cout << "     Overhead: " << overhead/1024.0 <<" kB or " << ohPerEv << " Bytes/event (" << 100*fraction << "%)\n";
  }

  std::cout << "Total: " << std::setprecision(2) << std::fixed << totalSize/(1024.0*eventCounter) << " kB/event" << std::endl; 
  return 0;
}
