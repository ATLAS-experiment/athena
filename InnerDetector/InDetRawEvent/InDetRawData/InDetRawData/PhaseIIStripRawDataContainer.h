/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef PHASEII_STRIPRAWDATACONTAINER_H
#define PHASEII_STRIPRAWDATACONTAINER_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthLinks/DeclareIndexingPolicy.h"
#include "PhaseIIInDetRawDataContainer.h"

#include <cassert>
#include <cstdint>

namespace PhaseII {

   /// @brief The container for strip RDOs
   /// The base class is the InDetRawDataContainer with a coordinate dimension of 1.
   class StripRawDataContainer  : public InDetRawDataContainer<1> {
   protected:
      // helper constants to unpack the bit packed information from the data word.
      // To extract the data the bits are shifted first and then the mask is applied
      static constexpr std::uint32_t MASK_GROUPSIZE = 0x7FF;
      static constexpr unsigned int SHIFT_GROUPSIZE = 0;
      static constexpr std::uint32_t MASK_TIMEBIN = 0x7;
      static constexpr unsigned int SHIFT_TIMEBIN = 22;
      static constexpr std::uint32_t MASK_ERRORS = 0x7;
      static constexpr unsigned int SHIFT_ERRORS = 25;
      // alternative interpretation for the upper most bits
      static constexpr std::uint32_t MASK_ONTIME = 0x1;
      static constexpr unsigned int SHIFT_ONTIME = 23;
      static constexpr std::uint32_t MASK_FIRSTHITERROR = 0x1;
      static constexpr unsigned int SHIFT_FIRSTHITERROR = 29;
      static constexpr std::uint32_t MASK_SECONDHITERROR = 0x1;
      static constexpr unsigned int SHIFT_SECONDHITERROR = 30;
   public:
      /// @brief Extract the number of consecutive strip hits starting from the coordinates of the corresponding RDO from the data word.
      static int getGroupSize(std::uint32_t dataWord)      { return unpack(MASK_GROUPSIZE,      SHIFT_GROUPSIZE,      dataWord); }
      /// @brief Extract the time bin from the data word.
      static int getTimeBin(std::uint32_t dataWord)        { return unpack(MASK_TIMEBIN,        SHIFT_TIMEBIN,        dataWord); }
      /// @brief Extract error information from the data word.
      static int getErrors(std::uint32_t dataWord)         { return unpack(MASK_ERRORS,         SHIFT_ERRORS,         dataWord); }
      // alternatives
      // @TODO not full clear when the alternative interpretation of the upper-bits is to be used and when not.
      static bool OnTime(std::uint32_t dataWord)           { return unpack(MASK_ONTIME,         SHIFT_ONTIME,         dataWord); };
      static bool FirstHitError(std::uint32_t dataWord)    { return unpack(MASK_FIRSTHITERROR,  SHIFT_FIRSTHITERROR,  dataWord); };
      static bool SecondHitError(std::uint32_t dataWord)   { return unpack(MASK_SECONDHITERROR, SHIFT_SECONDHITERROR, dataWord); };

      /// @brief Create a bit-packed data word.
      static std::uint32_t makeWord(unsigned int group_size, unsigned int time_bin, unsigned int errors ) {
         return   pack(MASK_GROUPSIZE, SHIFT_GROUPSIZE, group_size)
                | pack(MASK_TIMEBIN,   SHIFT_TIMEBIN,   time_bin)
                | pack(MASK_ERRORS,    SHIFT_ERRORS,    errors);
      }

      /// @brief The data format of the RAW data.
      // @TODO for which data periods was/is SCT1/SCT3 being used?
      enum ERawDataType { SCT1 = 1, SCT3 =3, UNKNOWN=0};
      /// @brief Return the type of the original RAW data the RDO container is based on.
      ERawDataType dataType() const { return m_dataType; }
      /// @brief Set the RAW data type of this container.
      void setDataType(ERawDataType type) { assert(m_dataType == UNKNOWN);  m_dataType=type; }
   protected:
      ERawDataType m_dataType=UNKNOWN;
   };

   /// @brief A proxy for a strip RDO which adds convenience methods to interpret the data word to the base proxy.
   template <AccessPolicy accessPolicy=AccessPolicy::Const>
   class StripRawDataProxy
      : public RawDataProxyBase<typename Utils::ContainerAccessHelper<PhaseII::StripRawDataContainer, accessPolicy>::ContainerType >
   {
   public:
      using BASE = RawDataProxyBase<typename Utils::ContainerAccessHelper<PhaseII::StripRawDataContainer, accessPolicy>::ContainerType >;
      using BASE::BASE;

      using ReadOnlyProxy = StripRawDataProxy<AccessPolicy::Const>;
      int getGroupSize() const                      {return StripRawDataContainer::getGroupSize(this->dataWord());}
      int getTimeBin() const                        {return StripRawDataContainer::getTimeBin(this->dataWord());}
      int getErrors() const                         {return StripRawDataContainer::getErrors(this->dataWord());}
      // alterntives
      bool OnTime() const                           {return StripRawDataContainer::OnTime(this->dataWord());}
      bool FirstHitError() const                    {return StripRawDataContainer::FirstHitError(this->dataWord());}
      bool SecondHitError() const                   {return StripRawDataContainer::SecondHitError(this->dataWord());}
      StripRawDataContainer::ERawDataType dataType() const   {return this->container().dataType(); }

   };

   // provide means to RawDataCollectionProxies to discover the RDO proxy type to be used for a certain i.e. strip RDO container.
   namespace RawData {
      namespace details {
         template <>
         struct traits<StripRawDataContainer>  {
            template <AccessPolicy accessPolicy>
            using RawDataProxy = StripRawDataProxy<accessPolicy>;
         };
      }
   }

   // Define all the proxies for read only access of the strip raw data
   template <AccessPolicy accessPolicy=AccessPolicy::Const>
   using StripRawDataTypeTraits = RawDataTypeTraits<typename Utils::ContainerAccessHelper<StripRawDataContainer,
                                                                                                             accessPolicy>::ContainerType >;
}

// Pool converter do not like namespaces
using PhaseIIStripRawDataContainer = PhaseII::StripRawDataTypeTraits<PhaseII::AccessPolicy::Mutable>::ContainerCollection;

CLASS_DEF(PhaseIIStripRawDataContainer, 1329968921, 1)
#endif
