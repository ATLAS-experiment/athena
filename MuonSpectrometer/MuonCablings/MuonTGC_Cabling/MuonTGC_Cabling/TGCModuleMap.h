/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCMODULEMAP_HH
#define MUONTGC_CABLING_TGCMODULEMAP_HH

#include <map>
#include <memory>

#include "MuonTGC_Cabling/TGCModuleId.h"

namespace MuonTGC_Cabling {

class TGCModuleMap {
   public:
    using Store_t = std::map<int, std::unique_ptr<TGCModuleId>>;
    // Constructor & Destructor
    TGCModuleMap() = default;
    /** @brief Move constructor */
    TGCModuleMap(TGCModuleMap&& other) = default;
    /** @brief Copy constructor */
    TGCModuleMap(const TGCModuleMap&) = delete;
    /** @brief Move assignment operator */
    TGCModuleMap& operator=(TGCModuleMap&& other) = default;
    /** @brief Copy assignment operator */
    TGCModuleMap& operator=(const TGCModuleMap& other) = delete;
    /** @brief Returns the begin iterator of the underlying map */
    Store_t::const_iterator begin() const;
    /** @brief Returns the end iterator of the underlying map */
    Store_t::const_iterator end() const;

    Store_t::iterator begin();

    Store_t::iterator end();

    virtual ~TGCModuleMap();
    /** @brief Return a certain module and remove it from the
     *         map
     * @param port: Connector port of the module */
    std::unique_ptr<TGCModuleId> popModule(const int connector);

    void insert(int connector, std::unique_ptr<TGCModuleId> moduleId);

    std::size_t size() const;

    bool empty() const;

    void clear();

   private:
    Store_t m_moduleMap;
};

}  // namespace MuonTGC_Cabling

#endif
