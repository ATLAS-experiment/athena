/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGINGGRIDBOUNDSHANDLER_H
#define BTAGGINGGRIDBOUNDSHANDLER_H

#include "xAODBTaggingEfficiency/BTaggingBoundsHandler.h"
#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODJet/Jet.h"
#include <string>
#include <vector> 
#include <map>
#include <set>
#include <stdexcept>
#include <utility>
#include <format>

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
    // Vector that holds the list of handlers 
    std::vector<handlerClass> m_handlers;
    // Vector that holds the list of variables that is used by the handlers 
    // e.g. pT, mass, abseta etc 
    std::vector<BTaggingToolUtil::varType> m_varTypes;
    // Pointer to the function used for retrieving the jet quantities 
    BTaggingBoundsHandler::JetQuantityGetter* m_funcGetJetQuantity = nullptr;
    // This map allows to group for each variable the handlers that are falling in the same range 
    // Here storing pointers of the handlers as the handler are stored in the m_handlers vector already 
    // In this way the matching of a jet with the corresponding handler is faster as it avoids considering handlers that are not relevant 
    // std::less<> comparator allows to search the varBounds map directly with a jet value
    // i.e. m_varBoundsSetHandlers.at(vType).find(jetValue) using the varBounds/float operator<
    // This works because the varBounds stored in the map are disjoint i.e. not overlapping (and sorted)
    // See MR https://gitlab.cern.ch/atlas/athena/-/merge_requests/92635
    // for more information 
    std::map<BTaggingToolUtil::varType, std::map<BTaggingBoundsHandler::varBounds, std::set<const handlerClass*>, std::less<>>> m_varBoundsSetHandlers;
};

template<class handlerClass>
BTaggingGridBoundsHandler<handlerClass>::BTaggingGridBoundsHandler(const std::string & context, std::vector<handlerClass>&& handlers)
  : m_handlers(std::move(handlers))
{
  // If the vector of handler is empty raise error 
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

  // Get the list of variables used by the handlers. 
  // Since the same variables are used for all handlers (check done in the overlap check above)
  // retrieve that list from the first handler in the vector 
  // It has been checked above the vector is never empty 
  m_varTypes = m_handlers.at(0).getListVariables();

  // Get the jet quantity function 
  // The overlap check above ensures that the function that returns the jet quantity is the same 
  // accross all handlers hence retrieving it from the first handler 
  m_funcGetJetQuantity = m_handlers.at(0).m_funcGetJetQuantity;

  // For sanity check let's make sure that this function is not a null pointer 
  // and it points to a function that has been initialized 
  if (!m_funcGetJetQuantity || !(*m_funcGetJetQuantity)){
    throw std::runtime_error("Function to get jet quantity is either a null pointer or points to std::function that has not been initialized for context=" + context);
  }


  // Now we want to group handlers by bounds range for each variable 
  // so the matching jet-handler will be faster in the getHandlerPtr function 
  // Loop over each variable 
  for (const BTaggingToolUtil::varType & vType : m_varTypes){

    // For each variable the varBounds vector 
    // will contains the bounds corresponding to the current variable 
    // for all handler
    std::vector<float> varBounds;

    // Get epsilon value for float comparison from the first handler for the current variable
    // The epsilon value should in any case be the same for all handlers
    float varEps = m_handlers.at(0).m_varBinBounds.at(vType).varEps;

    // Loop over each handler
    for (const handlerClass& handler : m_handlers){
      // // For debugging print the handler 
      // std::cout << "handler=" << handler << std::endl;

      // Retrieve the lower and upper bounds associated to the current variable and current handler 
      const BTaggingBoundsHandler::varBounds & hdlrBounds = handler.m_varBinBounds.at(vType);
      float lowerBound = hdlrBounds.lowerBound;
      float upperBound = hdlrBounds.upperBound;

      // // For debugging print information 
      // std::cout << "varName=" << BTaggingToolUtil::getVariableName(vType) << ", lowerBound=" << lowerBound << ", upperBound=" << upperBound << std::endl;
      bool addLowerBound = true;
      bool addUpperBound = true;
      // Loop over values already in varBounds 
      for (const float &value: varBounds){
        // Only do the check if the value is still considered to be added 
        // NB: lowerBound and upperBound are ensured to be different thanks to the handler checks 
        if (addLowerBound && BTaggingToolUtil::approxEqual(value, lowerBound, varEps)){
          // It means that lowerBound and the current value are the same 
          // here using approxEqual as comparing two floats 
          // so the lowerBound value is already in the varBounds vector 
          // and thus should not be added 
          addLowerBound = false;
        }
        // Same reasoning as above but for the upper bound 
        if (addUpperBound && BTaggingToolUtil::approxEqual(value, upperBound, varEps)){
          addUpperBound = false;
        }

        // If both lower and upper bound should not be added 
        // exist this loop there is no point on continuing 
        if (!addLowerBound && !addUpperBound){ break ; }
      }
      // If addLowerBound is still true it means that lowerBound value 
      // it not found in the vector thus add it 
      // otherwise do nothing 
      if (addLowerBound){
        varBounds.push_back(lowerBound);
      }
      // Same reasoning but for the upperBound value
      if (addUpperBound){ 
        varBounds.push_back(upperBound);
      }
    }
    // Once having processed all handlers 
    // then sort the varBounds vector by increasing values
    std::sort(varBounds.begin(), varBounds.end());
    // For debugging 
    // std::cout << "varName=" << BTaggingToolUtil::getVariableName(vType) << ", varBounds=" << std::format("{}", varBounds) << std::endl;

    // Now loop over the sorted vector 
    for (unsigned int i = 0; i < varBounds.size() - 1; i++){
      // Build the corresponding varBounds 
      BTaggingBoundsHandler::varBounds bounds;
      bounds.lowerBound = varBounds[i];
      bounds.upperBound = varBounds[i+1];
      bounds.varEps = varEps;

      // For sanity check make sure that lowerBound is strictly smaller than upperBound 
      if (bounds.lowerBound >= bounds.upperBound){ 
        throw std::runtime_error("bounds.lowerBound >= bounds.upperBound for context=" + context);
      }

      std::set<const handlerClass*> handlerSet;
      // Loop again over handlers
      for (const handlerClass& handler : m_handlers){
        // Retrieve the bounds of the current handler for the current  variable 
        const BTaggingBoundsHandler::varBounds & hdlrBounds = handler.m_varBinBounds.at(vType);
        
        // If there is an overlap between the bounds of the handler and the current bound that was just built 
        // it means the handler covers the current bounds range 
        // hence add the pointer to the handler to the set as the handler should be considered for that current bound
        // Otherwise the handler does not corresponds to the current bounds 
        if (hdlrBounds.overlaps(bounds)){
          handlerSet.insert( &handler );
        }
      }

      // If the set of handler overlapping with the current bound 
      // is not empty then add that set to the map 
      if (handlerSet.size()){
        m_varBoundsSetHandlers[vType][bounds] = std::move(handlerSet);
        // // For debugging 
        // std::cout << "varName=" << BTaggingToolUtil::getVariableName(vType) << ", bounds(lowerBounds, upperBound)=("<< bounds.lowerBound << ", " << bounds.upperBound  << "): " ;
        // // Iterate over the set 
        // bool firstHandler = true;
        // for (const handlerClass *handler : m_varBoundsSetHandlers[vType][bounds]){
        //   if (!firstHandler){
        //     std::cout << ", ";
        //   }
        //   firstHandler = false;
        //   // Print handler content 
        //   std::cout << (*handler);
        // }
        // std::cout << std::endl;
      }
    }
  }
}

