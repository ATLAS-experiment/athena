/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_STRING_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_STRING_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarMet/MetDef.h>
#include <ColumnarCore/StringColumn.h>

namespace columnar
{
  /// @brief an example of a columnar tool that reads a vector column
  ///
  /// This illustrates how to read vector columns in a tool.  The actual
  /// example is a bit arcane, but it hopefully illustrates how to read
  /// vector columns.

  class StringExampleTool final
    : public asg::AsgTool,
      public ColumnarTool<>
  {
  public:

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( StringExampleTool, asg::IAsgTool )

    StringExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    virtual void callEvents (EventContextRange events) const override;


    /// @brief the pt cut to apply
    Gaudi::Property<float> m_ptCut {this, "ptCut", 10e3, "pt cut (in MeV)"};


    /// @brief the object accessor for the met map
    ///
    /// This is equivalent to a `ReadHandleKey` in the xAOD world.  It
    /// is used to access the met range/container for a given
    /// event.
    MetAccessor<ObjectColumn> metAcc {*this, "Met"};


    /// @brief a string column accessor
    ///
    /// There is essentially just a single string column in PHYSLITE, so
    /// I'm going with that.
    MetAccessor<std::string> nameAcc {*this, "name"};


    /// @brief the selection decorator for the met terms
    ///
    /// This is the equivalent to an `AuxElement::Decorator` in the xAOD
    /// world.  One thing to note is that the tools generally run on
    /// many objects, not a a single object.  So the tool doesn't have
    /// the option to return individual output values.  Instead it needs
    /// to provide an output value per object, which in the columnar
    /// world is done by filling a column.
    MetDecorator<char> selectionDec {*this, "selection"};
  };
}

#endif
