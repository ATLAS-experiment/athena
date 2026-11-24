/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGINGBOUNDSHANDLER_H
#define BTAGGINGBOUNDSHANDLER_H

#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODJet/Jet.h"
#include <nlohmann/json_fwd.hpp>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
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
  // The grid needs the bounds of the handlers to build its look-up table
  template<class handlerClass> friend class BTaggingGridBoundsHandler;

  public:
    // Function returning the value of a given variable for a jet e.g. the jet pT in GeV
    using JetQuantityGetter = std::function<float(const xAOD::Jet&, BTaggingToolUtil::varType)>;
    using json = nlohmann::ordered_json;

    BTaggingBoundsHandler(const json& jsonConfig, JetQuantityGetter *funcGetJetQuantity);

    // Prevent handler from being copied forcing passing them by (const) references or using pointers
    // It ensures that the code is efficient memory wise as no duplicate can be created
    // The handlers should only be stored in vectors or maps and used directly basically
    BTaggingBoundsHandler& operator=(BTaggingBoundsHandler&&) noexcept = default;
    BTaggingBoundsHandler(BTaggingBoundsHandler&&) noexcept = default;
    BTaggingBoundsHandler(const BTaggingBoundsHandler&) = delete;
    BTaggingBoundsHandler& operator=(const BTaggingBoundsHandler&) = delete;
    virtual ~BTaggingBoundsHandler() = default;

    // Returns the list of variables of the handler 
    std::vector<BTaggingToolUtil::varType> getListVariables() const;

    // Check if jet falls in the bounds of the handler 
    bool isJetWithinBounds(const xAOD::Jet& jet) const;

    // Two handlers i.e. N-dimensional bins overlap if their bounds overlap
    // for every variable. As soon as one variable has non overlapping bounds the bins are disjoint
    bool overlaps(const BTaggingBoundsHandler& o) const;

    // For printing the list of variables with lower and upper bounds
    // e.g. "BTaggingBoundsHandler(pT: [250, 500], mass: [50, 100])"
    friend std::ostream& operator<<(std::ostream& os, const BTaggingBoundsHandler& handler) { return (os << handler.to_string()); }
    // To get the content of the handler as a string
    // Use the operator << function
    virtual std::string to_string() const;
    // To get only the bounds as string 
    // e.g. "pT: [250, 500], mass: [50, 100]"
    std::string onlyBounds_to_string() const;

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

    // Class for storing a min and max value for a given variable
    // The varBounds structure also defines an ordering
    // which allows to sort bins in increasing or decreasing order basically
    // Finally an overlap function allows to check if two varBounds have some overlap
    class varBounds {
      private: 
        float m_lowerBound = std::numeric_limits<float>::lowest();
        float m_upperBound = std::numeric_limits<float>::max();
        // Epsilon value to compare lowerBound and upperBound values
        float m_varEps = 1.e-6f;

      public:
        // Default constructor
        varBounds() = default;
        // Real constructor
        varBounds(const float lowerBound, const float upperBound, const float varEps = 1.e-6f){
          m_lowerBound = lowerBound; 
          m_upperBound = upperBound;
          m_varEps = varEps;

          // Make sure that lowerBound and upperBound are not NaNs or infinite float 
          // In this way the minimal and maximal values that lowerBound and upperBound can take 
          // are std::numeric_limits<float>::lowest() and std::numeric_limits<float>::max()
          if (!std::isfinite(m_lowerBound)){
            throw std::runtime_error("varBounds requires a finite lower bound but got lowerBound=" + std::to_string(m_lowerBound));
          }
          if (!std::isfinite(m_upperBound)){
            throw std::runtime_error("varBounds requires a finite upper bound but got upperBound=" + std::to_string(m_upperBound));
          }

          // Make sure that lowerBound < upperBound 
          // !(a < b) also catches NaN 
          if (!(m_lowerBound < m_upperBound)) {
            throw std::runtime_error( "varBounds requires lowerBound < upperBound but got lowerBound=" + std::to_string(m_lowerBound) + " and upperBound=" + std::to_string(m_upperBound));
          }
          // Make sure varEps is positive 
          if (!(m_varEps >= 0.f) || !std::isfinite(m_varEps)) {
            throw std::runtime_error("varBounds requires a finite varEps and varEps >=0 but got varEps=" + std::to_string(m_varEps));
          }
        }
        // Copy construction
        varBounds(const varBounds&) = default;
        varBounds& operator=(const varBounds&) = default;

        float getLowerBound() const { return m_lowerBound; }
        float getUpperBound() const { return m_upperBound; }
        float getVarEpsilon() const { return m_varEps; }

        // Two half-open intervals [low, high) overlap if each one starts before the other ends.
        // Touching edges (upper = other lower) are not considered overlapping.
        bool overlaps(const varBounds& o) const;

        // Define an ordering of varBounds compare first lowerBound then upperBound
        // within tolerance
        bool operator<(const varBounds& oBounds) const;
        bool operator>(const varBounds& oBounds) const { return oBounds < *this; }
        // Define also equal and not equal operator within tolerance
        bool operator==(const varBounds& oBounds) const;
        bool operator!=(const varBounds& oBounds) const { return !(*this == oBounds); }
        
        // Comparison with a value
        // varBounds < value if the interval ends at or below the value
        // value < varBounds if the value is below the start of the interval
        // Hence the value is "equivalent" to the varBounds if lowerBound <= value < upperBound
        // This allows to look up a value in a std::map<varBounds, ..., std::less<>> using find(value)
        // provided the varBounds stored in the map are disjoint
        // Exact comparisons are used to be consistent with isJetWithinBounds
        // Special case: if upperBound = std::numeric_limits<float>::max() the interval is closed
        // on the upper side i.e. [lowerBound, max] so that value == max is considered within bounds
        bool operator<(const float value) const {
          // Any NaN is ordered after every interval 
          // so that NaN values are never considered within bounds
          if (std::isnan(value)) { return true; }

          // If the upper bound is the maximal value a float can take 
          // then value is still considered within the bounds if equal to that max value 
          if (m_upperBound == std::numeric_limits<float>::max()){
            return m_upperBound < value;
          }
          // For regular cases the value is greater than varBounds
          // if it is at least equal or greater to the upper bound of varBounds
          return m_upperBound <= value;
        }
        friend bool operator<(const float value, const varBounds& vBounds) {
          // The value is lower than the varBounds if value is strictly lower than the lower bound of varBounds
          // if value == lowerBound then value is NOT lower than the varBounds
          return value < vBounds.m_lowerBound; 
        }
        // Greater than operators defined from the operators above 
        // varBounds > value if the value is below the start of the interval
        // value > varBounds if the interval ends at or below the value
        bool operator>(const float value) const { return value < *this; }
        friend bool operator>(const float value, const varBounds& vBounds) { return vBounds < value; }

        bool isWithinBounds(const float value) const { 
          // Use the operators defined above to ensure that 
          // lowerBound <= value < upperBound 
          // or lowerBound <= value <= upperBound if upperBound = std::numeric_limits<float>::max())
          // Neither value is lower or greater than the bounds hence it falls 
          // in the range [lowerBound, upperBound)
          // or [lowerBound, upperBound] if upperBound = std::numeric_limits<float>::max()
          return (!(*this < value) && !(value < *this)); 
        }
    };

    protected:
      // Pointer to the function used to retrieve the jet values to compare to the bounds
      JetQuantityGetter *m_funcGetJetQuantity = nullptr;

      // Map that will contain the variable type and the bounds 
      std::map<BTaggingToolUtil::varType, varBounds> m_varBinBounds;

      // Ordering of handlers loop over variables as stored in the map
      // and let the first variable whose bounds differ decide of the ordering
      bool operator<(const BTaggingBoundsHandler& oBounds) const;
      bool operator>(const BTaggingBoundsHandler& oBounds) const { return oBounds < *this; }
      // Define also equal and not equal operator
      // if neither is less than the other then handlers are equal
      bool operator==(const BTaggingBoundsHandler& oBounds) const { return !(*this < oBounds) && !(*this > oBounds); }
      bool operator!=(const BTaggingBoundsHandler& oBounds) const { return !(*this == oBounds); }
};

#endif // BTAGGINGBOUNDSHANDLER_H
