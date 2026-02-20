/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef PHASEII_PIXELRAWDATACONTAINER_H
#define PHASEII_PIXELRAWDATACONTAINER_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthLinks/DeclareIndexingPolicy.h"
#include "PhaseIIInDetRawDataContainer.h"

#include <vector>
#include <cstdint>

namespace PhaseII {

   /// @brief The container for pixel RDOs
   /// The base class is the InDetRawDataContainer with a coordinate dimension of 2.
   class PixelRawDataContainer : public InDetRawDataContainer<2> {
   protected:
      // helper constatnts to unpack the bit packed information from the data word.
      // To extract the data the bits are shifted first and then the mask is applied
      static constexpr std::uint32_t MASK_TOT = 0xFF;
      static constexpr unsigned int SHIFT_TOT = 0;
      static constexpr std::uint32_t MASK_BCID = 0xFF;
      static constexpr unsigned int SHIFT_BCID = 8;
      static constexpr std::uint32_t MASK_LVL1D = 0xFF;
      static constexpr unsigned int SHIFT_LVL1D = 16;
      static constexpr std::uint32_t MASK_LVL1A = 0xF;
      static constexpr unsigned int SHIFT_LVL1A = 24;

   private:
      // an addtional error container which will be filled by the bytestream converter.
      // It will container first one value per module for module specific errors, then
      // one value per module times max. number of front-ends for front-end specific
      // errors.
      // @TODO Currently just follows the same design of  IDCInDetBSErrContainer, but it should get an interface.
      std::vector<std::uint64_t>  m_errors;

   public:
      // convenience methods to interpret the data word.
      static int getToT(std::uint32_t dataWord)     { return unpack(MASK_TOT,   SHIFT_TOT,   dataWord); }
      static int getBCID(std::uint32_t dataWord)    { return unpack(MASK_BCID,  SHIFT_BCID,  dataWord); }
      static int getLVL1A(std::uint32_t dataWord)   { return unpack(MASK_LVL1A, SHIFT_LVL1A, dataWord); }
      static int getLVL1ID(std::uint32_t dataWord)  { return unpack(MASK_LVL1D, SHIFT_LVL1D, dataWord); }

      // create the bit packed data word.
      static std::uint32_t makeWord( int tot, int bcid, int lvl1a, int lvl1d) {
         return   pack(MASK_TOT,   SHIFT_TOT,   tot)
                | pack(MASK_BCID,  SHIFT_BCID,  bcid)
                | pack(MASK_LVL1A, SHIFT_LVL1A, lvl1a)
                | pack(MASK_LVL1D, SHIFT_LVL1D, lvl1d);
      }

      // get the associated error container which can be empty or has one element per module + module * max. number of front-ends/per-module.
      const std::vector<std::uint64_t> &errors() const { return m_errors; }
      // get the associated error container which can be empty or should have one element per module + module * max. number of front-ends/per-module (read/write).
      std::vector<std::uint64_t> &errors()             { return m_errors; }
   };

   /// @brief A proxy for a pixel RDO which adds convenience methods to interpret the data word to the base proxy.
   template <AccessPolicy accessPolicy=AccessPolicy::ReadOnly>
   class PixelRawDataProxy  : public RawDataProxyBase<PhaseII::PixelRawDataContainer, accessPolicy >
   {
   public:
      using BASE = RawDataProxyBase<PhaseII::PixelRawDataContainer, accessPolicy >;
      using BASE::BASE;

      int getToT()    const { return PixelRawDataContainer::getToT(this->dataWord()); }
      int getBCID()   const { return PixelRawDataContainer::getBCID(this->dataWord()); }
      int getLVL1A()  const { return PixelRawDataContainer::getLVL1A(this->dataWord()); }
      int getLVL1ID() const { return PixelRawDataContainer::getLVL1ID(this->dataWord()); }
   };

   // provide means to RawDataCollectionProxies to discover the RDO proxy type to be used for a certain i.e. pixel RDO container.
   namespace RawData {
      namespace details {
         template <>
         struct traits<PixelRawDataContainer>  {
            template <AccessPolicy accessPolicy>
            using RawDataProxy = PixelRawDataProxy<accessPolicy>;
         };
      }
   }

   // Define all the proxies for read only access of the pixel raw data
   template <AccessPolicy accessPolicy=AccessPolicy::ReadOnly>
   using PixelRawDataContainerCollectionTypes = RawDataCollectionTypes<PixelRawDataContainer, accessPolicy>;
}

// Pool converter do not like namespaces
using PhaseIIPixelRawDataContainer = PhaseII::PixelRawDataContainerCollectionTypes<>::ContainerCollection;

CLASS_DEF(PhaseIIPixelRawDataContainer, 1261995829, 1)
#endif
