// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_ROOTTYPES_H
#define XAODROOTACCESS_ROOTTYPES_H

// ROOT include(s).
#include <ROOT/REntry.hxx>
#include <ROOT/RFieldBase.hxx>
#include <ROOT/RNTupleInspector.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleView.hxx>
#include <ROOT/RNTupleWriter.hxx>

// Make the RNTuple types available in the ROOT namespace
// with all versions of ROOT.
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 35, 0)
namespace ROOT {
using Experimental::REntry;
using Experimental::RNTupleInspector;
using Experimental::RNTupleModel;
using Experimental::RNTupleReader;
using Experimental::RNTupleView;
using Experimental::RNTupleWriter;
using Experimental::DescriptorId_t;
using Experimental::RException;
using Experimental::RFieldBase;
using Experimental::RFieldDescriptor;
}  // namespace ROOT
#endif  // ROOT_VERSION_CODE < ROOT_VERSION(6, 35, 0)

#endif  // XAODROOTACCESS_ROOTTYPES_H
