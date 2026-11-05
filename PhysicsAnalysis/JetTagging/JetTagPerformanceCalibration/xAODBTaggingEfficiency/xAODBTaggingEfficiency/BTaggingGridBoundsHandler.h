/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGINGGRIDBOUNDSHANDLER_H
#define BTAGGINGGRIDBOUNDSHANDLER_H

#include "xAODBTaggingEfficiency/BTaggingBoundsHandler.h"
#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODJet/Jet.h"
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>

// Template class that owns a set of disjoint bounds handlers 
// i.e. N-dimensional bins and finds the one a jet falls into
// The grid can be irregular and can have holes
template<class handlerClass>
class BTaggingGridBoundsHandler {
  // Make sure the handlerClass is BTaggingBoundsHandler or is a child class of BTaggingBoundsHandler
  static_assert(
    std::is_base_of_v<BTaggingBoundsHandler, handlerClass>,
    "BTaggingGridBoundsHandler requires a BTaggingBoundsHandler or a child class of the BTaggingBoundsHandler class");

  public:
    // Takes the ownership of the handlers as they can't be copied
    // Throws if the handlers don't have the same variables or if they overlap
    BTaggingGridBoundsHandler(const std::string & context, std::vector<handlerClass>&& handlers);

    // Prevent grid handler from being copied forcing passing them by (const) references or using pointers
    // It ensures that the code is efficient memory wise as no duplicate can be created
    // The handlers should only be stored in vectors or maps and used directly basically
    BTaggingGridBoundsHandler& operator=(BTaggingGridBoundsHandler&&) noexcept = default;
    BTaggingGridBoundsHandler(BTaggingGridBoundsHandler&&) noexcept = default;
    BTaggingGridBoundsHandler(const BTaggingGridBoundsHandler&) = delete;
    BTaggingGridBoundsHandler& operator=(const BTaggingGridBoundsHandler&) = delete;
    ~BTaggingGridBoundsHandler() = default;

    // Returns the handler the jet falls into, or a null pointer if the jet is
    // outside of the grid or falls into a hole of the grid
    const handlerClass* getHandlerPtr(const xAOD::Jet& jet) const;

  private:
    std::vector<handlerClass> m_handlers;
};

template<class handlerClass>
BTaggingGridBoundsHandler<handlerClass>::BTaggingGridBoundsHandler(const std::string & context, std::vector<handlerClass>&& handlers)
  : m_handlers(std::move(handlers))
{
  // If the 
  if (m_handlers.empty()){ 
    throw std::runtime_error("Vector of handlers provided is empty for context=" + context);
  }

  // Check there is no overlap between the handlers 
  // This check also ensures that the same variables
  // are used acrross the handlers provided 
  try {
    BTaggingBoundsHandler::checkOverlaps<handlerClass>(context, m_handlers);
  }
  catch (const std::exception& e) {
    throw std::runtime_error("Handler overlap issue found, " + std::string(e.what()));
  }
}

template<class handlerClass>
const handlerClass* BTaggingGridBoundsHandler<handlerClass>::getHandlerPtr(const xAOD::Jet& jet) const
{
  // Loop over handlers 
  for (const handlerClass& handler : m_handlers){
    // Check if jet falls within the bounds of the current handler 
    if (handler.isJetWithinBounds(jet)) {
      // If that the case then return the pointer to the hander 
      return &handler;
    }
  }
  // If reaching this point it means the jet is out of the grid 
  // either out of range or in a hole of the grid 
  // Thus return a null pointer as there is not matching handler 
  return nullptr;
}

#endif // BTAGGINGGRIDBOUNDSHANDLER_H
