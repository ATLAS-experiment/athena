// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#ifndef APRDEFAULTS_H
#define APRDEFAULTS_H

#include "CxxUtils/checker_macros.h"

#include <atomic>
#include <map>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>

namespace APRDefaults {

   // The write configuration namespace encapsulates all logic that is related to the naming scheme of containers in the output file
   namespace WriteConfig {
      // Container naming scheme
      enum class NamingScheme {
         Historical,  // Traditional TTree-based names (CollectionTree, POOLCollectionTree, etc.)
         Canonical    // Modern canonical names (EventData, EventTag, etc.)
      };

      // Name categories
      enum class NameType {
         EventData,
         EventTag,
         DataHeader,
         MetaData
      };

      // Technology-specific name sets
      namespace ContainerNames {
         struct Historical {
            static constexpr const char* EventData  = "CollectionTree";
            static constexpr const char* EventTag   = "POOLCollectionTree";
            static constexpr const char* DataHeader = "POOLContainer";
            static constexpr const char* MetaData   = "MetaData";
         };

         struct Canonical {
            static constexpr const char* EventData  = "EventData";
            static constexpr const char* EventTag   = "EventTag";
            static constexpr const char* DataHeader = "DataHeader";
            static constexpr const char* MetaData   = "MetaData";
         };
      }

      namespace detail {
         // Atomic global naming scheme - shared across all threads
         inline std::atomic<NamingScheme>& getGlobalScheme() {
            static std::atomic<NamingScheme> s_scheme{NamingScheme::Historical};
            return s_scheme;
         }

         template<NameType NT>
         constexpr const char* getNameImpl(NamingScheme scheme) {
            switch (scheme) {
               case NamingScheme::Canonical:
                  if constexpr (NT == NameType::EventData)  return ContainerNames::Canonical::EventData;
                  if constexpr (NT == NameType::EventTag)   return ContainerNames::Canonical::EventTag;
                  if constexpr (NT == NameType::DataHeader) return ContainerNames::Canonical::DataHeader;
                  if constexpr (NT == NameType::MetaData)   return ContainerNames::Canonical::MetaData;
                  break;
               case NamingScheme::Historical:
                  if constexpr (NT == NameType::EventData)  return ContainerNames::Historical::EventData;
                  if constexpr (NT == NameType::EventTag)   return ContainerNames::Historical::EventTag;
                  if constexpr (NT == NameType::DataHeader) return ContainerNames::Historical::DataHeader;
                  if constexpr (NT == NameType::MetaData)   return ContainerNames::Historical::MetaData;
                  break;
            }
            return "";
         }
      }

      // Set the global naming scheme (thread-safe)
      inline void setNamingScheme(NamingScheme scheme) {
         detail::getGlobalScheme().store(scheme, std::memory_order_relaxed);
      }

      // Get the current global naming scheme (thread-safe)
      inline NamingScheme getNamingScheme() {
         return detail::getGlobalScheme().load(std::memory_order_relaxed);
      }

      // Convenience method to parse strings
      inline std::optional<NamingScheme> parseNamingScheme(std::string_view name) {
         if (name == "Historical") return NamingScheme::Historical;
         if (name == "Canonical") return NamingScheme::Canonical;
         return std::nullopt;
      }

      // Public API with explicit scheme parameter
      constexpr const char* getName(NamingScheme scheme, NameType type) {
         switch (type) {
            case NameType::EventData:  return detail::getNameImpl<NameType::EventData>(scheme);
            case NameType::EventTag:   return detail::getNameImpl<NameType::EventTag>(scheme);
            case NameType::DataHeader: return detail::getNameImpl<NameType::DataHeader>(scheme);
            case NameType::MetaData:   return detail::getNameImpl<NameType::MetaData>(scheme);
         }
         return "";
      }

      // Convenience functions using global scheme
      inline const char* getEventDataName() {
         return getName(getNamingScheme(), NameType::EventData);
      }

