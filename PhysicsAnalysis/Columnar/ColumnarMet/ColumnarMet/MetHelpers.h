/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_MET_MET_HELPERS_H
#define COLUMNAR_MET_MET_HELPERS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/OptObjectId.h>
#include <ColumnarMet/MetDef.h>
#include <METUtilities/METHelpers.h>

namespace columnar
{
  namespace MetHelpers
  {
    /// @brief a special "accessor" that allows to do the MET term
    /// lookup by name
    ///
    /// These need to be handled differently between xAOD and array
    /// mode.  In xAOD mode this mechanism is directly build into the
    /// EDM objects.  In array mode it instead needs to rely on special
    /// columns the user has to provide for the met container.  This
    /// also includes some helper functions, that need to interact with
    /// the name mechanism in the EDM.
    ///
    /// One big difference between the two modes is that in xAOD mode
    /// the terms get created dynamically, while in array mode the user
    /// has to create all the terms ahead of time, as dynamically adding
    /// elements to a column adds a lot of complications that are better
    /// avoided.  This creates a slightly different behavior for
    /// `fillMET` in that in array mode it fails if the term does not
    /// exist.
    template<ContainerId CI = ContainerId::met,typename CM=ColumnarModeDefault>
    class MapLookupAccessor;

    template<ContainerId CI>
    class MapLookupAccessor<CI,ColumnarModeXAOD> final
    {
    public:
      using CM = ColumnarModeXAOD;

      MapLookupAccessor (ColumnarTool<CM>& /*columnBase*/) {}

      [[nodiscard]] OptObjectId<CI,CM> operator () (ObjectRange<CI,CM> metMap, const std::string& metKey) const
      {
        return metMap.getXAODObject()[metKey];
      }

      [[nodiscard]] ObjectId<CI,CM> getRequired (ObjectRange<CI,CM> metMap, const std::string& metKey) const
      {
        auto result = metMap.getXAODObject()[metKey];
        if (!result)
          throw std::runtime_error ("MET object does not have the requested term: " + metKey);
        return *result;
      }

      ObjectId<CI> fillMET(ObjectRange<CI,CM> metMap, const std::string& metKey, const MissingETBase::Types::bitmask_t metSource) const {
        xAOD::MissingET *metPtr = nullptr;
        if (met::fillMET(metPtr, &metMap.getXAODObject(), metKey, metSource).isFailure())
          throw std::runtime_error ("failed to fill MET term \"" + metKey + "\"");
        return ObjectId<CI> (*metPtr);}

      StatusCode tryCreateIfMissing (ObjectRange<CI,CM> metMap, const std::string& metKey, const MissingETBase::Types::bitmask_t metSource) const
      {
        auto iter = metMap.getXAODObject().find (metKey);
        if (iter != metMap.getXAODObject().end() && *iter != nullptr)
          return StatusCode::SUCCESS;
        xAOD::MissingET *metPtr = nullptr;
        if (met::fillMET(metPtr, &metMap.getXAODObject(), metKey, metSource).isFailure())
          return StatusCode::FAILURE;
        return StatusCode::SUCCESS;
      }
    };

    template<ContainerId CI>
    class MapLookupAccessor<CI,ColumnarModeArray> final
    {
    public:
      using CM = ColumnarModeArray;

    private:
      ColumnAccessor<CI,std::size_t,CM> m_nameHashAcc;

      // For output MET terms we have to pre-place all the MET terms in
      // the MET container, just so that we don't have to allocate them
      // dynamically.  To indicate whether a pre-placed MET term exists
      // the source bitmask is used and checked against m_nullSource.
      // This should be set to whatever an invalid source value is.
      // Technically we may not need this for input MET terms, but it
      // seems prudent to use it there as well for consistency.  Ideally
      // for output MET terms this should be an `update` mode, but I
      // don't have that defined yet.
      static constexpr MissingETBase::Types::bitmask_t m_nullSource = 0;
      AccessorTemplate<CI,MissingETBase::Types::bitmask_t,ContainerIdTraits<CI>::isMutable?ColumnAccessMode::output:ColumnAccessMode::input,CM> m_sourceAcc;

    public:
      MapLookupAccessor (ColumnarTool<CM>& columnBase) : m_nameHashAcc (columnBase, "nameHash"), m_sourceAcc (columnBase, "source") {}

