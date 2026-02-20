/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_BASEOVERLAPTOOL_H
#define ASSOCIATIONUTILS_BASEOVERLAPTOOL_H

// System includes
#include <string>
#include <memory>

// Framework includes
#include "AsgTools/AsgTool.h"

// EDM includes
#include "xAODBase/IParticle.h"

// Columnar includes
#include "ColumnarCore/ColumnarTool.h"
#include "ColumnarCore/ObjectColumn.h"
#include "ColumnarCore/ParticleDef.h"

// Local includes
#include "AssociationUtils/OverlapDecorationHelper.h"
#include "AssociationUtils/OverlapLinkHelper.h"
#include "AssociationUtils/IOverlapTool.h"

namespace ORUtils
{

  /// @class BaseOverlapTool
  /// @brief Common base class tool for overlap tools.
  ///
  /// This class was introduced to reduce the code duplication among the
  /// overlap tools. Most implementations use some common functionality such as
  /// input/output decorations, object linking, and output value logic control.
  /// This class then holds the code to handle those things, or holds the
  /// helper objects which implement the logic. It also defines the properties
  /// associated with these shared functionalities.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class BaseOverlapTool : public asg::AsgTool, public columnar::ColumnarTool<>, virtual public IOverlapTool
  {

      /// Create proper constructor for Athena
      ASG_TOOL_CLASS(BaseOverlapTool, asg::IAsgTool)

    public:

      /// Standalone constructor
      BaseOverlapTool(const std::string& name);

      /// Initialize base class functionality.
      /// Concrete tool specific initialization can be implemented by
      /// overriding the initializeDerived() method.
      StatusCode initialize() override final;

      /// The callEvents() for columnar tools
      virtual void callEvents (columnar::EventContextRange events) const override;

    protected:

      /// Initialization for derived tools.
      /// Default implementation does nothing.
      virtual StatusCode initializeDerived()
      { return StatusCode::SUCCESS; }

      /// @brief Common helper method to handle an overlap result.
      ///
      /// This method for now applies the user-defined object priorities
      /// set as the input decoration, so if the testParticle is higher prio
      /// than the refParticle, nothing happens. Otherwise, this method prints
      /// a debug message, decorates the testParticle as overlap, and
      /// optionally sets the object element link.
      template<columnar::RegularContainerIdConcept CI1,columnar::RegularContainerIdConcept CI2,typename CM>
      StatusCode handleOverlap(const columnar::ObjectId<CI1,CM>& testParticle,
                               const columnar::ObjectId<CI2,CM>& refParticle) const;


      /// check whether the container is of the right type
      template<typename XAODContainer,columnar::RegularContainerIdConcept CI,typename CM>
      StatusCode checkForXAODContainer(columnar::ObjectRange<CI,CM> cont, std::string_view message) const
      {
        if constexpr (CM::isXAOD) {
          if(typeid(cont.getXAODObjectNoexcept()) != typeid(XAODContainer) &&
            typeid(cont.getXAODObjectNoexcept()) != typeid(ConstDataVector<XAODContainer>)) {
            ATH_MSG_ERROR(message);
            return StatusCode::FAILURE;
          }
        }
        return StatusCode::SUCCESS;
      }

      /// @name Common configurable properties
      /// @{

      /// Input object decoration which specifies which objects to look at
      std::string m_inputLabel;
      /// Output object decoration which specifies overlapping objects
      std::string m_outputLabel;

      /// Toggle the output flag logic. If true, then non-overlapping
      /// objects will be assigned "true".
      bool m_outputPassValue;

      /// Flag to toggle overlap object links
      bool m_linkOverlapObjects;

      /// Enable user-priority scoring
      bool m_enableUserPrio;

      /// @}

    protected:
      /// @name Utilities
      /// @{

      struct BaseAccessors final : columnar::ColumnarTool<>
      {
        columnar::Particle1Accessor<columnar::ObjectColumn> m_particles1Acc {*this, ""};
        columnar::Particle2Accessor<columnar::ObjectColumn> m_particles2Acc {*this, ""};
        using ColumnarTool::ColumnarTool;
      };
      std::unique_ptr<BaseAccessors> m_baseAccessors {std::make_unique<BaseAccessors> (this)};

      /// Helper for handling input/output decorations
      std::unique_ptr<OverlapDecorationHelper<columnar::ContainerId::particle1>> m_decHelper1;
      std::unique_ptr<OverlapDecorationHelper<columnar::ContainerId::particle2>> m_decHelper2;

      /// Helper for linking overlap objects
      std::unique_ptr<OverlapLinkHelper<columnar::ContainerId::particle1>> m_objLinkHelper1;
      std::unique_ptr<OverlapLinkHelper<columnar::ContainerId::particle2>> m_objLinkHelper2;

      /// @}


    protected:
      /// @name Helper Functions
      /// @{

      void initializeDecorations(columnar::Particle1Range container) const {
        m_decHelper1->initializeDecorations (container); }
      void initializeDecorations(columnar::Particle2Range container) const {
        m_decHelper2->initializeDecorations (container); }
      [[nodiscard]] char getObjectPriority(columnar::Particle1Id obj) const {
        return m_decHelper1->getObjectPriority (obj); }
      [[nodiscard]] char getObjectPriority(columnar::Particle2Id obj) const {
        return m_decHelper2->getObjectPriority (obj); }
      [[nodiscard]] bool isSurvivingObject(columnar::Particle1Id obj) const {
        return m_decHelper1->isSurvivingObject (obj);}
      [[nodiscard]] bool isSurvivingObject(columnar::Particle2Id obj) const {
        return m_decHelper2->isSurvivingObject (obj);}
      [[nodiscard]] bool isRejectedObject(columnar::Particle1Id obj) const {
        return m_decHelper1->isRejectedObject (obj); }
      [[nodiscard]] bool isRejectedObject(columnar::Particle2Id obj) const {
        return m_decHelper2->isRejectedObject (obj); }
      void setObjectFail(columnar::Particle1Id obj) const {
        m_decHelper1->setObjectFail (obj); }
      void setObjectFail(columnar::Particle2Id obj) const {
        m_decHelper2->setObjectFail (obj); }
      template<columnar::ContainerIdConcept CI>
      StatusCode addObjectLink (columnar::Particle1Id p1, columnar::ObjectId<CI> p2) const {
        return m_objLinkHelper1->addObjectLink (p1, p2); }
      template<columnar::ContainerIdConcept CI>
      StatusCode addObjectLink (columnar::Particle2Id p1, columnar::ObjectId<CI> p2) const {
        return m_objLinkHelper2->addObjectLink (p1, p2); }

      /// @}

  }; // class BaseOverlapTool

  //---------------------------------------------------------------------------
  // Handle overlap condition
  //---------------------------------------------------------------------------
  template<columnar::RegularContainerIdConcept CI1,columnar::RegularContainerIdConcept CI2,typename CM>
  StatusCode BaseOverlapTool ::
  handleOverlap(const columnar::ObjectId<CI1,CM>& testParticle,
                const columnar::ObjectId<CI2,CM>& refParticle) const
  {
    // Apply user-priority override
    if(!m_enableUserPrio ||
       getObjectPriority(testParticle) <=
       getObjectPriority(refParticle))
    {
      if constexpr (CM::isXAOD)
      {
        /// Unit conversion constant
        const float invGeV = 1e-3;
        ATH_MSG_DEBUG("  Found overlap " << testParticle.getXAODObject().type() <<
                      " pt " << testParticle.getXAODObject().pt()*invGeV);
      } else
      {
        ATH_MSG_DEBUG("  Found overlap " << testParticle);
      }
      setObjectFail(testParticle);
      if(m_linkOverlapObjects) {
        ATH_CHECK( addObjectLink(testParticle, refParticle) );
      }
    }
    return StatusCode::SUCCESS;
  }


} // namespace ORUtils

#endif
