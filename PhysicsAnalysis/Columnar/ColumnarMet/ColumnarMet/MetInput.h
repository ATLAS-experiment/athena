/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_MET_MET_INPUT_H
#define COLUMNAR_MET_MET_INPUT_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ContainerId.h>
#include <ColumnarCore/ParticleDef.h>
#include <ColumnarMet/MetDef.h>
#include <xAODCore/ShallowAuxContainer.h>

namespace columnar
{
  namespace MetHelpers
  {
    /// @brief a class that provides all the accessors needed to access
    /// object momentum variables
    ///
    /// For columnar code we don't have the same momentum support as for
    /// xAOD code, mostly because most tools (so far) only need to read
    /// individual momentum members that are taken straight from disk.
    /// maybe at some point there will be more generic momentum support,
    /// but with MET being the only user (so far) it is a specialized
    /// helper for the MET tools.

    template<ContainerIdConcept CI = ContainerId::particle,typename CM=ColumnarModeDefault>
    struct InputMomentumAccessors final
    {
      InputMomentumAccessors (ColumnarTool<CM>& columnarBase)
        : pt (columnarBase, "pt"),
          eta (columnarBase, "eta"),
          phi (columnarBase, "phi"),
          m (columnarBase, "m")    
      {}

      InputMomentumAccessors (ColumnarTool<CM>& columnarBase, const std::string& prefix)
        : pt (columnarBase, prefix + "_pt"),
          eta (columnarBase, prefix + "_eta"),
          phi (columnarBase, prefix + "_phi"),
          m (columnarBase, prefix + "_m")    
      {}

      ColumnAccessor<CI,RetypeColumn<double,float>,CM> pt;
      ColumnAccessor<CI,RetypeColumn<double,float>,CM> eta;
      ColumnAccessor<CI,RetypeColumn<double,float>,CM> phi;
      ColumnAccessor<CI,RetypeColumn<double,float>,CM> m;

      // IMPORTANT:  These have to return float, not double, to match
      // the xAOD interface.  The internal calculations are done in
      // double, and the caller immediately converts it back to double,
      // but converting it to float here introduces a very tiny
      // truncation error that we need to match the xAOD implementation.
      [[nodiscard]] float px (ObjectId<CI,CM> object) const {
        return pt(object) * std::cos(phi(object));}
      [[nodiscard]] float py (ObjectId<CI,CM> object) const {
        return pt(object) * std::sin(phi(object));}

      [[nodiscard]] xAOD::JetFourMom_t jetP4 (ObjectId<CI,CM> jet) const
      {
        return xAOD::JetFourMom_t(pt(jet),eta(jet),phi(jet),m(jet));
      }
    };



    template<ContainerIdConcept CI = ContainerId::particle,typename CM = ColumnarModeDefault> class OriginalObjectHandle;
    template<ContainerIdConcept CI,typename CM> OriginalObjectHandle (const asg::AsgTool&,ObjectRange<CI,CM>) -> OriginalObjectHandle<CI,CM>;

