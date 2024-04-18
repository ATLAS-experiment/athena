#!/usr/bin/env python3

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
from collections import defaultdict, namedtuple

import ROOT
from AthenaCommon.Logging import logging
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaPython.PyAthena import Alg, StatusCode, py_svc
from PyUtils.MetaReader import read_metadata
from PyUtils.PoolFile import PoolOpts


class ValidateMetadataAlg(Alg):
    def __init__(self, name="ValidateMetadataAlg", metadata=None):
        super(ValidateMetadataAlg, self).__init__(name=name)
        self.events = set()
        self.Event = namedtuple("Event", ["runNumber", "lumiBlock", "eventNumber"])
        if metadata:
            self.metadata = metadata
        else:
            self.metadata = defaultdict(list)
        return

    def initialize(self):
        self.sg = py_svc("StoreGateSvc")
        return StatusCode.Success

    def execute(self):
        # Read the event info and check the uniqueness of run/lb/evtnumber
        if self.sg.contains("xAOD::EventInfo", "EventInfo"):
            ei = self.sg.retrieve("xAOD::EventInfo", "EventInfo")
            event = self.Event(
                runNumber=ei.runNumber(),
                lumiBlock=ei.lumiBlock(),
                eventNumber=ei.eventNumber(),
            )
            if event in self.events:
                logging.error("Event Data Validation FAILED!")
                return StatusCode.Failure
            else:
                self.events.add(event)
        else:
            logging.error("Could NOT find xAOD::EventInfo!")
            return StatusCode.Failure

        return StatusCode.Success

    def finalize(self):
        if set([event.runNumber for event in self.events]) != set(
            self.metadata["runNumbers"]
        ) or set([event.lumiBlock for event in self.events]) != set(
            self.metadata["lumiBlocks"]
        ):
            return StatusCode.Failure
        return StatusCode.Success


def validateInputMetadata(infile):
    logging.info(f"Using input file {infile}")

    current_file = ROOT.TFile(infile)
    md = read_metadata(infile, mode="full", unique_tag_info_values=False)

    nevts_esi = next(
        (
            md[infile][key]["numberOfEvents"]
            for key, value in md[infile].items()
            if isinstance(value, dict) and "numberOfEvents" in value
        ),
        None,
    )

    dataHeaderTree = current_file.Get(PoolOpts.TTreeNames.DataHeader)
    if isinstance(dataHeaderTree, ROOT.TTree):
        nevts_dh = dataHeaderTree.GetEntriesFast()
    else:
        if (
            current_file.GetListOfKeys().Contains(PoolOpts.RNTupleNames.DataHeader)
            and ROOT.gROOT.GetVersionInt() < 63100
        ):
            raise RuntimeError(
                "ROOT ver. 6.31/01 or greater needed to read RNTuple files"
            )
        dataHeaderRNT = current_file.Get(PoolOpts.RNTupleNames.DataHeader)
        if isinstance(dataHeaderRNT, ROOT.Experimental.RNTuple):
            nevts_dh = ROOT.Experimental.RNTupleReader.Open(dataHeaderRNT).GetNEntries()
        else:
            nevts_dh = None

    if not (md[infile]["nentries"] == nevts_esi == nevts_dh):
        logging.error(
            "Number of events from EventStreamInfo inconsistent with number of entries in DataHeader"
        )
        return 1

    tag_info = md[infile]["/TagInfo"]
    if "project_name" in tag_info and isinstance(tag_info["project_name"], list):
        if "IS_SIMULATION" in tag_info["project_name"] and any(
            [item for item in tag_info["project_name"] if item.startswith("data")]
        ):
            logging.error("/TagInfo contains values reserved for both MC and data")
            return 1
        if (
            len(
                set(
                    item[5:6]
                    for item in tag_info["project_name"]
                    if item.startswith("data")
                )
            )
            > 1
        ):
            logging.error("/TagInfo contains values from different data taking periods")
            return 1
    if (
        "data_year" in tag_info
        and isinstance(tag_info["data_year"], list)
        and set(tag_info["data_year"]) > 1
    ):
        logging.error("/TagInfo contains values from different data taking periods")
        return 1

    return 0


def checkFileMetaData(file_names):
    """Check if FileMetaData is in all files"""

    return all(
        [
            fmd_items
            for file_name in file_names
            for fmd_items in [
                value
                for _, value in GetFileMD(file_name)["metadata_items"].items()
                if "FileMetaData" in value
            ]
        ]
    )


if __name__ == "__main__":
    """
    Script to validate metadata for self-consistency and consistentcy with event data:
    - check if the number of events from EventStreamInfo equals to the number of entries in DataHeader
    - check if /TagInfo metadata contains inconsistent information
    - check if FileMetaData is present 
    - check uniqueness of run/lumiblock/event number per event and against the summary in the FileMetaData
    
    Help: Use as checkMetadata.py --filesInput=DAOD.pool.root"
    """

    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.lock()

    if any(sc := validateInputMetadata(infile) for infile in flags.Input.Files):
        sys.exit(sc)

    try:
        if not checkFileMetaData(flags.Input.Files):
            logging.error("FileMetaData missing")
            sys.exit(1)
    except Exception as exc:
        logging.error(f"Could not read metadata: {exc}")
        sys.exit(1)

    logging.info("Input file metadata looks OK")

    metadata = defaultdict(list)

    for filename in flags.Input.Files:
        metadata["runNumbers"] += GetFileMD(filename).get("runNumbers", [])
        metadata["lumiBlocks"] += GetFileMD(filename).get("lumiBlockNumbers", [])

    # Setup the main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    cfg = MainServicesCfg(flags)

    # Set EventPrintoutInterval to 100 events
    cfg.getService(cfg.getAppProps()["EventLoop"]).EventPrintoutInterval = 1000

    # Setup input reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg.merge(PoolReadCfg(flags))

    # Setup Validation Algorithm
    cfg.addEventAlgo(
        ValidateMetadataAlg("Validator", metadata=metadata),
        sequenceName="AthAlgSeq",
    )

    # Run the job
    sc = cfg.run()

    # Exit accordingly
    sys.exit(not sc.isSuccess())
