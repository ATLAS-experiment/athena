#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""An AthenaMP job over bytestream keeps every event and every input's ByteStreamMetadata, as a
serial job does: copy EventInfo of two 4-event RAW files to POOL with MP(2)+SharedWriter and
require all 8 events and both files' records in the output.
- AthenaMP workers reach their events with EventSelectorByteStream::seek(); it fails if seek()
  opens another file without firing the input file incidents, so that the metadata tools never
  see it. A chunk starts at the second file's first event, so that file is only ever opened by
  seek(), whichever worker gets which chunk.
- Chunks of 3 events leave a partial last chunk, which the event counter queues on the last
  file's EndInputFile; it loses those events if EndInputFile comes only at stop()."""
import glob
import sys

OUTPUT = "ByteStreamMetadataMP.pool.root"
EVENTS = 4


def fixtures():
    """Two small RAW files, cut from two standard Run-3 test inputs."""
    import os
    import eformat
    from libpyevent_storage import pickDataReader
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    files = []
    for src in defaultTestFiles.RAW_RUN3 + defaultTestFiles.RAW_RUN3_DATA22:
        dr = pickDataReader(src)
        # the name keeps project, run, stream and lumiblock
        core = os.path.basename(src).split("._0001.")[0] + ".test"
        out = eformat.ostream(directory=".", core_name=core, run_number=dr.runNumber(),
                              trigger_type=dr.triggerType(), detector_mask=dr.detectorMask(),
                              beam_type=dr.beamType(), beam_energy=dr.beamEnergy(),
                              meta_data_strings=list(dr.freeMetaDataStrings()))
        for i, event in enumerate(eformat.istream(src)):
            if i >= EVENTS:
                break
            out.write(event)
        files.append(out.last_filename())
        del out
    return files


def job(inputs):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import MetadataCategory
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    flags = initConfigFlags()
    flags.Input.Files = inputs
    flags.Output.AODFileName = OUTPUT
    flags.Concurrency.NumProcs = 2
    flags.MP.UseSharedWriter = True
    flags.MP.UseParallelCompression = False
    flags.MP.ChunkSize = 3
    flags.lock()
    cfg = MainServicesCfg(flags)
    cfg.merge(ByteStreamReadCfg(flags))
    cfg.merge(OutputStreamCfg(flags, "AOD", ItemList=["xAOD::EventInfo#EventInfo",
                                                      "xAOD::EventAuxInfo#EventInfoAux."]))
    cfg.merge(SetupMetaDataForStreamCfg(flags, "AOD", createMetadata=[MetadataCategory.ByteStreamMetaData]))
    return cfg.run().isSuccess()


def contents(fileName):
    """Number of events, and GUIDs of the ByteStreamMetadata records (one per bytestream file)."""
    import ROOT
    f = ROOT.TFile.Open(fileName)
    events = f.Get("CollectionTree").GetEntries()
    tree = f.Get("MetaData")
    tree.GetEntry(0)
    return events, {str(record.m_guid) for record in tree.ByteStreamMetadata}


def main():
    if not job(fixtures()):
        sys.exit("job failed")
    outputs = glob.glob(OUTPUT) + glob.glob(f"**/shared_writer/{OUTPUT}", recursive=True)
    if not outputs:
        sys.exit(f"no {OUTPUT}")

    events, found = contents(outputs[0])
    print(f"events: {events}, ByteStreamMetadata records: {sorted(found)}")
    if events != 2 * EVENTS:
        sys.exit(f"expected {2 * EVENTS} events")
    if len(found) != 2:
        sys.exit("expected one ByteStreamMetadata record per input")
    print("every event and every input's ByteStreamMetadata kept: OK")


if __name__ == "__main__":
    main()