      [[nodiscard]] OptObjectId<CI,CM> operator () (ObjectRange<CI,CM> metMap, const std::string& metKey) const
      {
        auto hash = std::hash<std::string>()(metKey);
        for (auto metObj : metMap)
        {
          if (m_nameHashAcc(metObj) == hash)
          {
            if (m_sourceAcc(metObj) != m_nullSource)
              return metObj;
            else
              return std::nullopt;
          }
        }
        return std::nullopt;
      }

      [[nodiscard]] ObjectId<CI,CM> getRequired (ObjectRange<CI,CM> metMap, const std::string& metKey) const
      {
        auto result = operator() (metMap, metKey);
        if (!result)
          throw std::runtime_error ("MET object does not have the requested term: " + metKey);
        return result.value();
      }

      ObjectId<CI> fillMET(ObjectRange<CI,CM> metMap, const std::string& metKey, const MissingETBase::Types::bitmask_t metSource) const {
        auto hash = std::hash<std::string>()(metKey);
        for (auto metObj : metMap)
        {
          if (m_nameHashAcc(metObj) == hash)
          {
            if (m_sourceAcc(metObj) != m_nullSource)
              throw std::runtime_error ("MET object already has the requested term: " + metKey);
            m_sourceAcc(metObj) = metSource;
            return metObj;
          }
        }
        throw std::runtime_error ("MET object does not have the requested term pre-placed: " + metKey);
      }

      StatusCode tryCreateIfMissing (ObjectRange<CI,CM> metMap, const std::string& metKey, const MissingETBase::Types::bitmask_t metSource) const
      {
        auto hash = std::hash<std::string>()(metKey);
        for (auto metObj : metMap)
        {
          if (m_nameHashAcc(metObj) == hash)
          {
            if (m_sourceAcc(metObj) == m_nullSource)
              m_sourceAcc(metObj) = metSource;
            return StatusCode::SUCCESS;
          }
        }
        throw std::runtime_error ("MET object does not have the requested term pre-placed: " + metKey);
      }
    };



    /// @brief a special accessor for the MET momentum
    ///
    /// MET has a somewhat special momentum structure, and the METMaker
    /// tool also needs to add to the momentum in a special way.  So
    /// this class tries to provide a somewhat convenient and robust
    /// interface for that.
    template<ContainerId CI = ContainerId::met,typename CM=ColumnarModeDefault>
    struct MetMomentumAccessors final
    {
      static constexpr bool isMutable = ContainerIdTraits<CI>::isMutable;
      static constexpr ColumnAccessMode CAM = isMutable?ColumnAccessMode::output:ColumnAccessMode::input;


      MetMomentumAccessors (ColumnarTool<CM>& columnarBase)
        : mpx (columnarBase, "mpx"),
          mpy (columnarBase, "mpy"),
          sumet (columnarBase, "sumet")
      {}

      AccessorTemplate<CI,float,CAM,CM> mpx;
      AccessorTemplate<CI,float,CAM,CM> mpy;
      AccessorTemplate<CI,float,CAM,CM> sumet;

      [[nodiscard]] double met (ObjectId<CI,CM> object) const {
        return std::hypot(mpx(object), mpy(object));}
      [[nodiscard]] double phi (ObjectId<CI,CM> object) const {
        return std::atan2(mpy(object), mpx(object));}

      void addParticle (ObjectId<CI,CM> met, float px,float py,float pt) const requires (isMutable) {
        mpx(met) -= px; mpy(met) -= py; sumet(met) += pt;}
      void addParticle (ObjectId<CI,CM> met, const xAOD::IParticle& particle) const requires (isMutable) {
        auto p4 = particle.p4();
        addParticle (met, p4.Px(), p4.Py(), particle.pt());}
      template<ContainerId CI2,typename MomAcc>
      void addParticle (ObjectId<CI,CM> met, const MomAcc& momAcc, const ObjectId<CI2,CM>& object) const requires (isMutable) && requires(const MomAcc& acc,ObjectId<CI2,CM> object) {float(acc.px(object));float(acc.py(object));float(acc.pt(object));} {
        this->addParticle(met, momAcc.px(object), momAcc.py(object), momAcc.pt(object));}
      template<ContainerId CI2>
      void addMet (ObjectId<CI,CM> met, const MetMomentumAccessors<CI2,CM>& momAcc, ObjectId<CI2,CM> metSource) const requires (isMutable) {
        mpx(met) += momAcc.mpx(metSource); mpy(met) += momAcc.mpy(metSource); sumet(met) += momAcc.sumet(metSource);}
    };
  }
}

#endif
