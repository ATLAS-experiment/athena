/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_MET_MET_ASSOCIATION_H
#define COLUMNAR_MET_MET_ASSOCIATION_H

#include <ColumnarCore/ContainerId.h>
#include <ColumnarEgamma/EgammaDef.h>
#include <ColumnarJet/JetDef.h>
#include <ColumnarMuon/MuonDef.h>
#include <ColumnarCore/VariantLinkColumn.h>
#include <ColumnarCore/VectorVectorColumn.h>
#include <ColumnarMet/MetInput.h>
#include <METUtilities/METHelpers.h>
#include <xAODBase/IParticleContainer.h>
#include <xAODMissingET/MissingETComposition.h>

namespace columnar
{
  /// a simple range adaptor that filters the elements of a range
  ///
  /// Once C++23 is available, this should be replaced with a range
  /// adaptor that does the same thing.  Same if we ever get a common
  /// implementation in CxxUtils.
  template<typename IteratorType,class FunctionType>
  class FilterRange;

  template<typename IteratorType,class FunctionType>
  class FilterRangeIterator
  {
    IteratorType m_iterator;
    const FilterRange<IteratorType,FunctionType> *m_range = nullptr;
  public:
    FilterRangeIterator (const IteratorType& val_iterator, const FilterRange<IteratorType,FunctionType> *val_range) noexcept
      : m_iterator (val_iterator), m_range (val_range)
    {}

    auto operator * () const noexcept {
      return *m_iterator;}

    FilterRangeIterator& operator ++ () noexcept {
      ++ m_iterator;
      while (m_range && m_iterator != m_range->m_end && !m_range->m_filter (*m_iterator))
        ++m_iterator;
      return *this;
    }

    bool operator == (const FilterRangeIterator& that) const noexcept {
      return m_iterator == that.m_iterator;}
    bool operator != (const FilterRangeIterator& that) const noexcept {
      return m_iterator != that.m_iterator;}
  };

  template<typename IteratorType,class FunctionType>
  class FilterRange final
  {
    friend class FilterRangeIterator<IteratorType,FunctionType>;
    IteratorType m_begin;
    IteratorType m_end;
    FunctionType m_filter;

  public:
    FilterRange (IteratorType begin, IteratorType end, FunctionType&& filter)
      : m_begin (begin), m_end (end), m_filter (std::move (filter))
    {
      while (m_begin != m_end && !m_filter (*m_begin))
        ++m_begin;
    }
    FilterRange (FilterRange&&) = default;
    FilterRange (const FilterRange&) = default;
    FilterRange& operator = (FilterRange&&) = default;
    FilterRange& operator = (const FilterRange&) = default;

    auto begin () const noexcept { return FilterRangeIterator<IteratorType,FunctionType>(m_begin,this); }
    auto end () const noexcept { return FilterRangeIterator<IteratorType,FunctionType>(m_end,this); }
    bool empty () const noexcept { return m_begin == m_end; }
  };



  template<typename CM = ColumnarModeDefault> class MetAssociationHelper;



