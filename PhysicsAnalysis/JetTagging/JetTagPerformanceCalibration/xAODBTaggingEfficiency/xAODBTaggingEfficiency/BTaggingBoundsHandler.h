/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGINGBOUNDSHANDLER_H
#define BTAGGINGBOUNDSHANDLER_H

#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODJet/Jet.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Parent class of all handlers defined by an N-dimensional bins
// i.e. a set of [low, high) bounds for a list of jet variables (e.g. pT, mass, abseta)
// It handles everything related to the bounds: parsing them from the JSON entry,
// checking if a jet falls into the bin, ordering and overlap of bins, printing of bounds.
// Child classes e.g. MCMCHandler only deal with the extra values 
// 
// More in details: 
// The json configuration that contains an entry called "binVariables". 
// The content that is not read by the BTaggingBoundsHandler can be stored in another entry e.g. called "content" 
// For example: 
// jsonConfig = { "binVariables": {"pT": [250, 300 ] , "mass": [50 , 75]}, "content":  {"SF": 0.9, uncerts:{ "SF_uncert_1": 0.1, "SF_uncert_2": 0.03}} }
// The binVariables can contain several variables with [low, high) values specified
// The getJetQuantity function is used to retrieve the jet values to compare to the bounds

class BTaggingBoundsHandler {
  public:
    // Function returning the value of a given variable for a jet e.g. the jet pT in GeV
    using JetQuantityGetter = std::function<float(const xAOD::Jet&, BTaggingToolUtil::varType)>;
    using json = nlohmann::ordered_json;

    BTaggingBoundsHandler(const json& jsonConfig, JetQuantityGetter funcGetJetQuantity);

    // Prevent handler from being copied forcing passing them by (const) references or using pointers
    // It ensures that the code is efficient memory wise as no duplicate can be created
    // The handlers should only be stored in vectors or maps and used directly basically
    BTaggingBoundsHandler& operator=(BTaggingBoundsHandler&&) noexcept = default;
    BTaggingBoundsHandler(BTaggingBoundsHandler&&) noexcept = default;
    BTaggingBoundsHandler(const BTaggingBoundsHandler&) = delete;
    BTaggingBoundsHandler& operator=(const BTaggingBoundsHandler&) = delete;
    virtual ~BTaggingBoundsHandler() = default;

    bool isJetWithinBounds(const xAOD::Jet& jet) const;

    // Two handlers i.e. N-dimensional bins overlap if their bounds overlap
    // for every variable. As soon as one variable has non overlapping bounds the bins are disjoint
    bool overlaps(const BTaggingBoundsHandler& o) const;

    // For printing the list of variables with lower and upper bounds
    // e.g. "pT: [250, 500], mass: [50, 100]"
    friend std::ostream& operator<<(std::ostream& os, const BTaggingBoundsHandler& handler);
    // To get the content of the handler as a string
    // Use the operator << function
    virtual std::string to_string() const {
      std::ostringstream oss;
      oss << *this;
      return oss.str();
    }

    // Check that the handlers i.e. N-dimensional bins of a set are disjoint
    template<class handlerClass>
    static void checkOverlaps(const std::string& context, const std::vector<handlerClass>& handlers) {
      for (size_t i = 0; i < handlers.size(); ++i) {
        for (size_t j = i + 1; j < handlers.size(); ++j) {
          if (handlers[i].overlaps(handlers[j])) {
            throw std::runtime_error("Overlapping bins for " + context + ": " + handlers[i].to_string() + " and " + handlers[j].to_string());
          }
        }
      }
    }

    // Check that, for each key of the map, the handlers of the associated set are disjoint
    template<class handlerClass>
    static void checkOverlaps(const std::string& context, const std::map<std::string, std::vector<handlerClass>>& handlersMap) {
      for (const auto& [key, handlers] : handlersMap) {
        checkOverlaps(context + " for label " + key, handlers);
      }
    }

    protected:
      // Function used to retrieve the jet values to compare to the bounds
      JetQuantityGetter m_funcGetJetQuantity;

      // Structure for storing a min and max value for a given variable
      // The varBounds structure also defines an ordering
      // which allows to sort bins in increasing or decreasing order basically
      // Finally an overlap function allows to check if two varBounds have some overlap
      struct varBounds {
        float lowerBound = std::numeric_limits<float>::lowest();
        float upperBound = std::numeric_limits<float>::max();

        // Epsilon value to compare lowerBound and upperBound values
        // This value can be changed if needed when initializing the structure
        float varEps = 1.e-6f;

        // Two half-open intervals [low, high) overlap if each one starts before the other ends.
        // Touching edges (upper = other lower) are not considered overlapping.
        bool overlaps(const varBounds& o) const;

        // Define an ordering of varBounds compare first lowerBound then upperBound
        // within tolerance
        bool operator<(const varBounds& o) const;
        bool operator>(const varBounds& o) const { return o < *this; }
        // Define also equal and not equal operator within tolerance
        bool operator==(const varBounds& o) const;
        bool operator!=(const varBounds& o) const { return !(*this == o); }
      };

      // Map that will contain the variable type and the bounds 
      std::map<BTaggingToolUtil::varType, varBounds> m_varBinBounds;

      // Ordering of handlers loop over variables as stored in the map
      // and let the first variable whose bounds differ decide of the ordering
      bool operator<(const BTaggingBoundsHandler& o) const;
      bool operator>(const BTaggingBoundsHandler& o) const { return o < *this; }
      // Define also equal and not equal operator
      // if neither is less than the other then handlers are equal
      bool operator==(const BTaggingBoundsHandler& o) const { return !(*this < o) && !(*this > o); }
      bool operator!=(const BTaggingBoundsHandler& o) const { return !(*this == o); }
      
      
};

#endif // BTAGGINGBOUNDSHANDLER_H
