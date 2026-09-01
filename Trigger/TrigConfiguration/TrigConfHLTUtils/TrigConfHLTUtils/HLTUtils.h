/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigConfHLTUtils_HLTUtils
#define TrigConfHLTUtils_HLTUtils

#include <string>
#include <inttypes.h>

#include "CxxUtils/checker_macros.h"
#include "CxxUtils/ConcurrentStrMap.h"
#include "CxxUtils/ConcurrentStrToValMap.h"
#include "CxxUtils/ConcurrentToValMap.h"
#include "CxxUtils/SimpleUpdater.h"

namespace TrigConf {

  typedef uint32_t HLTHash;

  /**
   * Two concurrent maps to store name->hash and hash->name mappings.
   *
   * We are using CxxUtils::Concurrent*Map which is optimized for frequent (lock-less)
   * reads and rare writes. Previous implementations used TBB's concurrent containers
   * but they are much slower for this use-case.
   */
  struct HashMap {
    ~HashMap();

    using Name2HashMap_t = CxxUtils::ConcurrentStrMap<HLTHash, CxxUtils::SimpleUpdater>;
    using Hash2NameMap_t = CxxUtils::ConcurrentToValMap<HLTHash, const std::string, CxxUtils::SimpleUpdater>;

    Name2HashMap_t name2hash{Name2HashMap_t::Updater_t()};  //!< name to hash map
    Hash2NameMap_t hash2name{Hash2NameMap_t::Updater_t()};  //!< hash to name map
  };

  /** Store for hash maps per category*/
  struct HashStore {
    ~HashStore();

    using HashMap_t = CxxUtils::ConcurrentStrToValMap<HashMap, CxxUtils::SimpleUpdater>;
    HashMap_t hashCat{HashMap_t::Updater_t()};  //!< one HashMap per category
  };


  class HLTUtils {
  public:
    /**@brief hash function translating TE names into identifiers*/
    static HLTHash string2hash( const std::string&, const std::string& category=s_defaultCategory );
    /**@brief hash function translating identifiers into names (via internal dictionary)*/
    static const std::string hash2string( HLTHash, const std::string& category=s_defaultCategory );
    /**@brief debugging output of internal dictionary*/
    static void hashes2file( const std::string& fileName="hashes2string.txt" );
    /**@brief debugging output of internal dictionary*/
    static void file2hashes( const std::string& fileName="hashes2string.txt" );

  private:
    /**@brief In-file identifier*/
    inline static const std::string s_newCategory{"##NewCategory"};
    /** Default category*/
    inline static const std::string s_defaultCategory{"TE"};
    /**@brief Nested concurrent hash-maps to store (key=hash, value=string) pairs for different hash categories*/
    inline static HashStore s_hashStore ATLAS_THREAD_SAFE{};
  };
 
}

#endif