template<class handlerClass>
const handlerClass* BTaggingGridBoundsHandler<handlerClass>::getHandlerPtr(const xAOD::Jet& jet) const
{ 

  // Perform the matching between the jet and the corresponding handler 
  // See MR https://gitlab.cern.ch/atlas/athena/-/merge_requests/92635 
  // for more explanation 

  bool firstVar = true; 
  std::set<const handlerClass*> handlersSetMatchingJetValues;

  // Loop over each variable
  for (const BTaggingToolUtil::varType & vType : m_varTypes){
    // Retrieve the corresponding jet value
    // e.g. the jet pT, mass, absolute eta or so 
    const float jetValue = (*m_funcGetJetQuantity)(jet, vType);

    // Find the set of handlers that are corresponding to the current jet value
    // i.e. the set associated to the varBounds such that lowerBound <= jetValue < upperBound
    // If the jet value falls outside of all bounds (out of range or in a hole)
    // then no handler can match the jet
    const auto itBounds = m_varBoundsSetHandlers.at(vType).find(jetValue);
    if (itBounds == m_varBoundsSetHandlers.at(vType).end()){
      // Jet value is out of bound there is no matching handler
      // thus return a null pointer
      return nullptr;
    }

    if (firstVar){
      // If it's the first variable 
      // Simply copy the set of handlers matching the jet value 
      handlersSetMatchingJetValues = itBounds->second;
      firstVar = false;
    }
    else {
      // If it is not the first variable 
      // handlersSetMatchingJetValues contains the 
      // the list of pointers to handlers whose bounds 
      // correspond to the jet values of the variables processed so far
      // The currentSet contains the list of handler pointers whose bounds 
      // contain the jet value corresponding to the current variable. 
      // Hence only keep the handlers present in both sets i.e. the intersection
      // such that handlersSetMatchingJetValues is the list of handler pointers 
      // matching the jet values for all variables processed up to now 
      // including the current one

      const std::set<const handlerClass*>& currentSet = itBounds->second;
      std::set<const handlerClass*> tmp;
      std::set_intersection(
        handlersSetMatchingJetValues.begin(), handlersSetMatchingJetValues.end(),
        currentSet.begin(), currentSet.end(),
        std::inserter(tmp, tmp.begin())
      );

      // So in the end handlersSetMatchingJetValues only contains 
      // the result of the intersection 
      handlersSetMatchingJetValues.swap(tmp);
    }

    if (!handlersSetMatchingJetValues.size()){
      // The jet values make the jet out of bound
      // thus return a null pointer
      return nullptr;
    }
  }

  // If arriving here it means a match was found between an handler and the jet provided
  // otherwise a null pointer would have been returned earlier 
  // There can only be one handler matching the jet as there is no overlap between handlers 
  // If that's not the case there is a fundamental logical issue with the implementation of the current class
  // and so we should absolutely raise an error 
  if (handlersSetMatchingJetValues.size() != 1){
    throw std::runtime_error("handlersSetMatchingJetValues size is not 1, there is a fundamental logic issue in the BTaggingGridBoundsHandler class");
  }

  // If arriving here all good we have only one handler in the set 
  // retrieve it and return the pointer to that handler
  // that is matching the jet
  const handlerClass* handler = *handlersSetMatchingJetValues.begin();
  return handler;
}

#endif // BTAGGINGGRIDBOUNDSHANDLER_H
