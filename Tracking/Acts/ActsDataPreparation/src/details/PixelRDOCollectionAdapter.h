/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_PIXELRDOCOLLECTIONADAPTER_H
#define ACTSTRK_PIXELRDOCOLLECTIONADAPTER_H

#include "InDetRawData/PhaseIIPixelRawDataContainer.h"
#include <InDetRawData/PixelRDO_Container.h>
#include <InDetRawData/PixelRDORawData.h>
#include "PixelReadoutGeometry/PixelModuleDesign.h"

#include "RDOCollectionAdapter.h"

#include <cassert>
#include <utility>

namespace ActsTrk {

   // RDO adapter for the PhaseIIPixelRawDataContainer
   template <>
   class RDOCollectionAdapter<PhaseIIPixelRawDataContainer> {
      PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy m_RDOs;
   public:
      RDOCollectionAdapter(PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy &&RDOs)
         : m_RDOs(std::move(RDOs))
      {}
      // test whether this object can be dereferenced to return the representation of a single module.
      // @note always true for this container
      static constexpr bool isValid()  { return true ; }
      operator bool() const { return isValid();}

      // dereferencing will return a representation of one module.
      const PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy  &operator*() const {
         return m_RDOs;
      }
      // return a pointer to the representation of one module.
      const PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy  *operator->() const {
         return &m_RDOs;
      }
      // check whether the RDO collection represented by this object is empty.
      bool empty() const {
         return m_RDOs.empty();
      }
      // result represents the module of the given id_hash when dereferenced.
      // @note must only be dereferenced if isValid is true.
      static std::optional<RDOCollectionAdapter> make(const PhaseIIPixelRawDataContainer &rdo_container,const IdentifierHash &id_hash) {
         auto rdo_container_proxy = PhaseII::makeRawDataCollectionProxy(rdo_container);
         if (id_hash < rdo_container_proxy.size()) { return RDOCollectionAdapter(rdo_container_proxy[id_hash]); }
         else { return std::optional<RDOCollectionAdapter>{}; }
      }

      // result represents an iterable range where each element represents one module
      static PhaseII::RawDataTypeTraits<const PhaseII::PixelRawDataContainer>::ContainerCollectionProxy
      range(const PhaseIIPixelRawDataContainer &rdo_container) {
         return PhaseII::makeRawDataCollectionProxy(rdo_container);
      }

   };


   // helper class to adapt different RDOs to have the same interface
   template <typename T_RDOContainer> class RDOAdapter;

   // RDO adapter for the PixelRDO_Container.
   // The underlying elements are of type PixelRDORawData.
   template <>
   class RDOAdapter<PixelRDO_Container> {
      const PixelRDORawData *m_rdo;
   public:
      RDOAdapter(const PixelRDORawData *rdo) : m_rdo(rdo) {}

      // compute the pixel identifier from the cell coordinates.
      template <typename T_CellProxy>
      Identifier computeIdentifier(const PixelID &pixelID, const Identifier &module_id, const T_CellProxy &cell_proxy) {
         return pixelID.pixel_id(module_id, cell_proxy.coordinates()[0], cell_proxy.coordinates()[1]);
      }

      // get the level1 accept information.
      int getLVL1A() const {
         return m_rdo->getLVL1A();
      }
      // get the time-over-threshold which is a measure of the collected charge.
      int getToT() const {
         return m_rdo->getToT();
      }
      // get the pixel coordinates
      // @return return the pixel coordinates in a format that can be passed to the PixelModuleDesign.
      std::array<InDetDD::PixelDiodeTree::CellIndexType,2> coordinates(const PixelID &pixelID) const {
         const Identifier& rdoId = m_rdo->identify();
         return InDetDD::PixelDiodeTree::makeCellIndex(pixelID.phi_index(rdoId),
                                                       pixelID.eta_index(rdoId));
      }
      // test whether the pixel is a "ganged" pixel.
      // Where ganged means that multiple pixels of the sensor are connected to the same readout channel.
      // Thus ganged pixels get the same coordinates assigned by the hardware.
      bool isGanged(const InDetDD::PixelModuleDesign& design, const PixelID &pixelID) const {
         const Identifier& rdoId = m_rdo->identify();
         InDetDD::SiCellId cellId(pixelID.phi_index(rdoId),
                                  pixelID.eta_index(rdoId));
         InDetDD::SiReadoutCellId readoutId = design.readoutIdOfCell(cellId);
         return  ( design.numberOfConnectedCells( readoutId ) > 1 );
      }
   };


   // RDO adapter for the PhaseIIPixelRawDataContainer.
   // The underlying elements are proxy objects representing a single RDO
   template <>
   class RDOAdapter<PhaseIIPixelRawDataContainer> {
      PhaseII::PixelRawDataTypeTraits<>::RawDataProxy m_rdoProxy;
   public:
      using index_t = typename PhaseII::PixelRawDataTypeTraits<>::RawDataProxy::index_t;
      RDOAdapter(PhaseII::PixelRawDataTypeTraits<>::RawDataProxy &&rdo_proxy)
         : m_rdoProxy(std::move(rdo_proxy))
      {}
      // compute the pixel identifier from the cell coordinates.
      template <typename T_CellProxy>
      Identifier computeIdentifier(const PixelID &pixelID, const Identifier &module_id, const T_CellProxy &) {
         return pixelID.pixel_id(module_id, m_rdoProxy.coordinates()[0], m_rdoProxy.coordinates()[1]);
      }
      // get the level1 accept information.
      int getLVL1A() const {
         return m_rdoProxy.getLVL1A();
      }
      // get the time-over-threshold which is a measure of the collected charge.
      int getToT() const {
         return m_rdoProxy.getToT();
      }
      // Return the  "index" which identifies the element this proxy refers to
      index_t index() const {
         return m_rdoProxy.index();
      }
      // get the pixel coordinates
      // @return return the pixel coordinates in a format that can be passed to the PixelModuleDesign.
      const std::array<std::int16_t,2> &coordinates(const PixelID &) const {
         return m_rdoProxy.coordinates();
      }
      // test whether the pixel is a "ganged" pixel.
      // Where ganged means that multiple pixels of the sensor are connected to the same readout channel.
      // Thus ganged pixels get the same coordinates assigned by the hardware.
      bool isGanged(const InDetDD::PixelModuleDesign& design, const PixelID &) const {
         assert( std::in_range<int>(m_rdoProxy.coordinates()[0]) );
         assert( std::in_range<int>(m_rdoProxy.coordinates()[1]) );
         InDetDD::SiCellId cellId(m_rdoProxy.coordinates()[0],
                                  m_rdoProxy.coordinates()[1]);
         InDetDD::SiReadoutCellId readoutId = design.readoutIdOfCell(cellId);
         return  ( design.numberOfConnectedCells( readoutId ) > 1 );
      }
   };
}
#endif
