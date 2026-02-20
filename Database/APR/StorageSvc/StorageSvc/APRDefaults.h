// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#ifndef APRDEFAULTS_H
#define APRDEFAULTS_H

#include <array>
#include <atomic>
#include <optional>
#include <string_view>

namespace APRDefaults {

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
         static std::atomic<NamingScheme> s_scheme{NamingScheme::Canonical};
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

   // Get all available naming schemes
   constexpr std::array<NamingScheme, 2> getAllNamingSchemes() {
      return {NamingScheme::Canonical, NamingScheme::Historical};
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

   static constexpr const char* IndexColName = "index_ref";
   static constexpr const char* DataHeaderTypeName = "DataHeader";
   static constexpr const char* DataHeaderFormTypeName = "DataHeaderForm";
   static constexpr const char* EventTagTypeName = "AttributeList";

}

#endif