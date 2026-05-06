/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#include "InDetRawData/PhaseIIPixelRawDataContainerMT.h"
#include "../src/PhaseIIPixelRawDataContainerCnv.h"
#include "PhaseIIInDetRawDataContainerCnv_common_test.h"
#include <cstdint>
#include <array>
#include <vector>
struct PixelRDOMaker : HelperBase<PixelID> {
   using ContainerType = PhaseIIPixelRawDataContainerMT;
   static std::array<std::int16_t,2> makeCoordinates(unsigned int module_i, unsigned int hit_i) {
      std::array<std::int16_t,2> coords{ static_cast<std::int16_t>(module_i & 0xffff),
                                                    static_cast<std::int16_t>(hit_i    & 0xffff)};
      return coords;
   }
   static std::uint32_t makeDataWord(unsigned int module_i, unsigned int hit_i) {
      unsigned int data_word =  ((static_cast<std::uint32_t>(module_i & 0xffff) << 16)
                                 | static_cast<std::uint32_t>(hit_i    & 0xffff));
      return data_word;
   }
   static std::string idName() { return std::string("PixelID"); }
};
int main() {
   ISvcLocator*svcloc = init();

   PixelRDOMaker pixel_helper;
   {
      SmartIF<StoreGateSvc> detstore{svcloc->service ("DetectorStore")};
      auto id_helper = pixel_helper.makeIdHelper ();
      CHECK( detstore->record (std::move(id_helper), pixel_helper.idName()) );
   }

   return testRoundTrip<PhaseIIPixelRawDataContainerMT, PhaseIIPixelRawDataContainer, PhaseIIPixelRawDataContainerCnv>
      (svcloc, pixel_helper, std::vector<unsigned int>{10u,20u,30u,40u,50u,60u});
}