  /// @brief the accessors @ref MetAssociationHelper needs to implement
  /// its operations
  template<typename CM = ColumnarModeDefault> class MetAssocationAccessors final
  {
    /// Public Members
    /// ==============
  public:

    using AssocId = ObjectId<ContainerId::metAssociation,CM>;
    using OptAssocId = OptObjectId<ContainerId::metAssociation,CM>;
    using PartId = ObjectId<ContainerId::particle,CM>;
    static constexpr bool useMissingETAssociationHelper = std::is_same_v<CM,ColumnarModeXAOD>;

    using constvec_t = MissingETBase::Types::constvec_t;
    using ConstVec = xAOD::MissingETAssociation::ConstVec;
    using bitmask_t = MissingETBase::Types::bitmask_t;

    static constexpr std::size_t invalidIndex = MissingETBase::Constants::invalidIndex;

    explicit MetAssocationAccessors (ColumnarTool<CM>& columnarBase)
      : calpx (columnarBase, "calpx"),
        calpy (columnarBase, "calpy"),
        calpz (columnarBase, "calpz"),
        cale (columnarBase, "cale"),
        calsumpt (columnarBase, "calsumpt"),
        calkeyAcc (columnarBase, "calkey"),
        trkpx (columnarBase, "trkpx"),
        trkpy (columnarBase, "trkpy"),
        trkpz (columnarBase, "trkpz"),
        trke (columnarBase, "trke"),
        trksumpt (columnarBase, "trksumpt"),
        trkkeyAcc (columnarBase, "trkkey"),
        jettrkpx (columnarBase, "jettrkpx"),
        jettrkpy (columnarBase, "jettrkpy"),
        jettrkpz (columnarBase, "jettrkpz"),
        jettrke (columnarBase, "jettrke"),
        jettrksumpt (columnarBase, "jettrksumpt"),
        objectLinks (columnarBase, "objectLinks"),
        jetLink (columnarBase, "jetLink"),
        isMisc (columnarBase, "isMisc"),
        overlapIndicesAcc (columnarBase, "overlapIndices"),
        overlapTypesAcc (columnarBase, "overlapTypes"),
        objectTypeAcc (columnarBase, "objectType")
    {
      if constexpr (!useMissingETAssociationHelper)
        useObjectFlagsAcc = ColumnUpdater<ContainerId::metAssociation,bitmask_t,CM> (columnarBase, "useObjectFlags");
    }

    MetAssociationHelper<CM> operator () (xAOD::MissingETAssociationHelper& helper) const;
    MetAssociationHelper<CM> operator () (ObjectRange<ContainerId::metAssociation,CM> map) const;

    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> calpx;
    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> calpy;
    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> calpz;
    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> cale;
    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> calsumpt;
    ColumnAccessor<ContainerId::metAssociation,std::vector<bitmask_t>,CM> calkeyAcc;

    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> trkpx;
    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> trkpy;
    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> trkpz;
    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> trke;
    ColumnAccessor<ContainerId::metAssociation,std::vector<float>,CM> trksumpt;
    ColumnAccessor<ContainerId::metAssociation,std::vector<bitmask_t>,CM> trkkeyAcc;

    ColumnAccessor<ContainerId::metAssociation,float,CM> jettrkpx;
    ColumnAccessor<ContainerId::metAssociation,float,CM> jettrkpy;
    ColumnAccessor<ContainerId::metAssociation,float,CM> jettrkpz;
    ColumnAccessor<ContainerId::metAssociation,float,CM> jettrke;
    ColumnAccessor<ContainerId::metAssociation,float,CM> jettrksumpt;

    using ObjectLinkCI = VariantContainerId<ContainerId::particle,ContainerId::jet,ContainerId::electron,ContainerId::photon,ContainerId::muon>;
    using ObjectLinkType = VariantObjectLink<ObjectLinkCI,CM>;
    ColumnAccessor<ContainerId::metAssociation,std::vector<ObjectLinkType>,CM> objectLinks;

    ColumnAccessor<ContainerId::metAssociation,LinkCastColumn<ContainerId::jet,xAOD::JetContainer>,CM> jetLink;

    ColumnAccessor<ContainerId::metAssociation,char,CM> isMisc;

    ColumnAccessor<ContainerId::metAssociation,std::vector<std::vector<std::size_t>>,CM> overlapIndicesAcc;
    ColumnAccessor<ContainerId::metAssociation,std::vector<std::vector<unsigned char>>,CM> overlapTypesAcc;

    MetHelpers::ObjectTypeAccessor<ContainerId::particle> objectTypeAcc;

    ColumnUpdater<ContainerId::metAssociation,bitmask_t,CM> useObjectFlagsAcc;



    /// Public Members from MissingETAssociation
    /// ========================================

    [[nodiscard]] static bitmask_t getObjMask(size_t objIndex) { 
      if(objIndex==invalidIndex) return 0;
      return 1<<objIndex;
    }

    [[nodiscard]] std::size_t sizeCal(AssocId assoc) const {
      return cale(assoc).size();}

    [[nodiscard]] decltype(auto) calkey(AssocId assoc) const {
      return calkeyAcc(assoc);}

    [[nodiscard]] bitmask_t calkey (AssocId assoc, size_t keyIdx) const {
      return keyIdx != invalidIndex && keyIdx < calkeyAcc(assoc).size() ? calkeyAcc(assoc)[keyIdx] : invalidIndex; }

    [[nodiscard]] ConstVec calVec(AssocId assoc, const xAOD::IParticle* pPart) const {
      return calVec(assoc, PartId(*pPart)); }
    template<ContainerId CI>
    [[nodiscard]] ConstVec calVec(AssocId assoc, ObjectId<CI,CM> pPart) const {
      constvec_t totalvec;
      for (size_t iKey = 0; iKey < this->sizeCal(assoc); iKey++) {
        if (this->calkey(assoc, iKey) & getObjMask(this->findIndex(assoc,pPart)))
          totalvec+=this->calVec(assoc, iKey);
      }
      return totalvec;
    }

    [[nodiscard]] ConstVec calVec(AssocId assoc, std::size_t keyIdx) const {
      return keyIdx != invalidIndex && keyIdx < this->sizeCal(assoc)
      ? constvec_t(calpx(assoc)[keyIdx],calpy(assoc)[keyIdx],calpz(assoc)[keyIdx],cale(assoc)[keyIdx],calsumpt(assoc)[keyIdx])
      : constvec_t(MissingETBase::Numerical::cpxError(),MissingETBase::Numerical::cpyError(),MissingETBase::Numerical::cpzError(),MissingETBase::Numerical::ceError(),MissingETBase::Numerical::sumptError()); }


    [[nodiscard]] std::size_t sizeTrk (AssocId assoc) const {
      return trke(assoc).size();}

    [[nodiscard]] decltype(auto) trkkey(AssocId assoc) const {
      return trkkeyAcc(assoc);}

    [[nodiscard]] bitmask_t trkkey(AssocId assoc, size_t keyIdx) const {
      return keyIdx != invalidIndex && keyIdx < this->sizeTrk(assoc) ? this->trkkey(assoc)[keyIdx] : invalidIndex; }

    [[nodiscard]] ConstVec trkVec(AssocId assoc, const xAOD::IParticle* pPart) const {
      return trkVec(assoc, PartId(*pPart)); }
    template<ContainerId CI>
    [[nodiscard]] ConstVec trkVec(AssocId assoc, ObjectId<CI,CM> pPart) const {
      constvec_t totalvec;
      for (size_t iKey = 0; iKey < this->sizeTrk(assoc); iKey++) {
        if (this->trkkey(assoc,iKey) & getObjMask(this->findIndex(assoc,pPart)))
          totalvec+=this->trkVec(assoc,iKey);
      }
      return totalvec; }
    [[nodiscard]] ConstVec trkVec(AssocId assoc, ObjectLinkType pPart) const {
      constvec_t totalvec;
      for (size_t iKey = 0; iKey < this->sizeTrk(assoc); iKey++) {
        if (this->trkkey(assoc,iKey) & getObjMask(this->findIndex(assoc,pPart)))
          totalvec+=this->trkVec(assoc,iKey);
      }
      return totalvec; }

    [[nodiscard]] ConstVec trkVec(AssocId assoc, size_t keyIdx) const {
      return keyIdx != invalidIndex && keyIdx < this->sizeTrk(assoc) 
        ? constvec_t(this->trkpx(assoc)[keyIdx],this->trkpy(assoc)[keyIdx],this->trkpz(assoc)[keyIdx],this->trke(assoc)[keyIdx],this->trksumpt(assoc)[keyIdx])
        : constvec_t(MissingETBase::Numerical::cpxError(),MissingETBase::Numerical::cpyError(),MissingETBase::Numerical::cpzError(),MissingETBase::Numerical::ceError(),MissingETBase::Numerical::sumptError()); }

    [[nodiscard]] ConstVec jetTrkVec(AssocId assoc) const {
      return constvec_t(this->jettrkpx(assoc),this->jettrkpy(assoc),this->jettrkpz(assoc),this->jettrke(assoc),this->jettrksumpt(assoc)); }

    [[nodiscard]] size_t size(AssocId assoc) const {
      return this->objectLinks(assoc).size(); }

    [[nodiscard]] auto objects(AssocId assoc) const {
      return this->objectLinks(assoc);}

    [[nodiscard]] auto overlapIndices(AssocId assoc) const {
      return overlapIndicesAcc(assoc); }

    [[nodiscard]] auto overlapIndices(AssocId assoc,const xAOD::IParticle* pPart) const {
      return this->overlapIndices(assoc, ParticleId(*pPart)); }
    [[nodiscard]] auto overlapIndices(AssocId assoc,PartId pPart) const {
      return this->overlapIndices(assoc, this->findIndex(assoc, pPart)); }

    [[nodiscard]] std::span<const std::size_t> overlapIndices(AssocId assoc,std::size_t objIdx) const {
      if (objIdx == invalidIndex) return std::span<const std::size_t>();
      auto indices = overlapIndicesAcc(assoc);
      return objIdx < indices.size() ? indices[objIdx] : std::span<const std::size_t>(); }

    [[nodiscard]] std::span<const unsigned char> overlapTypes (AssocId assoc, std::size_t objIdx) const {
      if (objIdx == invalidIndex) return std::span<const unsigned char>();
      auto types = this->overlapTypesAcc(assoc);
      return objIdx < types.size() ? types[objIdx] : std::span<const unsigned char>(); }

    [[nodiscard]] std::size_t findIndex(AssocId assoc,const xAOD::IParticle* pPart) const {
      return findIndex(assoc, ParticleId(*pPart));}
    template<ContainerId CI>
    [[nodiscard]] std::size_t findIndex(AssocId assoc,ObjectId<CI,CM> pPart) const {
      std::size_t idx = 0;
      for (auto link : this->objectLinks(assoc)) {
        if (link == pPart)
          return idx;
        ++idx;
      }
      return invalidIndex;
    }
    [[nodiscard]] std::size_t findIndex(AssocId assoc,ObjectLinkType pPart) const {
      std::size_t idx = 0;
      for (auto link : this->objectLinks(assoc)) {
        if (link == pPart)
          return idx;
        ++idx;
      }
      return invalidIndex;
    }

    [[nodiscard]] bool hasAlternateConstVec(AssocId assoc) const {
      if (isMisc(assoc)) return false;
      auto refJet = this->refJet(assoc);
      if (!refJet) return false;
      return containsPhysics(assoc,refJet.value());}

    [[nodiscard]] xAOD::JetFourMom_t getAlternateConstVec(AssocId assoc) const {
      xAOD::JetFourMom_t newvec;
      constvec_t newpt;
      auto refJet = this->refJet(assoc);
      size_t newptIndex = refJet ? findIndex(assoc,refJet.value()) : invalidIndex;
      if (newptIndex == invalidIndex) return newvec;
      bitmask_t mask = (bitmask_t)1 << newptIndex;
      
      for (size_t iKey = 0; iKey < sizeCal(assoc); iKey++) if (mask & calkey(assoc)[iKey]) newpt += calVec(assoc,iKey);
      newvec.SetPxPyPzE(newpt.cpx(),newpt.cpy(),newpt.cpz(),newpt.ce());
      return newvec; 
    }

    [[nodiscard]] auto refJet(AssocId assoc) const {
      return this->jetLink(assoc); }

    [[nodiscard]] bool containsPhysics(AssocId assoc,JetId pPhys) const {
      for(auto link : this->objectLinks(assoc)) {
        if(link == pPhys) {
          return true;
        }
      }
      return false;}


    [[nodiscard]] OptAssocId getMiscAssociation(MetAssociationRange map) const {
      // I don't know if it matters, but in the original xAOD code the
      // iteration is done in reverse order, so I'm doing the same here
      // as well.
      for (auto assocIter = map.rbegin(); assocIter != map.rend(); ++assocIter)
      {
        if (isMisc(*assocIter))
          return *assocIter;
      }
      // If we reach here, no misc association was found.
      return OptAssocId{};}
  };



