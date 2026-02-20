/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_OVERLAPDECORATIONHELPER_H
#define ASSOCIATIONUTILS_OVERLAPDECORATIONHELPER_H

// EDM includes
#include "xAODBase/IParticleContainer.h"

// Columnar includes
#include "ColumnarCore/ColumnAccessor.h"
#include "ColumnarCore/ColumnarTool.h"
#include "ColumnarCore/ObjectRange.h"

// Local includes
#include "AssociationUtils/OverlapRemovalDefs.h"

namespace ORUtils
{

  /// @class OverlapDecorationHelper
  /// @brief Encapsulates the code needed to access and set overlap-related
  /// decorations.
  ///
  /// This utility class is used by the OverlapTools, but in principle could
  /// probably be used by a user as well.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  template<columnar::ContainerIdConcept CI>
  class OverlapDecorationHelper : public columnar::ColumnarTool<>
  {

    public:

      /// @brief Constructor
      ///
      /// @param inputLabel Input decoration name
      /// @param outputLabel Output decoration name
      /// @param outputPassValue Specifies the boolean value to assign to
      ///        passing, or non-overlapping objects. Default value is false
      ///        for historical reasons.
      OverlapDecorationHelper(const std::string& inputLabel,
                              const std::string& outputLabel,
                              bool outputPassValue = false);

      /// Check if object is flagged as input for OR
      bool isInputObject(columnar::ObjectId<CI> obj) const;

      /// Check if an object has been rejected by decoration
      bool isRejectedObject(columnar::ObjectId<CI> obj) const;

      /// Check if object is surviving OR thus far
      bool isSurvivingObject(columnar::ObjectId<CI> obj) const;

      /// Get the user priority score, which is currently the input decoration
      char getObjectPriority(columnar::ObjectId<CI> obj) const;

      /// Set output decoration on object, pass or fail
      void setOverlapDecoration(columnar::ObjectId<CI> obj, bool result) const;

      /// Shorthand way to set an object as passing overlap removal
      void setObjectPass(columnar::ObjectId<CI> obj) const;

      /// Shorthand way to set an object as failing overlap removal
      void setObjectFail(columnar::ObjectId<CI> obj) const;

      /// Check if output decoration has been applied to a container.
      /// Returns false if the container is empty.
      /// Output logic independent.
      bool isDecorated(columnar::ObjectRange<CI> container) const;

      /// Initialize decorations for a container to "pass".
      /// Note that the value written depends on the output pass-value.
      void initializeDecorations(columnar::ObjectRange<CI> container) const;

      /// Helper method for setting all objects as passing
      void resetDecorations(columnar::ObjectRange<CI> container) const;

    private:

      /// Toggle usage of input label
      bool m_useInputLabel;

      /// Input label accessor
      columnar::ColumnAccessor<CI,char> m_inputAccessor;
      /// Output decorator
      columnar::ColumnDecorator<CI,char> m_outputDecorator;

      /// Output decoration logic
      bool m_outputPassValue;

  }; // class OverlapDecorationHelper

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  OverlapDecorationHelper<CI>::
  OverlapDecorationHelper(const std::string& inputLabel,
                          const std::string& outputLabel,
                          bool outputPassValue)
    : m_useInputLabel(!inputLabel.empty()),
      m_outputDecorator(*this, outputLabel),
      m_outputPassValue(outputPassValue)
  {
    if(m_useInputLabel)
      resetAccessor (m_inputAccessor, *this, inputLabel);
  }

  //---------------------------------------------------------------------------
  // Determine if object is currently OK for input to OR
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  bool OverlapDecorationHelper<CI>::isInputObject
  (columnar::ObjectId<CI> obj) const
  {
    // Input label is turned off if empty string
    if(!m_useInputLabel) return true;
    return m_inputAccessor(obj);
  }

  //---------------------------------------------------------------------------
  // Determine if object is currently rejected by OR
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  bool OverlapDecorationHelper<CI>::isRejectedObject
  (columnar::ObjectId<CI> obj) const
  {
    // isRejected = isInput && (output == fail)
    return isInputObject(obj) && ( m_outputDecorator(obj) != m_outputPassValue );
  }
  //---------------------------------------------------------------------------
  // Determine if object is NOT currently rejected by OR (is surviving)
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  bool OverlapDecorationHelper<CI>::isSurvivingObject
  (columnar::ObjectId<CI> obj) const
  {
    // isSurviving = isInput && (output == pass)
    return isInputObject(obj) && ( m_outputDecorator(obj) == m_outputPassValue );
  }
  //---------------------------------------------------------------------------
  // Retrieve the user-defined object priority
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  char OverlapDecorationHelper<CI>::
  getObjectPriority(columnar::ObjectId<CI> obj) const
  {
    // We current reuse the input decoration as the priority score
    return m_inputAccessor(obj);
  }

  //---------------------------------------------------------------------------
  // Set the overlap (output) decoration
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  void OverlapDecorationHelper<CI>::setOverlapDecoration
  (columnar::ObjectId<CI> obj, bool result) const
  {
    m_outputDecorator(obj) = result;
  }
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  void OverlapDecorationHelper<CI>::setObjectPass(columnar::ObjectId<CI> obj) const
  {
    setOverlapDecoration(obj, m_outputPassValue);
  }
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  void OverlapDecorationHelper<CI>::setObjectFail(columnar::ObjectId<CI> obj) const
  {
    setOverlapDecoration(obj, !m_outputPassValue);
  }
  
  //---------------------------------------------------------------------------
  // Check if output decoration has been applied to a container
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  bool OverlapDecorationHelper<CI>::isDecorated
  (columnar::ObjectRange<CI> container) const
  {
    return container.size() > 0 &&
           m_outputDecorator.isAvailable(container[0]);
  }
  
  //---------------------------------------------------------------------------
  // Initialize output decoration
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  void OverlapDecorationHelper<CI>::initializeDecorations
  (columnar::ObjectRange<CI> container) const
  {
    if(!isDecorated(container))
      resetDecorations(container);
  }
  
  //---------------------------------------------------------------------------
  // Reset output decoration
  //---------------------------------------------------------------------------
  template<columnar::ContainerIdConcept CI>
  void OverlapDecorationHelper<CI>::resetDecorations
  (columnar::ObjectRange<CI> container) const
  {
    for(auto obj : container){
      // This isn't terrible intuitive, but in order to support both output
      // logic modes in a reasonable way, we initialize the output flag to the
      // logical AND of isInput and outputPassValue. This results in non-input
      // objects being initialized to 'false' regardless of output logic.
      bool result = ( isInputObject(obj) && m_outputPassValue );
      setOverlapDecoration(obj, result);
    }
    //for(auto obj : container) setObjectPass(obj);
  }

} // namespace ORUtils

#endif
