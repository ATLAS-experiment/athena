/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HGTD_ALIGNGENTOOLS_IHGTD_ALIGNDBTOOL_H
#define HGTD_ALIGNGENTOOLS_IHGTD_ALIGNDBTOOL_H

// Interface for the HGTD alignment database tool.
// The interface provides utilities to create, modify, retrieve,
// print and store HGTD alignment payloads.
// Alignment constants are represented at detector-module level,
// where each HGTD detector element owns one AlignableTransform.
// Fatima Bendebba, started 2026

#include <string>

#include "GaudiKernel/IAlgTool.h"
#include "GeoPrimitives/GeoPrimitives.h"

class Identifier;

static const InterfaceID
IID_IHGTD_AlignDBTool("IHGTD_AlignDBTool",1,0);

class IHGTD_AlignDBTool : virtual public IAlgTool {

public:
    static const InterfaceID& interfaceID();

    /// Create an empty alignment payload
    virtual StatusCode createDB() = 0;

    /// Replace the transform of one module
    virtual bool setTrans(const Identifier&,
                          unsigned int level,
                          const Amg::Transform3D&) const = 0;

    /// Replace the transform using translation + rotations
    virtual bool setTrans(const Identifier&,
                          unsigned int level,
                          const Amg::Vector3D&,
                          double alpha,
                          double beta,
                          double gamma) const = 0;

    /// Apply an incremental transform
    virtual bool tweakTrans(const Identifier&,
                            unsigned int level,
                            const Amg::Transform3D&) const = 0;

    /// Apply an incremental transform using translation + rotations
    virtual bool tweakTrans(const Identifier&,
                            unsigned int level,
                            const Amg::Vector3D&,
                            double alpha,
                            double beta,
                            double gamma) const = 0;

    /// Retrieve transform
    virtual Amg::Transform3D getTrans(const Identifier&,
                                      unsigned int level) const = 0;

    /// Stream payload to POOL
    virtual StatusCode outputObjs() = 0;

    /// Register payload in the Conditions DB
    virtual StatusCode fillDB(const std::string& tag,
                              unsigned int run1,
                              unsigned int event1,
                              unsigned int run2,
                              unsigned int event2) const = 0;

    /// Print payload
    virtual void printDB() const = 0;

    /// Sort AlignableTransform entries
    virtual void sortTrans() const = 0;
};

inline const InterfaceID&
IHGTD_AlignDBTool::interfaceID()
{
    return IID_IHGTD_AlignDBTool;
}

#endif