    template<ContainerIdConcept CI> class OriginalObjectHandle<CI,ColumnarModeXAOD> final
    {
      bool m_originalInputs = false;
      bool m_isShallowCopy = false;

    public:

      using CM = ColumnarModeXAOD;

      explicit OriginalObjectHandle (const asg::AsgTool& tool, ObjectRange<CI,CM> container)
      {
        if (!container.empty())
        {
          using namespace MetDef;
          m_originalInputs = !acc_originalObject.isAvailable(*container.getXAODObject().front());
          m_isShallowCopy = dynamic_cast<const xAOD::ShallowAuxContainer*>(container.getXAODObject().front()->container()->getConstStore());
          if (tool.msgLvl(MSG::VERBOSE))
          {
            tool.msg(MSG::VERBOSE) << "const store = " << container.getXAODObject().front()->container()->getConstStore() << endmsg;
          }
        }
      }

      [[nodiscard]] bool originalInputs () const {
        return m_originalInputs;
      }

      [[nodiscard]] bool isShallowCopy () const {
        return m_isShallowCopy;
      }

      [[nodiscard]] ObjectId<CI,CM> getOriginal (const ObjectId<CI,CM>& id) const
      {
        if (m_originalInputs)
        {
          return id;
        } else
        {
          using namespace MetDef;
          auto *originalObject = *acc_originalObject(id.getXAODObject());
          if (!originalObject)
            throw std::runtime_error ("originalObjectLink not available for MET input");
          return ObjectId<CI,CM> (dynamic_cast<typename CI::xAODObjectIdType&>(*originalObject));
        }
      }

      ObjectId<CI,CM> getNominalObject (const ObjectId<CI,CM>& jet) const
      {
        using namespace MetDef;
        if (acc_nominalObject.isAvailable(jet.getXAODObject())) {
          auto nominal = acc_nominalObject(jet.getXAODObject());
          if (nominal && *nominal)
          {
            return ObjectId<CI,CM> {static_cast<CI::xAODObjectIdType&>(**nominal)};
          }
        }
        std::ostringstream message;
        message << "No nominal calibrated jet available for jet " << jet << ". Cannot simplify overlap removal!";
        throw std::runtime_error (message.str());
      }
    };

    template<ContainerIdConcept CI> class OriginalObjectHandle<CI,ColumnarModeArray> final
    {
    public:

      using CM = ColumnarModeArray;

      explicit OriginalObjectHandle (const asg::AsgTool& /*tool*/, ObjectRange<CI,CM> /*container*/)
      {};

      [[nodiscard]] bool originalInputs () const {
        return true;
      }

      [[nodiscard]] bool isShallowCopy () const {
        return false;
      }

      [[nodiscard]] ObjectId<CI,CM> getOriginal (const ObjectId<CI,CM>& id) const
      {
        return id;
      }

      ObjectId<CI,CM> getNominalObject (const ObjectId<CI,CM>& /*jet*/) const
      {
        throw std::runtime_error ("no nominal object available in columnar mode");
      }
    };



    /// @brief an accessor that allows to access the xAOD object type of
    /// an input object
    template<ContainerIdConcept CI,typename CM=ColumnarModeDefault>
    struct ObjectTypeAccessor;

    template<ContainerIdConcept CI>
    struct ObjectTypeAccessor<CI,ColumnarModeXAOD> final
    {
      using CM = ColumnarModeXAOD;

      ObjectTypeAccessor (ColumnarTool<CM>& /*columnBase*/, const std::string& /*columnName*/) {}

      [[nodiscard]] xAODType::ObjectType operator() (ObjectId<CI,CM> object) const
      {
        return object.getXAODObject().type();
      }

      [[nodiscard]] std::optional<xAODType::ObjectType> operator() (ObjectRange<CI,CM> container) const
      {
        if (container.empty())
          return std::nullopt;
        return container.getXAODObject().front()->type();
      }
    };

    template<ContainerIdConcept CI>
    struct ObjectTypeAccessor<CI,ColumnarModeArray> final
    {
      using CM = ColumnarModeArray;

      // FIX ME: ideally this should rely on a global column with a
      // single type, not a type per object, but that functionality
      // doesn't currently (13 Jan 25) exist.
      ColumnAccessor<CI,RetypeColumn<xAODType::ObjectType,std::uint16_t>,CM> typeAcc;

      ObjectTypeAccessor (ColumnarTool<CM>& columnBase, const std::string& columnName)
        : typeAcc (columnBase, columnName) {}

      [[nodiscard]] xAODType::ObjectType operator () (ObjectId<CI,CM> object) const
      {
        return typeAcc(object);
      }

      [[nodiscard]] std::optional<xAODType::ObjectType> operator() (ObjectRange<CI,CM> container) const
      {
        // Technically we may be able to return a type here, but for
        // consistency with the xAOD mode we return nullopt if the
        // container is empty.
        if (container.empty())
          return std::nullopt;
        return typeAcc(container[0]);
      }
    };
  }
}

#endif
