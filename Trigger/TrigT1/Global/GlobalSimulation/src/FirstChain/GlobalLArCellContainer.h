/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
 
#ifndef GLOBALSIM_GLOBALLARCELLCONTAINER_H
#define GLOBALSIM_GLOBALLARCELLCONTAINER_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "GlobalLArCell.h"

#include <map>

namespace GlobalSim {

  /** @brief Helper struct to keep track of correct order of FEB2 for each MUX */
  struct Feb2MuxInfo {
      std::string muxName;
      int indexOnMux{};
  };

  class GlobalLArCellContainer : public DataVector<GlobalSim::GlobalLArCell> {

  public:

    using DataVector<GlobalLArCell>::DataVector;

    /** @brief Default constructor */
    GlobalLArCellContainer() = default;

    /** @brief Helper struct to hold overflow and error flags */
    struct StatusFlags {
        bool overflow{false};
        bool error{false};
    };

    /** @brief Main constructor setting up FEB2 LUTs */
    GlobalLArCellContainer(const std::map<std::string, Feb2MuxInfo>& febMap);

    /** @brief Copy constructor */
    GlobalLArCellContainer(const GlobalLArCellContainer& other);

    /** @brief Reimplementation of the push_back function to fill LArCells */
    void push_back(const GlobalLArCell& theCell);

    /** @brief Function to return the full list of FEB2 names */
    const std::unordered_set<std::string>& getFeb2Keys() const { return m_feb2Keys; };

    /** @brief Function to return the full list of MUX names */
    const std::unordered_set<std::string>& getMuxKeys() const { return m_muxKeys; };

    /** @brief Function to get all GlobalLArCells for a given FEB2 name */
    const std::vector<GlobalLArCell*>& getCellsForFeb2(const std::string& feb2) const;

    /** @brief Function to get ordered list of FEB2s for a given MUX name */
    const std::vector<std::string>& getOrderedFeb2sForMux(const std::string& mux) const;

    /** @brief Function to set maximum number of cells per FEB2 */
    void setMaxCellsPerFeb2 (int n) { m_maxCellsPerFeb2 = n; }

    /** @brief Set overflow and error flag for a FEB2 */
    void setFeb2Flags(const std::string& feb2Key, bool overflow, bool error);

    /** @brief Function to get maximum number of cells per FEB2 */
    std::size_t getMaxCellsPerFeb2() const { return m_maxCellsPerFeb2; }

    /** @brief Function to get the associated MUX name for a given FEB2 */
    const std::string& getMuxForFeb2(const std::string& feb2Key) const;

    /** @brief Check if a given FEB2 is in overflow */
    bool feb2InOverflow(const std::string& feb2Key) const;

    /** @brief Check if a given FEB2 is in error */
    bool feb2InError(const std::string& feb2Key) const;

    /** @brief Check if a given MUX is in overflow */
    bool muxInOverflow(const std::string& muxKey) const;

    /** @brief Check if a given MUX is in error */
    bool muxInError(const std::string& muxKey) const;

  private:

    /** @brief map which stores overflow and error bits for each FEB2 */
    std::map<std::string, StatusFlags> m_feb2Flags;
    /** @brief map which stores overflow and error bits for each MUX */
    std::map<std::string, StatusFlags> m_muxFlags;
    /** @brief vector of all FEB2 names */
    std::unordered_set<std::string> m_feb2Keys;
    /** @brief vector of all MUX names */
    std::unordered_set<std::string> m_muxKeys;
    /** @brief map which keeps track of which cells are associated with which FEB2 */
    std::map<std::string, std::vector<GlobalLArCell*>> m_feb2ToCells;
    /** @brief map which holds the ordered list of FEB2s for each MUX */
    std::map<std::string, std::vector<std::string>> m_muxToFeb2Ordered;
    /** @brief map which acts as lookuptable to get the associated MUX from a FEB2 name */
    std::map<std::string, std::string> m_feb2ToMux;
    /** @brief Maximum number of cells per FEB2 in given energy encoding scheme */
    std::size_t m_maxCellsPerFeb2 = 0;

  };

} //namespace GlobalSim

CLASS_DEF(GlobalSim::GlobalLArCellContainer, 1117669828, 1)
SG_BASE(GlobalSim::GlobalLArCellContainer, DataVector<GlobalSim::GlobalLArCell> );

#endif
