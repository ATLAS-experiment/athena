/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HGTD_ALIGNGENTOOLS_HGTDALIGNDBTOOL_H
#define HGTD_ALIGNGENTOOLS_HGTDALIGNDBTOOL_H

// HGTD_AlignDBTool.h
// AlgTool for creating and managing HGTD alignment payloads.
// Unlike the legacy Inner Detector alignment framework, HGTD alignment
// constants are represented directly at the detector-module level.
// Each HGTD detector element has one AlignableTransform entry.
// The tool provides utilities to
//  • create an empty alignment payload
//  • modify module transforms
//  • retrieve module transforms
//  • print payload contents
//  • stream payloads to POOL/SQLite
//  • register payloads in the Conditions Database
// Fatima Bendebba, started 2026

#include <string>
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "CxxUtils/checker_macros.h"
#include "HGTD_AlignGenTools/IHGTD_AlignDBTool.h"
#include "HGTD_Identifier/HGTD_ID.h"
#include "Identifier/IdentifierHash.h"

class Identifier;
class AlignableTransform;
class HGTD_DetectorManager;
class HGTD_ID;

class ATLAS_NOT_THREAD_SAFE HGTD_AlignDBTool
  : public AthAlgTool,
    virtual public IHGTD_AlignDBTool
{

public:

    HGTD_AlignDBTool(const std::string& type,
                    const std::string& name,
                    const IInterface* parent);

    virtual ~HGTD_AlignDBTool() = default;

    StatusCode initialize() override;
    StatusCode createDB() override;

    bool setTrans(const Identifier&,
                  unsigned int level,
                  const Amg::Transform3D&) const override;

    bool setTrans(const Identifier&,
                  unsigned int level,
                  const Amg::Vector3D&,
                  double alpha,
                  double beta,
                  double gamma) const override;

    bool tweakTrans(const Identifier&,
                    unsigned int level,
                    const Amg::Transform3D&) const override;

    bool tweakTrans(const Identifier&,
                    unsigned int level,
                    const Amg::Vector3D&,
                    double alpha,
                    double beta,
                    double gamma) const override;

    virtual Amg::Transform3D getTrans(const Identifier&,
                                      unsigned int level) const override;

    StatusCode outputObjs() override;

    StatusCode fillDB(const std::string& tag,
                      unsigned int run1,
                      unsigned int event1,
                      unsigned int run2,
                      unsigned int event2) const override;

    void printDB() const override;

    void sortTrans() const override;

private:

    /// Build the AlignableTransform tag corresponding to one HGTD module
    std::string moduleTag(const Identifier& id) const;
    
    /// Retrieve writable AlignableTransform
    AlignableTransform* getTransPtr(const Identifier& id) const;

    /// Retrieve read-only AlignableTransform
    const AlignableTransform* cgetTransPtr(const Identifier& id) const;

    /// HGTD detector manager
    const HGTD_DetectorManager* m_detManager{};

    /// HGTD Identifier helper
    const HGTD_ID* m_hgtdId{nullptr};

    /// Output stream
    ToolHandle<IAthenaOutputStreamTool> m_condStream{
        this,
        "CondStream",
        "AthenaOutputStreamTool/AthenaOutputStreamTool"
    };

    /// Alignment folder
    Gaudi::Property<std::string> m_dbRoot{
        this,
        "DBRoot",
        "/HGTD/Align",
        "Root folder for HGTD alignment conditions"
    };

    /// Detector manager name
    Gaudi::Property<std::string> m_detManagerName{
        this,
        "DetectorManager",
        "HGTD",
        "HGTD detector manager name"
    };

};

#endif