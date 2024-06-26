/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef StationSelector_H
#define StationSelector_H

#include "MuonGeoModel/Station.h"

#include <atomic>
#include <map>
#include <string>
#include <vector>

namespace MuonGM {

    class StationSelector {
      public:
        using StationMap = std::map<std::string, Station*, std::less<std::string>>;
        using StationIterator = StationMap::const_iterator ;
        StationSelector(const MYSQL& mysql, const std::string& filename);
        StationSelector(const MYSQL& mysql, std::vector<std::string> s);
        StationIterator begin();
        StationIterator end();
        static void SetSelectionType(int t);

      private:
        std::vector<std::string> m_selector{};
        StationMap m_theMap{};
        bool select(const std::string& name);
        static std::atomic<int> m_selectType;
    };

} // namespace MuonGM

#endif
