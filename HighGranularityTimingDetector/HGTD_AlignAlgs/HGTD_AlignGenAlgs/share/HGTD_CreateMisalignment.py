#!/usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# ----------------------------------------------------------
# Output files produced by this job:
#
#  - HGTD_CreateMisalignment.root
#       Validation ROOT tree (THistSvc)
#
#  - HGTD_Misalignment.pool.root
#       POOL conditions payload
#
#  - HGTD_MisalignmentModeX.db
#       SQLite COOL database
# ----------------------------------------------------------

# --------------------------------------------------------------
# Flags
# --------------------------------------------------------------
def getFlags(**kwargs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    from AthenaConfiguration.TestDefaults import (
        defaultGeometryTags,
        defaultConditionsTags,
    )

    flags = initConfigFlags()

    flags.Input.isMC = True
    flags.Input.Files = []

    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.GeoModel.Align.Dynamic = False

    # HGTD geometry
    flags.HGTD.Geometry.isAlignable = True
    flags.HGTD.Geometry.useGeoModelXml = True
    flags.HGTD.Geometry.isLocal = True

    flags.HGTD.Geometry.Filename = kwargs.get(
        "GeometryFile",
        "HGTD_Detector/HGTD.gmx"
    )

    setupDetectorFlags(
        flags,
        custom_list=["HGTD"],
        toggle_geometry=True
    )

    #----------------------------------------------------------
    # Output SQLite database
    #----------------------------------------------------------

    MisalignMode = int(kwargs.get("MisalignMode", 1))

    databaseFilename = f"HGTD_MisalignmentMode{MisalignMode}.db"

    flags.IOVDb.DBConnection = (
        f"sqlite://;schema={databaseFilename};dbname=OFLCOND"
    )

    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.Exec.OutputLevel = 2
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.lock()
    return flags

# --------------------------------------------------------------
# Build job
# --------------------------------------------------------------
def CreateMis(flags, **kwargs):

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.ComponentFactory import CompFactory
    from HGTD_AlignGenTools.HGTD_AlignGenToolsConfig import HGTD_AlignDBToolCfg
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg

    acc = MainServicesCfg(flags)
    acc.merge(IOVDbSvcCfg(flags))

    # ----------------------------------------------------------
    # ROOT output for validation
    # Creates HGTD_CreateMisalignment.root containing one entry
    # per HGTD detector element with the applied misalignment.
    # ----------------------------------------------------------

    histoSvc = CompFactory.THistSvc(
        Output=[
            "CREATEMISALIGN DATAFILE='HGTD_CreateMisalignment.root' " "TYPE='ROOT' OPT='RECREATE'"
        ]
    )

    acc.addService(histoSvc)
    acc.setAppProperty("HistogramPersistency", "ROOT")

    # ----------------------------------------------------------
    # Configure HGTD alignment DB tool
    # ----------------------------------------------------------

    kargsTool = {}
    kargsTool.setdefault("DBRoot", "/HGTD/Align")

    HGTDCondStream = CompFactory.AthenaOutputStreamTool(
        "CondStream_write",
        OutputFile="HGTD_Misalignment.pool.root"
    )

    HGTDCondStream.PoolContainerPrefix = "<type>"
    HGTDCondStream.TopLevelContainerName = ""
    HGTDCondStream.SubLevelBranchName = "<key>"

    kargsTool.setdefault("CondStream", HGTDCondStream)

    dbTool = acc.popToolsAndMerge(
        HGTD_AlignDBToolCfg(flags, **kargsTool)
    )

    MisalignMode = int(kwargs.get("MisalignMode", 1))
    createFreshDB = True
    outFile = f"HGTD_MisalignmentMode{MisalignMode}"

    print()
    print("==============================================")
    print(f" HGTD Misalignment Mode {MisalignMode}")
    print("==============================================")
    print()

    # ----------------------------------------------------------
    # Convert command-line strings to the correct property types
    # ----------------------------------------------------------

    kwargs["MisalignMode"] = int(kwargs.get("MisalignMode", 1))
    kwargs["ApplyTranslation"] = (str(kwargs.get("ApplyTranslation", "True")).lower() in ("true", "1", "yes"))
    kwargs["ApplyRotation"] = (str(kwargs.get("ApplyRotation", "False")).lower() in ("true", "1", "yes"))

    kwargs["ShiftX"] = float(kwargs.get("ShiftX", 0.05))
    kwargs["ShiftY"] = float(kwargs.get("ShiftY", 0.0))
    kwargs["ShiftZ"] = float(kwargs.get("ShiftZ", 0.0))

    kwargs["SigmaX"] = float(kwargs.get("SigmaX", 0.01))
    kwargs["SigmaY"] = float(kwargs.get("SigmaY", 0.01))
    kwargs["SigmaZ"] = float(kwargs.get("SigmaZ", 0.01))

    kwargs["MaxModules"] = int(kwargs.get("MaxModules", -1))
    kwargs["TestHash"] = int(kwargs.get("TestHash", -1))

    kwargs["DoDebugPrint"] = True
    kwargs["WriteToDB"] = True
    kwargs["CreateFreshDB"] = createFreshDB
    kwargs["SQLiteTag"] = f"HGTD_MisalignmentMode_{MisalignMode}"
    kwargs["OutputFile"] = outFile + ".txt"

    alg_kwargs = dict(kwargs)
    alg_kwargs.pop("GeometryFile", None)

    alg_kwargs["RandomStream"] = kwargs.get(
        "RandomStream",
        "HGTDMisalignment"
    )

    # ----------------------------------------------------------
    # Add HGTD misalignment algorithm
    # ----------------------------------------------------------

    from HGTD_AlignGenAlgs.HGTD_AlignAlgsConfig import (
        HGTD_CreateMisalignAlgCfg
    )

    alg_kwargs["AlignDBTool"] = dbTool

    acc.merge(
        HGTD_CreateMisalignAlgCfg(
            flags,
            **alg_kwargs
        )
    )

    return acc

# --------------------------------------------------------------
# Main
# --------------------------------------------------------------
if __name__ == "__main__":
    import sys

    if len(sys.argv[1:]):

        kwargs = dict(
            arg.split("=")
            for arg in sys.argv[1:]
        )
        print(kwargs)

    else:

        kwargs = {}
        print("No command-line arguments")

    flags = getFlags(**kwargs)
    flags.dump()

    acc = CreateMis(flags, **kwargs)

    iovdb = acc.getService("IOVDbSvc")
    print("======================================")
    print("IOVDbSvc.dbConnection =", iovdb.dbConnection)
    print("======================================")

    acc.printConfig(
        withDetails=True,
        summariseProps=True
    )

    sc = acc.run(1)

    if sc.isFailure():

        print("Failed to run HGTD Misalignment Algorithm")
        sys.exit(-1)

    print("HGTD Misalignment finished successfully.")