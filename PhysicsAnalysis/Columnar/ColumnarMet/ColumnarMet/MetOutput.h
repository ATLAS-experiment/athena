/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_MET_MET_OUTPUT_H
#define COLUMNAR_MET_MET_OUTPUT_H

#include <ColumnarCore/ContainerId.h>
#include <ColumnarMet/MetInput.h>
#include <METUtilities/METHelpers.h>

namespace columnar
{
  namespace MetHelpers
  {
    /// @brief a special "decorator" for decorating the weight an object
    /// contributes to a MET term
    ///
    /// This needs a special decorator because in xAOD mode the weights
    /// get decorated on the MET term itself, while in array mode it
    /// gets decorated on the objects.  Essentially the memory
    /// management of adding variable length arrays onto the MET terms
    /// is very difficult in Array mode, whereas adding an extra column
    /// that contains a float per object is very straightforward.  It is
    /// up to the user to make sure that for each term a different
    /// weight vector gets passed in.

    template<ContainerIdConcept CI_MET=ContainerId::mutableMet,ContainerIdConcept CI_OBJ=ContainerId::particle,typename CM=ColumnarModeDefault> class ObjectWeightDecorator;


    template<ContainerIdConcept CI_MET,ContainerIdConcept CI_OBJ> class ObjectWeightDecorator<CI_MET,CI_OBJ,ColumnarModeXAOD> final
    {
      /// Common Public Members
      /// =====================
    public:

      using CM = ColumnarModeXAOD;

      ObjectWeightDecorator (ColumnarTool<CM>& /*columnarBase*/, const std::string& /*weightSuffix*/, bool val_writeWeights) : writeWeights (val_writeWeights) {}

      bool writeWeights;
    };


    template<ContainerIdConcept CI_MET,ContainerIdConcept CI_OBJ> class ObjectWeightDecorator<CI_MET,CI_OBJ,ColumnarModeArray> final
    {
      /// Common Public Members
      /// =====================
    public:

      using CM = ColumnarModeArray;

      ObjectWeightDecorator (ColumnarTool<CM>& columnarBase, const std::string& weightSuffix, bool /*writeWeights*/) : weightDec (columnarBase, "MetObjectWeight" + weightSuffix, {.isOptional = true}) {}

      ColumnDecorator<CI_OBJ,float,CM> weightDec;
    };



    /// @brief a "handle" for recording object weights via @ref
    /// ObjectWeightDecorator
    ///
    /// This is initialized with a decorator, a MET term, and a
    /// container and then does the "right thing" to record the object
    /// weight based on the columnar mode.

    template<ContainerIdConcept CI_MET=ContainerId::mutableMet,ContainerIdConcept CI_OBJ=ContainerId::particle,typename CM=ColumnarModeDefault> class ObjectWeightHandle;

    template<ContainerIdConcept CI_MET,ContainerIdConcept CI_OBJ> class ObjectWeightHandle<CI_MET,CI_OBJ,ColumnarModeXAOD> final
    {
      /// Common Public Members
      /// =====================
    public:

      using CM = ColumnarModeXAOD;

      using iplink_t = MetDef::iplink_t;

      ObjectWeightHandle (const asg::AsgTool& tool, const ObjectWeightDecorator<CI_MET,CI_OBJ,CM>& decorator, ObjectId<CI_MET,CM> met, const ObjectRange<CI_OBJ,CM>& container)
      {
        using namespace MetDef;

        dec_constitObjLinks(met.getXAODObject()) = std::vector<iplink_t>(0);
        m_uniqueLinks = &dec_constitObjLinks(met.getXAODObject());
        m_uniqueLinks->reserve (container.size());
        if (decorator.writeWeights)
        {
          dec_constitObjWeights(met.getXAODObject()) = std::vector<float>(0);
          m_uniqueWeights = &dec_constitObjWeights(met.getXAODObject());
          m_uniqueWeights->reserve (container.size());
        }
    
        if(container.getXAODObject().ownPolicy() == SG::OWN_ELEMENTS) {
          m_collectionSgKey = tool.getKey(&container.getXAODObject());
          if(m_collectionSgKey == 0) {
            std::ostringstream message;
            message << "Could not get the SG key for the collection with pointer: "
                    << &container.getXAODObject();
            throw std::runtime_error(message.str());
          }
        }
      }

      [[nodiscard]] std::size_t size () const noexcept {
        return m_uniqueLinks->size();}

      void emplace_back (const ObjectId<CI_OBJ,CM>& particle,float weight) {
        using iplink_t = ElementLink<xAOD::IParticleContainer>;
        iplink_t objLink;

        if(m_collectionSgKey == 0) {
          const xAOD::IParticleContainer* ipc = static_cast<const xAOD::IParticleContainer*>(particle.getXAODObject().container());
          objLink = iplink_t(*ipc, particle.getXAODObject().index());
        } else {
          objLink = iplink_t(m_collectionSgKey, particle.getXAODObject().index());
        }
        emplace_back (objLink, weight);
      }


      /// Private Members
      /// ===============
    private:

      SG::sgkey_t m_collectionSgKey = 0;

      std::vector<iplink_t>* m_uniqueLinks = nullptr;
      std::vector<float>* m_uniqueWeights = nullptr;

      std::vector<iplink_t>& uniqueLinks () noexcept {
        return *m_uniqueLinks;}
      std::vector<float>& uniqueWeights () {
        if (!m_uniqueWeights)
          throw std::runtime_error ("weights not enabled");
        return *m_uniqueWeights;}

      void emplace_back (const iplink_t& link,float weight) {
        m_uniqueLinks->emplace_back (link);
        if (m_uniqueWeights)
          m_uniqueWeights->emplace_back (weight);}
    };



    template<ContainerIdConcept CI_MET,ContainerIdConcept CI_OBJ> class ObjectWeightHandle<CI_MET,CI_OBJ,ColumnarModeArray> final
    {
      /// Common Public Members
      /// =====================
    public:

      using CM = ColumnarModeArray;

      ObjectWeightHandle (const asg::AsgTool& /*tool*/, const ObjectWeightDecorator<CI_MET,CI_OBJ,CM>& decorator, ObjectId<CI_MET,CM> /*met*/, const ObjectRange<CI_OBJ,CM>& container)
      {
        if (!decorator.weightDec.isAvailable(container))
          throw std::runtime_error ("weights column not provided");

        m_weights = decorator.weightDec(container);
        for (auto& weight : m_weights)
          weight = 0;
        m_offset = container.beginIndex();
      }

      [[nodiscard]] std::size_t size () const noexcept
      {
        std::size_t result = 0;
        for (auto weight : m_weights)
          if (weight != 0)
            ++ result;
        return result;
      }

      void emplace_back (const ObjectId<CI_OBJ,CM>& particle,float weight)
      {
        m_weights[particle.getIndex() - m_offset] = weight;
      }



      /// Private Members
      /// ===============
    private:

      std::span<float> m_weights;
      std::size_t m_offset = 0;
    };
  }
}

#endif
