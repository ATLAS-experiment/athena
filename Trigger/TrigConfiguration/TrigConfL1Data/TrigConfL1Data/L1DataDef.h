/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigConf_L1DataDef
#define TrigConf_L1DataDef

#include <string>
#include <map>
#include <vector>

#include "CxxUtils/checker_macros.h"

namespace TrigConf {

   class L1DataDef {
   public:
      
      enum TriggerType {
         EM=1, TAU, MUON,                         // 1-3
         JET, FJET, FJ, JE, JF, JB,               // 4-9
         XE, TE, XS,                              // 10 - 12
         MBTS, MBTSSI, LUCID, TRT, BCM, ZDC, NIM, // 13 - 19
         ZB, TH, M, BCMCMB, CALREQ, BPTX,         // 20 - 25
         TOPO,                                    // 26
         ALFA,                                    // 27
         BGRP = 100, RNDM, PCLK,                  // 100 - 102
         UNDEF = 199                              // 199
      };
      
      struct TriggerTypeConfig {
         TriggerTypeConfig(TriggerType t=UNDEF, const std::string& n="UNDEF", unsigned int m=0, bool i=false) :
            type(t), name(n), max(m), internal(i) {};
         TriggerType   type; // the trigger type EM, BGRP, NIM
         std::string   name; // the name "EM", "BGRP", "NIM"
         unsigned int  max; // the maximum number of thresholds
         bool          internal; // true for internal triggers
      };

      typedef TriggerTypeConfig InternalTriggerType;

      typedef std::map<TriggerType, TriggerTypeConfig> TypeConfigMap_t;
      typedef std::map<std::string, TriggerType> STypeTypeMap_t;

   public:

      static void setMaxThresholdsFromL1Version(unsigned int l1version);

      static void printMaxThresholds();

      static TriggerType stringAsType(const std::string& type) { return g_sTypeType[type]; }

      static std::string& typeAsString(TriggerType tt) { return typeConfig(tt).name; }

      static const std::vector<TriggerType>& types() { return g_Types; }

      static const TypeConfigMap_t& typeConfigs() { return g_typeConfigs; }
      static TriggerTypeConfig& typeConfig(TriggerType tt);

      static bool addConfig(TriggerType tt, const std::string& name, unsigned int max, bool internal=false) {
         g_typeConfigs.insert(TypeConfigMap_t::value_type(tt, TriggerTypeConfig(tt, name, max, internal)));
         g_sTypeType.insert(STypeTypeMap_t::value_type(name,tt));
         g_Types.push_back(tt);
         return true;
      }

      static void setOldJEP3Cabling();
      static void setNewJEP3Cabling();

   private:
      // thread-safe because set once during initialize
      inline static TypeConfigMap_t g_typeConfigs ATLAS_THREAD_SAFE;     // maps TriggerType (e.g. L1DataDef::EM) to the TriggerTypeConfig object
      inline static STypeTypeMap_t g_sTypeType ATLAS_THREAD_SAFE;        // maps string to TriggerType (e.g. "EM" -> L1DataDef::EM)
      inline static std::vector<TriggerType> g_Types ATLAS_THREAD_SAFE;
      inline static unsigned int g_l1Version ATLAS_THREAD_SAFE {0};

   };

}

#endif
