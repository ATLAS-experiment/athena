/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_OPT_OBJECT_ID_H
#define COLUMNAR_CORE_OPT_OBJECT_ID_H

#include <ColumnarInterfaces/IColumnarTool.h>
#include <ColumnarCore/ObjectId.h>

namespace columnar
{
  /// @brief a class representing a single optional object (electron, muons, etc.)
  ///
  /// This essentially behaves like an `std::optional<ObjectId>`, and is
  /// used in cases in which a given object may or may not exist.  For
  /// xAOD only code this is typically handled by a pointer with
  /// `nullptr` taking the empty value.  This is its own type both for
  /// compactness and to allow a slightly more efficient representation
  /// internally.
  template<ContainerId CI, typename CM = ColumnarModeDefault> class OptObjectId;





  template<ContainerId CI> class OptObjectId<CI,ColumnarModeXAOD> final
  {
    /// Common Public Members
    /// =====================
  public:

    static_assert (ContainerIdTraits<CI>::isDefined, "ContainerId not defined, include the appropriate header");

    using xAODObject = typename ContainerIdTraits<CI>::xAODObjectIdType;

    OptObjectId () noexcept = default;

    OptObjectId (std::nullopt_t) noexcept {}

    OptObjectId (ObjectId<CI,ColumnarModeXAOD> val_object) noexcept
      : m_object (&val_object.getXAODObject())
    {}

    OptObjectId (xAODObject *val_object) noexcept
      : m_object (val_object)
    {}

    OptObjectId (const OptObjectId<CI,ColumnarModeXAOD>& that) noexcept = default;

    OptObjectId& operator = (const OptObjectId<CI,ColumnarModeXAOD>& that) noexcept = default;

    explicit operator bool () const noexcept {
      return m_object != nullptr;}

    [[nodiscard]] bool has_value () const noexcept {
      return m_object != nullptr;}

    [[nodiscard]] ObjectId<CI,ColumnarModeXAOD> value () const {
      if (m_object == nullptr)
        throw std::bad_optional_access();
      return ObjectId<CI,ColumnarModeXAOD> (*m_object);}

    [[nodiscard]] ObjectId<CI,ColumnarModeXAOD> operator * () const {
      if (m_object == nullptr)
        throw std::bad_optional_access();
      return ObjectId<CI,ColumnarModeXAOD> (*m_object);}

    [[nodiscard]] xAODObject *getXAODObject () const noexcept {
      return m_object;}

    [[nodiscard]] bool operator == (const OptObjectId<CI,ColumnarModeXAOD>& that) const noexcept {
      return m_object == that.m_object;}



    /// Private Members
    /// ===============
  private:

    xAODObject *m_object = nullptr;
  };

  template<ContainerId CI>
  bool operator== (const OptObjectId<CI,ColumnarModeXAOD>& lhs, const OptObjectId<CI,ColumnarModeXAOD>& rhs)
  {
    return lhs.getXAODObject() == rhs.getXAODObject();
  }

  template<ContainerId CI>
  bool operator!= (const OptObjectId<CI,ColumnarModeXAOD>& lhs, const OptObjectId<CI,ColumnarModeXAOD>& rhs)
  {
    return lhs.getXAODObject() != rhs.getXAODObject();
  }




  template<ContainerId CI> class OptObjectId<CI,ColumnarModeArray> final
  {
    /// Common Public Members
    /// =====================
  public:

    static_assert (ContainerIdTraits<CI>::isDefined, "ContainerId not defined, include the appropriate header");

    using xAODObject = typename ContainerIdTraits<CI>::xAODObjectIdType;

    OptObjectId () noexcept = default;

    OptObjectId (std::nullopt_t) noexcept {}

    OptObjectId (ObjectId<CI,ColumnarModeArray> val_object) noexcept
      : m_data (val_object.getData()), m_index (val_object.getIndex())
    {}

    OptObjectId (xAODObject * /*val_object*/)
    {
      throw std::logic_error ("can't call xAOD function in columnar mode");
    }

    OptObjectId (const OptObjectId<CI,ColumnarModeArray>& that) noexcept = default;

    OptObjectId& operator = (const OptObjectId<CI,ColumnarModeArray>& that) noexcept = default;

    [[nodiscard]] xAODObject *getXAODObject () const {
      throw std::logic_error ("can't call xAOD function in columnar mode");}

    explicit operator bool () const noexcept {
      return m_index != invalidObjectIndex;}

    [[nodiscard]] bool has_value () const noexcept {
      return m_index != invalidObjectIndex;}

    [[nodiscard]] ObjectId<CI,ColumnarModeArray> value () const {
      if (m_index == invalidObjectIndex)
        throw std::bad_optional_access();
      return ObjectId<CI,ColumnarModeArray> (m_data, m_index);}

    [[nodiscard]] ObjectId<CI,ColumnarModeArray> operator * () const {
      if (m_index == invalidObjectIndex)
        throw std::bad_optional_access();
      return ObjectId<CI,ColumnarModeArray> (m_data, m_index);}
  
    [[nodiscard]] bool operator == (const OptObjectId<CI,ColumnarModeArray>& that) const noexcept {
      return m_index == that.m_index;}



    /// Mode-Specific Public Members
    /// ============================
  public:

    explicit OptObjectId (void **val_data, int val_index) noexcept
      : m_data (val_data), m_index (val_index)
    {}

    explicit OptObjectId (void **val_data, unsigned val_index) noexcept
      : m_data (val_data), m_index (val_index)
    {}

    explicit OptObjectId (void **val_data, std::size_t val_index) noexcept
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
    std::size_t m_index = invalidObjectIndex;
  };

  template<ContainerId CI>
  bool operator== (const OptObjectId<CI,ColumnarModeArray>& lhs, const OptObjectId<CI,ColumnarModeArray>& rhs)
  {
    return lhs.getIndex() == rhs.getIndex();
  }

  template<ContainerId CI>
  bool operator!= (const OptObjectId<CI,ColumnarModeArray>& lhs, const OptObjectId<CI,ColumnarModeArray>& rhs)
  {
    return lhs.getIndex() != rhs.getIndex();
  }




  using OptJetId = OptObjectId<ContainerId::jet>;
  using OptMutableJetId = OptObjectId<ContainerId::mutableJet>;
  using OptMuonId = OptObjectId<ContainerId::muon>;
  using OptElectronId = OptObjectId<ContainerId::electron>;
  using OptPhotonId = OptObjectId<ContainerId::photon>;
  using OptEgammaId = OptObjectId<ContainerId::egamma>;
  using OptClusterId = OptObjectId<ContainerId::cluster>;
  using OptTrackId = OptObjectId<ContainerId::track>;
  using OptTrack0Id = OptObjectId<ContainerId::track0>;
  using OptTrack1Id = OptObjectId<ContainerId::track1>;
  using OptTrack2Id = OptObjectId<ContainerId::track2>;
  using OptVertexId = OptObjectId<ContainerId::vertex>;
  using OptParticleId = OptObjectId<ContainerId::particle>;
  using OptParticle0Id = OptObjectId<ContainerId::particle0>;
  using OptParticle1Id = OptObjectId<ContainerId::particle1>;
  using OptMetId = OptObjectId<ContainerId::met>;
  using OptMet0Id = OptObjectId<ContainerId::met0>;
  using OptMet1Id = OptObjectId<ContainerId::met1>;
  using OptMutableMetId = OptObjectId<ContainerId::mutableMet>;
  using OptMetAssociationId = OptObjectId<ContainerId::metAssociation>;
  using OptEventInfoId = OptObjectId<ContainerId::eventInfo>;
}

#endif