      inline const char* getEventTagName() {
         return getName(getNamingScheme(), NameType::EventTag);
      }

      inline const char* getDataHeaderName() {
         return getName(getNamingScheme(), NameType::DataHeader);
      }

      inline const char* getMetaDataName() {
         return getName(getNamingScheme(), NameType::MetaData);
      }

      // Overloads with explicit scheme
      constexpr const char* getEventDataName(NamingScheme scheme) {
         return getName(scheme, NameType::EventData);
      }

      constexpr const char* getEventTagName(NamingScheme scheme) {
         return getName(scheme, NameType::EventTag);
      }

      constexpr const char* getDataHeaderName(NamingScheme scheme) {
         return getName(scheme, NameType::DataHeader);
      }

      constexpr const char* getMetaDataName(NamingScheme scheme) {
         return getName(scheme, NameType::MetaData);
      }
   } // namespace WriteConfig

   // The read configuration namespace encapsulates all logic that is related to the naming scheme of containers in the input file
   namespace ReadConfig {
      // Container names default to legacy values in read mode for backwards compatibility
      struct ContainerNames {
         std::string EventTag = "POOLCollectionTree";
         std::string DataHeader = "POOLContainer";
      };

      namespace detail {
         inline std::shared_mutex& getMutex() {
            static std::shared_mutex s_mutex;
            return s_mutex;
         }

         inline const ContainerNames& getDefaultNames() {
            static const ContainerNames s_defaultNames{};
            return s_defaultNames;
         }

         inline std::map<std::string, ContainerNames, std::less<>>& getNamesByDatabase() {
            static std::map<std::string, ContainerNames, std::less<>> s_namesByDatabase ATLAS_THREAD_SAFE;
            return s_namesByDatabase;
         }
      }

      // Setters (exclusive write lock)
      inline void setEventTagName(std::string_view databaseName, std::string_view name) {
         std::unique_lock lock{detail::getMutex()};
         detail::getNamesByDatabase().try_emplace(std::string{databaseName}).first->second.EventTag = name;
      }

      inline void setDataHeaderName(std::string_view databaseName, std::string_view name) {
         std::unique_lock lock{detail::getMutex()};
         detail::getNamesByDatabase().try_emplace(std::string{databaseName}).first->second.DataHeader = name;
      }

      // Removers (exclusive write lock)
      inline void clearDatabase(std::string_view databaseName) {
         std::unique_lock lock{detail::getMutex()};
         auto& m = detail::getNamesByDatabase();
         const auto it = m.find(databaseName);
         if (it != m.end()) m.erase(it);
      }

      inline void clearAll() {
         std::unique_lock lock{detail::getMutex()};
         detail::getNamesByDatabase().clear();
      }

      // Getters (shared read lock)
      inline std::string getEventTagName(std::string_view databaseName) {
         std::shared_lock lock{detail::getMutex()};
         const auto& namesByDatabase = detail::getNamesByDatabase();
         const auto it = namesByDatabase.find(databaseName);
         return (it != namesByDatabase.end()) ? it->second.EventTag : detail::getDefaultNames().EventTag;
      }

      inline std::string getDataHeaderName(std::string_view databaseName) {
         std::shared_lock lock{detail::getMutex()};
         const auto& namesByDatabase = detail::getNamesByDatabase();
         const auto it = namesByDatabase.find(databaseName);
         return (it != namesByDatabase.end()) ? it->second.DataHeader : detail::getDefaultNames().DataHeader;
      }
   } // namespace ReadConfig

   static constexpr const char* IndexColName = "index_ref";
   static constexpr const char* DataHeaderTypeName = "DataHeader";
   static constexpr const char* DataHeaderFormTypeName = "DataHeaderForm";
   static constexpr const char* EventTagTypeName = "AttributeList";
   static constexpr const char* ParamsKeyEventTag = "POOL_CONTAINERNAME_EVENTTAG";
   static constexpr const char* ParamsKeyDataHeader = "POOL_CONTAINERNAME_DATAHEADER";

}

#endif