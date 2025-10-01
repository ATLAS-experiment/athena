/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalLArCellContainer.h"
#include <ranges>
#include <algorithm>

namespace GlobalSim {

  // Main constructor setting up FEB2 LUTs
  GlobalLArCellContainer::GlobalLArCellContainer(const std::map<std::string,Feb2MuxInfo>& febMap) {

      std::map<std::string, std::vector<std::pair<int,std::string>>> tempMap;

      // Loop over map of FEB2s to establish LUTs and lists
      for (const auto & [feb2, muxInfo] : febMap) {
          m_feb2Flags.insert({feb2, {false, false}});
          m_feb2Keys.insert(feb2);
          m_muxKeys.insert(muxInfo.muxName);
          m_muxFlags.insert({muxInfo.muxName, {false, false}});
          m_feb2ToMux.insert({feb2, muxInfo.muxName});

          tempMap[muxInfo.muxName].emplace_back(muxInfo.indexOnMux, feb2);
      }

      // Sort FEB2 vectors for each MUX
      for (auto& [muxName, vec] : tempMap) {
          std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
              return a.first < b.first; // sort by indexOnMux
          });

          // Copy only the FEB2 names into m_muxToFeb2Ordered
          std::vector<std::string> orderedFeb2s;
          orderedFeb2s.reserve(vec.size());
          for (const auto& [idx, feb2] : vec) {
              orderedFeb2s.push_back(feb2);
          }
          m_muxToFeb2Ordered[muxName] = std::move(orderedFeb2s);
      }
  }


  // Copy constructor
  GlobalLArCellContainer::GlobalLArCellContainer(const GlobalLArCellContainer& other)
    : DataVector<GlobalLArCell>() 
  {
      // Deep-copy DataVector elements
      for (const auto cellPtr : other) {
          DataVector<GlobalLArCell>::push_back(std::make_unique<GlobalLArCell>(*cellPtr));
      }

      // Copy the maps and sets
      m_feb2Flags = other.m_feb2Flags;
      m_feb2Keys = other.m_feb2Keys;
      m_muxKeys = other.m_muxKeys;
      m_muxFlags = other.m_muxFlags;
      m_maxCellsPerFeb2 = other.m_maxCellsPerFeb2;
      m_muxToFeb2Ordered = other.m_muxToFeb2Ordered;
      m_feb2ToMux = other.m_feb2ToMux;
  }


  // Reimplementation of the push_back function to fill GlobalLArCells
  void GlobalLArCellContainer::push_back(const GlobalLArCell& theCell) {
    auto cell = std::make_unique<GlobalLArCell>(theCell);
    auto* cellPtr = cell.get();
    DataVector<GlobalLArCell>::push_back(std::move(cell));

    m_feb2ToCells[cellPtr->getFEB2()].push_back(cellPtr);
  }


  // Function to get all GlobalLArCells for a given FEB2 name
  const std::vector<GlobalLArCell*>& GlobalLArCellContainer::getCellsForFeb2(const std::string& feb2) const {

      static const std::vector<GlobalLArCell*> emptyCellVector; // fallback if not found
      auto it = m_feb2ToCells.find(feb2);

      if (it != m_feb2ToCells.end()) {
          return it->second;
      }

      return emptyCellVector;
  }


  // Function to get ordered list of FEB2s for a given MUX name
  const std::vector<std::string>& GlobalLArCellContainer::getOrderedFeb2sForMux(const std::string& mux) const {

      static const std::vector<std::string> emptyMux;
      auto it = m_muxToFeb2Ordered.find(mux);
      if (it != m_muxToFeb2Ordered.end()) {
          return it->second;
      }

      return emptyMux;
  }


  // Function to get the associated MUX name for a given FEB2
  const std::string& GlobalLArCellContainer::getMuxForFeb2(const std::string& feb2Key) const {
      auto it = m_feb2ToMux.find(feb2Key);
      if (it == m_feb2ToMux.end()) {
          throw std::runtime_error("Unknown FEB2 key: " + feb2Key);
      }
      return it->second;
  }


  // Function to set overflow and error flag for a FEB2
  void GlobalLArCellContainer::setFeb2Flags(const std::string& feb2Key, bool overflow, bool error) {
      auto& flags = m_feb2Flags[feb2Key];
      flags.overflow = overflow;
      flags.error = error;

      // Propagate FEB2 flags to the MUX
      if (overflow) {
          auto& muxFlags = m_muxFlags[m_feb2ToMux[feb2Key]];
          muxFlags.overflow = overflow;
      }
      if (error) {
          auto& muxFlags = m_muxFlags[m_feb2ToMux[feb2Key]];
          muxFlags.error = error;
      }
  }

 
  // Function to check whether a given FEB2 is in overflow 
  bool GlobalLArCellContainer::feb2InOverflow(const std::string& feb2Key) const {
      auto it = m_feb2Flags.find(feb2Key);
      return (it != m_feb2Flags.end()) ? it->second.overflow : false;
  }


  // Function to check if a given FEB2 is in error
  bool GlobalLArCellContainer::feb2InError(const std::string& feb2Key) const {
      auto it = m_feb2Flags.find(feb2Key);
      return (it != m_feb2Flags.end()) ? it->second.error : false;
  }

 
  // Function to check whether a given MUX is in overflow 
  bool GlobalLArCellContainer::muxInOverflow(const std::string& muxKey) const {
      auto it = m_muxFlags.find(muxKey);
      return (it != m_muxFlags.end()) ? it->second.overflow : false;
  }


  // Function to check if a given MUX is in error
  bool GlobalLArCellContainer::muxInError(const std::string& muxKey) const {
      auto it = m_muxFlags.find(muxKey);
      return (it != m_muxFlags.end()) ? it->second.error : false;
  }

} // namespace GlobalSim