  /// @brief a columnar version of @ref xAOD::MissingETAssociationHelper
  template<typename CM> class MetAssociationHelper final
  {
    /// Public Members
    /// ==============
  public:

    using PartId = ObjectId<ContainerId::particle,CM>;
    using JetId = ObjectId<ContainerId::jet,CM>;
    using AssocId = ObjectId<ContainerId::metAssociation,CM>;
    using OptAssocId = OptObjectId<ContainerId::metAssociation,CM>;
    using ObjectLinkType = MetAssocationAccessors<CM>::ObjectLinkType;

    using constvec_t = MissingETBase::Types::constvec_t;
    using ConstVec = xAOD::MissingETAssociation::ConstVec;
    using bitmask_t = MissingETBase::Types::bitmask_t;

    static constexpr bool useMissingETAssociationHelper = std::is_same_v<CM,ColumnarModeXAOD>;
    static constexpr std::size_t invalidIndex = MissingETBase::Constants::invalidIndex;


    MetAssociationHelper (xAOD::MissingETAssociationHelper& val_helper, const columnar::MetAssocationAccessors<CM>& val_accessors)
      : m_xaodHelper (&val_helper), m_map (*val_helper.map()), m_accessors (&val_accessors)
    {}

    MetAssociationHelper (ObjectRange<ContainerId::metAssociation,CM> map, const columnar::MetAssocationAccessors<CM>& val_accessors)
      : m_map (map), m_accessors (&val_accessors)
    {}


    [[nodiscard]] xAOD::MissingETAssociationHelper& getXAODObject () const {
      if constexpr (useMissingETAssociationHelper)
        return *m_xaodHelper;
      else
        throw std::logic_error ("can't call xAOD function in columnar mode");}


    /// @brief the underlying association map
    [[nodiscard]] ObjectRange<ContainerId::metAssociation,CM> map () const noexcept {
      return m_map;}


    /// Object Selection Flag Members
    /// =============================

    [[nodiscard]] bool objSelected (const xAOD::IParticle* obj) const {
      return objSelected(PartId(*obj));}
    template<ContainerId CI>
    [[nodiscard]] bool objSelected (ObjectId<CI,CM> obj) const {
      for (decltype(auto) assoc : getAssociations(obj)) if(objSelected(assoc,obj)) return true;
      return false;}
    [[nodiscard]] bool objSelected (ObjectLinkType obj) const {
      for (decltype(auto) assoc : getAssociations(obj)) if(objSelected(assoc,obj)) return true;
      return false;}
    [[nodiscard]] bool objSelected(AssocId assocId, std::size_t objIdx) const {
      if constexpr (useMissingETAssociationHelper)
        return m_xaodHelper->objSelected(&assocId.getXAODObject(), objIdx);
      else {
        if (objIdx >= sizeof(bitmask_t)*8) return false;
        return bool(m_accessors->useObjectFlagsAcc(assocId) & (static_cast<MissingETBase::Types::bitmask_t>(1)<<objIdx));
      }
    }
    [[nodiscard]] bool objSelected(AssocId assocId, const xAOD::IParticle* pPart) const {
      return objSelected(assocId, PartId(*pPart));}
    template<ContainerId CI>
    [[nodiscard]] bool objSelected(AssocId assocId, ObjectId<CI,CM> pPart) const {
      return objSelected(assocId, m_accessors->findIndex(assocId,pPart));}
    [[nodiscard]] bool objSelected(AssocId assocId, ObjectLinkType pPart) const {
      return objSelected(assocId, m_accessors->findIndex(assocId,pPart));}


    [[nodiscard]] bitmask_t getObjSelectionFlags(AssocId assocId) const {
      if constexpr (useMissingETAssociationHelper)
        return m_xaodHelper->getObjSelectionFlags(&assocId.getXAODObject());
      else
        return m_accessors->useObjectFlagsAcc(assocId);
    }


    void setObjSelectionFlag(AssocId assocId, size_t objIdx, bool status) {
      if constexpr (useMissingETAssociationHelper)
        m_xaodHelper->setObjSelectionFlag(&assocId.getXAODObject(), objIdx, status);
      else
      {
        if (objIdx >= sizeof(bitmask_t)*8) return;
        bitmask_t mask = static_cast<bitmask_t>(1) << objIdx;
        if (status) m_accessors->useObjectFlagsAcc(assocId) |= mask;
        else m_accessors->useObjectFlagsAcc(assocId) &= ~mask;
      }
    }
    void setObjSelectionFlag(AssocId assocId, const xAOD::IParticle* pPart, bool status) {
      setObjSelectionFlag(assocId,PartId(*pPart),status);}
    template<ContainerId CI>
    void setObjSelectionFlag(AssocId assocId, ObjectId<CI,CM> pPart, bool status) {
      setObjSelectionFlag(assocId,m_accessors->findIndex(assocId,pPart),status);}
    void setObjSelectionFlag(AssocId assocId, const ObjectLinkType& pPart, bool status) {
      setObjSelectionFlag(assocId,m_accessors->findIndex(assocId,pPart),status);}



    /// Object Overlap Members
    /// ======================

    [[nodiscard]] bool hasOverlaps(AssocId assocId, size_t objIdx,MissingETBase::UsageHandler::Policy p) const {
      if ( objIdx == invalidIndex ) return false;
      auto indices = m_accessors->overlapIndices(assocId, objIdx);
      auto types = m_accessors->overlapTypes(assocId, objIdx);
      for(size_t iOL=0; iOL<indices.size(); ++iOL) {
        if(objSelected(assocId, indices[iOL])) {
          // printf("Test object %lu for overlaps: OL type %i\n",indices[iOL],(int)types[iOL]);
          switch(p) {
          case MissingETBase::UsageHandler::TrackCluster:      
            if((types[iOL] & 1<<xAOD::Type::CaloCluster) || (types[iOL] & 1<<xAOD::Type::TrackParticle)) {break;}
            else {continue;}
          case MissingETBase::UsageHandler::OnlyCluster:
            if(types[iOL] & 1<<xAOD::Type::CaloCluster) {break;}
            else {continue;}
          case MissingETBase::UsageHandler::OnlyTrack:
            if(types[iOL] & 1<<xAOD::Type::TrackParticle) {break;}
            else {continue;}
          case MissingETBase::UsageHandler::ParticleFlow:
            if(types[iOL] & 1<<xAOD::Type::ParticleFlow) {break;}
            else {continue;}
          case MissingETBase::UsageHandler::TruthParticle:
            if(types[iOL] & 1) {break;}
            else {continue;}
          case MissingETBase::UsageHandler::AllCalo:
            if(types[iOL] & ~(1<<xAOD::Type::TrackParticle)) {break;}
            else {continue;}
          default: continue;
          }
          return true;
        }
      }
      return false;
    }

    [[nodiscard]] bool selectIfNoOverlaps(const xAOD::IParticle* obj,MissingETBase::UsageHandler::Policy p) {
      return selectIfNoOverlaps(PartId(*obj), p);}
    [[nodiscard]] bool selectIfNoOverlaps(PartId obj,MissingETBase::UsageHandler::Policy p) {
      auto assocs = getAssociations(obj);
      bool overlaps(false);
      for(decltype(auto) assoc : assocs) overlaps |= hasOverlaps(assoc,m_accessors->findIndex(assoc,obj),p);
      if (overlaps) return false; 
      for(decltype(auto) assoc : assocs) setObjSelectionFlag(assoc,obj,true);
      return true;}



    /// Object Association Members
    /// ==========================

    [[nodiscard]] OptAssocId getJetAssociation(const xAOD::Jet* pJet) const {
      return getJetAssociation(JetId(*pJet));}
    [[nodiscard]] OptAssocId getJetAssociation(JetId pJet) const {
      // this uses a linear search, which is the same as the original
      // xAOD code, but was flagged there as potentially inefficient
      for (auto assoc : m_map) {
        if (m_accessors->jetLink(assoc) == pJet) {
          return assoc;
        }
      }
      return std::nullopt;}

    [[nodiscard]] auto getAssociations(const xAOD::IParticle* pPart) const {
      return getAssociations(PartId(*pPart));}
    template<ContainerId CI>
    [[nodiscard]] auto getAssociations(ObjectId<CI,CM> pPart) const {
      // In the original xAOD code, this returned a `std::vector` by
      // value.  In columnar code we prefer not to pass vectors by
      // value, so we return a filtered range instead.
      return FilterRange (m_map.begin(), m_map.end(),
        [accessors = m_accessors, pPart] (AssocId assoc) {
          return (accessors->findIndex(assoc,pPart) != invalidIndex);});}
    
    [[nodiscard]] auto getMiscAssociation() const {
      return m_accessors->getMiscAssociation(m_map);}


    /// Other Members
    /// =============

    [[nodiscard]] constvec_t getConstVec(const xAOD::IParticle* pPart,MissingETBase::UsageHandler::Policy p) const {
      return getConstVec(PartId(*pPart), p);}
    [[nodiscard]] constvec_t getConstVec(PartId pPart,MissingETBase::UsageHandler::Policy p) const {
      constvec_t totalvec;
      for (decltype(auto) assoc : getAssociations(pPart)) {
        if (p==MissingETBase::UsageHandler::OnlyTrack) {
          totalvec += m_accessors->trkVec(assoc, pPart);
        } else {
          totalvec += m_accessors->calVec(assoc, pPart);
        }
      }
      return totalvec;}
  
    [[nodiscard]] ConstVec overlapCalVec(AssocId assoc) const {
      constvec_t calvec;
      for (size_t iKey = 0; iKey < m_accessors->sizeCal(assoc); iKey++) {
        bool selector = (getObjSelectionFlags(assoc) & m_accessors->calkey(assoc)[iKey]) ? !m_accessors->isMisc(assoc) : m_accessors->isMisc(assoc);
        if (selector) calvec+=m_accessors->calVec(assoc,iKey);
      }
      return calvec;}

    [[nodiscard]] ConstVec overlapTrkVec(AssocId assoc) const {
        constvec_t trkvec;
        for (size_t iKey = 0; iKey < m_accessors->sizeTrk(assoc); iKey++) {
          bool selector = (getObjSelectionFlags(assoc) & m_accessors->trkkey(assoc)[iKey]) ? !m_accessors->isMisc(assoc) : m_accessors->isMisc(assoc);
          if (selector) trkvec+=ConstVec(m_accessors->trkpx(assoc)[iKey],m_accessors->trkpy(assoc)[iKey],m_accessors->trkpz(assoc)[iKey],m_accessors->trke(assoc)[iKey],m_accessors->trksumpt(assoc)[iKey]);
        }
        return trkvec;
      }



    /// Private Members
    /// ===============
  private:

    /// @brief the xAOD helper object, only used in xAOD mode, but its
    /// easier to define it in both modes regardless
    xAOD::MissingETAssociationHelper* m_xaodHelper = nullptr;

    /// @brief the underlying MET association map
    ObjectRange<ContainerId::metAssociation,CM> m_map;

    /// @brief the accessors for the members of the MET association map
    const MetAssocationAccessors<CM>* m_accessors = nullptr;
  };



  template<typename CM>
  MetAssociationHelper<CM> MetAssocationAccessors<CM> ::
  operator () (xAOD::MissingETAssociationHelper& helper) const
  {
    return MetAssociationHelper<CM> (helper, *this);
  }

  template<typename CM>
  MetAssociationHelper<CM> MetAssocationAccessors<CM> ::
  operator () (ObjectRange<ContainerId::metAssociation,CM> map) const
  {
    return MetAssociationHelper<CM> (map, *this);
  }
}

#endif
