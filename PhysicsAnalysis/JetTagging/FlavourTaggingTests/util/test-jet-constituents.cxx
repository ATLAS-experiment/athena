// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODJet/JetContainer.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODBase/IParticle.h"
#include "AthContainers/AuxElement.h"

#include "AsgMessaging/MessageCheck.h"

#include "TFile.h"
#include "TTree.h"
#include "TError.h"

#include <iostream>
#include <iomanip>

//coverity[root_function]
int main ATLAS_NOT_THREAD_SAFE (int argc, char *argv[]) {

  ANA_CHECK_SET_TYPE (int);
  using namespace asg::msgUserCode;
  gErrorIgnoreLevel = kError;

  if (argc < 3 || argc > 4) {
    std::cerr << "usage: " << argv[0] << ": <DAOD> <jet collection> [max events]"
              << "\n\n"
              << "This test checks jet constituents and their thinning.\n"
              << "It verifies that:\n"
              << "  1. All jet constituent links are valid\n"
              << "  2. All constituents can be cast to FlowElement\n"
              << "  3. Counts charged vs neutral constituents\n"
              << "  4. Checks originalObjectLink availability\n"
              << "  5. Validates otherObjects pointers\n"
              << "\n"
              << "Arguments:\n"
              << "  <DAOD>          : Input DAOD file path\n"
              << "  <jet collection>: Name of jet collection to check\n"
              << "  [max events]    : Optional maximum number of events to process\n"
              << "\n"
              << "Return codes:\n"
              << " -1: usage error\n"
              << "  1: broken constituent links found\n"
              << "  2: constituent not a FlowElement\n"
              << "  3: invalid otherObjects in charged FlowElements\n"
              << "  4: invalid otherObjects in neutral FlowElements\n"
              << "  0: all constituents valid" << std::endl;
    return -1;
  }
  std::string file = argv[1];
  std::string jets_name = argv[2];
  long long maxEvents = -1;
  if (argc == 4) {
    maxEvents = std::stoll(argv[3]);
  }

  // The name of the application:
  const std::string APP_NAME = "TestJetConstituents";

  // Set up the environment:
  ANA_CHECK( xAOD::Init() );

  // Set up the event object:
  xAOD::TEvent event(xAOD::TEvent::kAthenaAccess);

  // Open the file:
  std::unique_ptr<TFile> ifile(TFile::Open(file.c_str(), "READ"));
  if ( ! ifile.get() || ifile->IsZombie()) {
    std::cerr << "Couldn't open file: " << file << std::endl;
    return 1;
  }

  // Connect the event object to it:
  ANA_CHECK( event.readFrom(ifile.get()) );

  unsigned long long nJetsTotal = 0;
  unsigned long long nConstituentsTotal = 0;
  unsigned long long nChargedTotal = 0;
  unsigned long long nNeutralTotal = 0;
  unsigned long long nInvalidLinks = 0;
  unsigned long long nNotFlowElement = 0;
  unsigned long long nWithOriginalLink = 0;
  unsigned long long nOtherObjectsCharged = 0;
  unsigned long long nOtherObjectsNeutral = 0;
  unsigned long long nInvalidOtherObjectsCharged = 0;
  unsigned long long nInvalidOtherObjectsNeutral = 0;
  unsigned long long nMissingSignalType = 0;

  unsigned long long entries = event.getEntries();
  if (maxEvents > 0 && maxEvents < (long long)entries) {
    entries = maxEvents;
  }
  std::cout << "Processing " << entries << " events..." << std::endl;

  for (unsigned long long entry = 0; entry < entries; ++entry) {
    // Load the event:
    if (event.getEntry(entry) < 0) {
      std::cerr << "Couldn't load entry " << entry << " from file "
                << file << std::endl;
      return 1;
    }

    const xAOD::JetContainer *jets = nullptr;
    ANA_CHECK( event.retrieve(jets, jets_name) );

    for (const xAOD::Jet *const jet : *jets) {
      nJetsTotal++;
      size_t nConstituents = jet->numConstituents();
      nConstituentsTotal += nConstituents;

      for (size_t i = 0; i < nConstituents; ++i) {
        const auto& link = jet->constituentLinks().at(i);

        // Check if link is valid
        if (!link.isValid()) {
          nInvalidLinks++;
          continue;
        }

        // Get the actual constituent - might need to follow originalObjectLink
        const xAOD::IParticle* constituent = *link;

        // Check if this is a view container element that needs dereferencing
        static SG::AuxElement::ConstAccessor<ElementLink<xAOD::IParticleContainer>> originalAcc("originalObjectLink");
        if (originalAcc.isAvailable(*constituent)) {
          const ElementLink<xAOD::IParticleContainer>& origLink = originalAcc(*constituent);
          if (origLink.isValid()) {
            constituent = *origLink;
            nWithOriginalLink++;
          }
        }

        // Try to cast to FlowElement
        const xAOD::FlowElement* flowElement = dynamic_cast<const xAOD::FlowElement*>(constituent);
        if (!flowElement) {
          nNotFlowElement++;
          continue;
        }

        // Check if charged or neutral
        bool isCharged = flowElement->isCharged();
        if (isCharged) {
          nChargedTotal++;
        } else {
          nNeutralTotal++;
        }

        // Check otherObjects and their validity
        std::vector<const xAOD::IParticle*> others = flowElement->otherObjects();
        for (const xAOD::IParticle* other : others) {
          if (isCharged) {
            nOtherObjectsCharged++;
            if (!other) {
              nInvalidOtherObjectsCharged++;
            }
          } else {
            nOtherObjectsNeutral++;
            if (!other) {
              nInvalidOtherObjectsNeutral++;
            }
          }
        }
      }
    }
  }

  // Print summary
  std::cout << "\n========================================" << std::endl;
  std::cout << "SUMMARY" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Events processed:           " << entries << std::endl;
  std::cout << "Jets found:                 " << nJetsTotal << std::endl;
  std::cout << "Total constituents:         " << nConstituentsTotal << std::endl;
  std::cout << "  Charged:                  " << nChargedTotal
            << " (" << std::fixed << std::setprecision(1)
            << (nConstituentsTotal > 0 ? 100.0 * nChargedTotal / nConstituentsTotal : 0.0)
            << "%)" << std::endl;
  std::cout << "  Neutral:                  " << nNeutralTotal
            << " (" << std::fixed << std::setprecision(1)
            << (nConstituentsTotal > 0 ? 100.0 * nNeutralTotal / nConstituentsTotal : 0.0)
            << "%)" << std::endl;
  std::cout << "Invalid constituent links:  " << nInvalidLinks << std::endl;
  std::cout << "Non-FlowElement constituents: " << nNotFlowElement << std::endl;
  std::cout << "Missing signalType:         " << nMissingSignalType << std::endl;
  std::cout << "Constituents with originalObjectLink: " << nWithOriginalLink << std::endl;
  std::cout << "\nOther Objects:" << std::endl;
  std::cout << "  Charged otherObjects:     " << nOtherObjectsCharged;
  if (nInvalidOtherObjectsCharged > 0) {
    std::cout << " (INVALID: " << nInvalidOtherObjectsCharged << ")";
  }
  std::cout << std::endl;
  std::cout << "  Neutral otherObjects:     " << nOtherObjectsNeutral;
  if (nInvalidOtherObjectsNeutral > 0) {
    std::cout << " (INVALID: " << nInvalidOtherObjectsNeutral << ")";
  }
  std::cout << std::endl;
  std::cout << "  Total otherObjects:       " << (nOtherObjectsCharged + nOtherObjectsNeutral) << std::endl;

  if (nConstituentsTotal > 0) {
    std::cout << "\nAverage constituents/jet:   " << std::fixed << std::setprecision(2)
              << (double)nConstituentsTotal / nJetsTotal << std::endl;
  }
  std::cout << "========================================\n" << std::endl;

  // Return codes
  if (nInvalidLinks > 0) {
    std::cerr << "ERROR: Found " << nInvalidLinks << " invalid constituent links!" << std::endl;
    return 1;
  }
  if (nNotFlowElement > 0) {
    std::cerr << "ERROR: Found " << nNotFlowElement << " constituents that are not FlowElements!" << std::endl;
    return 2;
  }
  if (nInvalidOtherObjectsCharged > 0) {
    std::cerr << "ERROR: Found " << nInvalidOtherObjectsCharged << " invalid otherObjects in charged FlowElements!" << std::endl;
    return 3;
  }
  if (nInvalidOtherObjectsNeutral > 0) {
    std::cerr << "ERROR: Found " << nInvalidOtherObjectsNeutral << " invalid otherObjects in neutral FlowElements!" << std::endl;
    return 4;
  }

  std::cout << "SUCCESS: All constituent links are valid!" << std::endl;
  return 0;
}
