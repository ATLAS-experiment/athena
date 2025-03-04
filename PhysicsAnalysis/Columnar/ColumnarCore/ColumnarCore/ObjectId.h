/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_OBJECT_ID_H
#define COLUMNAR_CORE_OBJECT_ID_H

#include <ColumnarCore/ContainerId.h>
#include <iostream>
#include <stdexcept>

namespace columnar
{
  /// @brief a class representing a single object (electron, muons, etc.)
  template<ContainerId O, typename CM = ColumnarModeDefault> class ObjectId;





  template<ContainerId O> class ObjectId<O,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    static_assert (ContainerIdTraits<O>::isDefined, "ContainerId not defined, include the appropriate header");

    using xAODObject = typename ContainerIdTraits<O>::xAODObjectIdType;

    ObjectId (xAODObject& val_object) noexcept
      : m_object (&val_object)
    {}

    ObjectId (const ObjectId<O,ColumnarModeXAOD>& that) noexcept = default;

    template<ContainerId CI2> requires (ContainerIdTraits<CI2>::isMutable && ContainerIdTraits<CI2>::constId == O)
    ObjectId (const ObjectId<CI2,ColumnarModeXAOD>& that) noexcept
      : m_object (&that.getXAODObject())
    {}

    ObjectId& operator = (const ObjectId<O,ColumnarModeXAOD>& that) noexcept = default;

    [[nodiscard]] xAODObject& getXAODObject () const noexcept {
      return *m_object;}

    template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectId<O,ColumnarModeXAOD>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}



    /// Private Members
    /// ===============
  private:

    xAODObject *m_object = nullptr;
  };

  template<ContainerId CI>
  bool operator== (const ObjectId<CI,ColumnarModeXAOD>& lhs, const ObjectId<CI,ColumnarModeXAOD>& rhs)
  {
    return &lhs.getXAODObject() == &rhs.getXAODObject();
  }

  template<ContainerId CI>
  bool operator!= (const ObjectId<CI,ColumnarModeXAOD>& lhs, const ObjectId<CI,ColumnarModeXAOD>& rhs)
  {
    return &lhs.getXAODObject() != &rhs.getXAODObject();
  }




  template<ContainerId O> class ObjectId<O,ColumnarModeArray> final
  {
    /// Common Public Members
    /// =====================
  public:

    static_assert (ContainerIdTraits<O>::isDefined, "ContainerId not defined, include the appropriate header");

    using CM = ColumnarModeArray;
    using xAODObject = typename ContainerIdTraits<O>::xAODObjectIdType;

    ObjectId (xAODObject& /*val_object*/)
    {
      throw std::logic_error ("can't call xAOD function in columnar mode");
    }

    ObjectId (const ObjectId<O,ColumnarModeArray>& that) noexcept = default;

    template<ContainerId CI2> requires (ContainerIdTraits<CI2>::isMutable && ContainerIdTraits<CI2>::constId == O)
    ObjectId (const ObjectId<CI2,ColumnarModeArray>& that) noexcept
      : m_data (that.getData()), m_index (that.getIndex())
    {}

    ObjectId& operator = (const ObjectId<O,ColumnarModeArray>& that) noexcept = default;

    [[nodiscard]] xAODObject& getXAODObject () const {
      throw std::logic_error ("can't call xAOD function in columnar mode");}

    template<typename Acc,typename... Args>
      requires std::invocable<Acc,ObjectId<O,ColumnarModeArray>,Args...>
    [[nodiscard]] decltype(auto) operator() (Acc& acc, Args&&... args) const {
      return acc (*this, std::forward<Args> (args)...);}



    /// Mode-Specific Public Members
    /// ============================
  public:

    explicit ObjectId (void **val_data, std::size_t val_index) noexcept
      : m_data (val_data), m_index (val_index)
    {}

    [[nodiscard]] std::size_t getIndex () const noexcept {
      return m_index;}

    [[nodiscard]] void **getData () const noexcept {
      return m_data;}



    /// Private Members
    /// ===============
  private:

    void **m_data = nullptr;
    std::size_t m_index = 0u;
  };

  template<ContainerId CI>
  bool operator== (const ObjectId<CI,ColumnarModeArray>& lhs, const ObjectId<CI,ColumnarModeArray>& rhs)
  {
    return lhs.getIndex() == rhs.getIndex();
  }

  template<ContainerId CI>
  bool operator!= (const ObjectId<CI,ColumnarModeArray>& lhs, const ObjectId<CI,ColumnarModeArray>& rhs)
  {
    return lhs.getIndex() != rhs.getIndex();
  }




  using JetId = ObjectId<ContainerId::jet>;
  using MutableJetId = ObjectId<ContainerId::mutableJet>;
  using MuonId = ObjectId<ContainerId::muon>;
  using EventInfoId = ObjectId<ContainerId::eventInfo>;
  using EventContextId = ObjectId<ContainerId::eventContext>;
  using ElectronId = ObjectId<ContainerId::electron>;
  using PhotonId = ObjectId<ContainerId::photon>;
  using EgammaId = ObjectId<ContainerId::egamma>;
  using ClusterId = ObjectId<ContainerId::cluster>;
  using TrackId = ObjectId<ContainerId::track>;
  using VertexId = ObjectId<ContainerId::vertex>;
  using ParticleId = ObjectId<ContainerId::particle>;
  using Particle0Id = ObjectId<ContainerId::particle0>;
  using Particle1Id = ObjectId<ContainerId::particle1>;
  using MetId = ObjectId<ContainerId::met>;
  using Met0Id = ObjectId<ContainerId::met0>;
  using Met1Id = ObjectId<ContainerId::met1>;
  using MutableMetId = ObjectId<ContainerId::mutableMet>;
  using MetAssociationId = ObjectId<ContainerId::metAssociation>;
}

#endif
