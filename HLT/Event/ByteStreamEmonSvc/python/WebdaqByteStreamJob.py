#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
"""Example job running over raw events delivered by the Phase 2 event sampling
(ByteStreamWebdaqInputSvc) instead of reading a bytestream file.

The job is configured exactly like a normal job reading a raw data file, and
the file input is swapped for the sampling service at the very end.

Usage:

  python -m ByteStreamEmonSvc.WebdaqByteStreamJob \\
      --partition=test --groupID=myGroup --streams physics:main [--reco] [flags...]

  No input file is needed: the run information (run number, detector mask, ...)
  comes from the running system via the webproxy when the first event is
  requested. The flags that would otherwise be autoconfigured from an input file
  are set to generic online defaults, any of them can be overridden on the
  command line, e.g. Input.RunNumbers=[123456].

  For offline testing --filesInput can still be given; the flags are then
  autoconfigured from that file as usual, but no events are read from it.

  Without --reco a minimal algorithm printing xAOD::EventInfo is run.
  With --reco the standard RecoSteering is run on the sampled events.
"""

import json

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def EventInfoPrinterCfg(flags):
    """Minimal payload: dump xAOD::EventInfo (run/event/LB/BCID...) of each event"""
    from AthenaCommon.Constants import DEBUG
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.xAODReader.EventInfoReaderAlg(
        "EventInfoPrinter", OutputLevel=DEBUG))
    return acc


def parseStreams(streams):
    """Convert ['physics:main', 'physics:MinBias', 'calibration:X'] into
    {'physics': ['main', 'MinBias'], 'calibration': ['X']}"""
    result = {}
    for s in streams:
        stype, _, sname = s.partition(':')
        if not sname:
            raise ValueError(f"Stream '{s}' must be given as type:name")
        result.setdefault(stype, []).append(sname)
    return result


def setNoInputFileFlags(flags):
    """Set the flags that are otherwise autoconfigured from the input file.

    Online there is no input file. The actual run information is read from
    IS by ByteStreamWebdaqInputSvc at run time, so generic values are used
    here (the same as AthenaConfiguration.AutoConfigOnlineRecoFlags uses
    when IS is not available).
    """
    from AthenaConfiguration.Enums import LHCPeriod
    flags.Input.Files = []
    flags.Input.RunNumbers = [-1]
    flags.Input.LumiBlockNumbers = [-1]
    flags.Input.TimeStamps = [-1]
    flags.Input.ProjectName = "data_test"
    flags.Input.ProcessingTags = []
    flags.GeoModel.Run = LHCPeriod.Run3   # avoid geometry DB lookup
    flags.Trigger.EDMVersion = 3


def main():
    import argparse
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import Format

    flags = initConfigFlags()

    # Raw data files carry no geometry/conditions tag. Set defaults before
    # parsing the command line so they can be overridden there, e.g.
    # GeoModel.AtlasVersion=... IOVDb.GlobalTag=...
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA
    flags.IOVDb.DatabaseInstance = "CONDBR2"

    # Without input file set all input dependent flags before parsing the
    # command line, so that explicit flag settings there take precedence
    pre_parser = argparse.ArgumentParser(add_help=False)
    pre_parser.add_argument("--filesInput")
    if pre_parser.parse_known_args()[0].filesInput is None:
        setNoInputFileFlags(flags)

    parser = flags.getArgumentParser()
    grp = parser.add_argument_group("Event sampling")
    grp.add_argument("--partition", default="",
                     help="Partition name (default: $TDAQ_PARTITION)")
    grp.add_argument("--samplerType", default="eb",
                     help="Sampler type (default: eb)")
    grp.add_argument("--samplerNames", nargs="*", default=[],
                     help="Explicit list of sampler names")
    grp.add_argument("--groupID", default="",
                     help="Group ID, clients in the same group share events")
    grp.add_argument("--streams", nargs="*", default=[],
                     help="Selected streams as type:name, e.g. physics:main")
    grp.add_argument("--l1Type", type=lambda x: int(x, 0), default=None,
                     help="L1 trigger type selection")
    grp.add_argument("--statusMask", type=lambda x: int(x, 0), default=None,
                     help="Status word mask")
    grp.add_argument("--timeout", type=int, default=60,
                     help="Event receive timeout in seconds, -1 == infinity")
    grp.add_argument("--processCorrupted", action="store_true",
                     help="Also process corrupted events")
    grp.add_argument("--reco", action="store_true",
                     help="Run standard reconstruction instead of the EventInfo printer")

    args = flags.fillFromArgs(parser=parser)

    # Raw data, real detector
    flags.Input.Format = Format.BS
    flags.Input.isMC = False
    flags.Common.isOnline = True
    if flags.Exec.MaxEvents == 0:
        flags.Exec.MaxEvents = -1

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    if args.reco:
        from RecJobTransforms.RecoSteering import RecoSteering
        acc.merge(RecoSteering(flags))
    else:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags))
        acc.merge(EventInfoPrinterCfg(flags))

    # Now swap file input for event sampling
    criteria = {}
    if args.streams:
        criteria["streams"] = parseStreams(args.streams)
    if args.l1Type is not None:
        criteria["l1_type"] = args.l1Type
    if args.statusMask is not None:
        criteria["status_mask"] = args.statusMask

    from ByteStreamEmonSvc.WebdaqByteStreamConfig import replaceByteStreamInputWithWebdaq
    replaceByteStreamInputWithWebdaq(
        flags, acc,
        Partition=args.partition,
        SamplerType=args.samplerType,
        SamplerNames=args.samplerNames,
        GroupID=args.groupID,
        Criteria=json.dumps(criteria) if criteria else "",
        Timeout=args.timeout,
        ProcessCorruptedEvents=args.processCorrupted)

    acc.printConfig(withDetails=False)

    import sys
    sys.exit(acc.run().isFailure())


if __name__ == "__main__":
    main()
