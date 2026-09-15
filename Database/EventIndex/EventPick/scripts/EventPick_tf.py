#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""
Transformation script to pick events from ATLAS files (RAW/BS, ESD, AOD, DAOD, EVNT, HITS) using acmd filter-files.
"""

import sys
import re
import logging
from PyJobTransforms.trfExceptions import TransformExecutionException
from PyJobTransforms.transform import transform
from PyJobTransforms.trfExe import scriptExecutor
import PyJobTransforms.trfArgClasses as trfArgClasses

# Fetch the framework's centralized logger profile instance
msg_log = logging.getLogger('EventPick_tf')

class AcmdFilterExecutor(scriptExecutor):
    """
    Subclass scriptExecutor to handle dynamic ATLAS file types in the graph.
    Parses the event file locally and passes a formatted Python list literal to acmd.
    """
    def __init__(self, name="AcmdFilterFiles"):
        supported_inputs = ['BSFiles', 'ESDFiles', 'AODFiles', 'DAODFiles', 'EVNTFiles', 'HITSFiles']
        supported_outputs = [f"{t.replace('Files', '')}_PICKED" for t in supported_inputs]
        super().__init__(name=name, exe="acmd", inData=supported_inputs, outData=supported_outputs)
        self.exeArgs = []

    def preExecute(self, *args, **kwargs):
        # 1. Run standard parent method execution flow
        super().preExecute(*args, **kwargs)

        # 2. Re-assign the logger directly to protect it from framework resets
        self.log = msg_log

        # 3. Identify which input and output keys were supplied on the command line
        input_key = next((k for k in self.conf.argdict if k.startswith('input') and k.endswith('Files')), None)
        output_key = next((k for k in self.conf.argdict if k.startswith('output') and k.endswith('File')), None)

        if not input_key or not output_key:
            raise TransformExecutionException("Execution failed: Missing required input files or output file specification.")

        # 4. Extract and match data type variants
        in_match = re.match(r"input([A-Z_]+)Files", input_key)
        out_match = re.match(r"output([A-Z_]+)_PICKEDFile", output_key)
        
        in_type = in_match.group(1) if in_match else None
        out_type = out_match.group(1) if out_match else None

        if in_type != out_type:
            msg = f"Data Mismatch Error: Input type '{in_type}' does not match output type '{out_type}'! Execution aborted."
            msg_log.critical(msg)
            raise TransformExecutionException(msg)

        # 5. Retrieve argument payloads
        input_files = self.conf.argdict[input_key].value
        output_file = self.conf.argdict[output_key].value[0]
        event_list_file = self.conf.argdict['eventList'].value[0]

        # 6. Read and parse the event list file into a valid Python list literal string
        try:
            events = []
            with open(event_list_file, 'r') as f:
                for line in f:
                    line = line.strip()
                    if not line or line.startswith('#'):
                        continue
                    # Handle both single event numbers or comma/space separated tokens per line
                    if ',' in line or ' ' in line:
                        # If the line contains run/event numbers like "12345, 67890", convert to tuple
                        tokens = [int(x) for x in re.split(r'[,\s]+', line) if x]
                        events.append(tuple(tokens))
                    else:
                        events.append(int(line))
            
            # Format explicitly as a string representation of the Python list for eval() compatibility
            selection_expr = str(events)
            msg_log.info(f"Successfully parsed {len(events)} events from {event_list_file}")
        except Exception as e:
            raise TransformExecutionException(f"Failed to read/parse event list file '{event_list_file}': {str(e)}")

        # 7. Construct the execution command using the evaluated string expression literal
        self._cmd = [
            "acmd", "filter-files",
            "-s", selection_expr,
            "-o", str(output_file)
        ] + [str(f) for f in input_files]

        msg_log.info(f"Graph verification passed ({in_type}Files -> {out_type}_PICKED).")
        msg_log.info(f"Dynamically generated command: {' '.join(self._cmd)}")


if __name__ == '__main__':
    executor_set = set()
    trf = transform(executor=executor_set, description='Pick specific events from multiple ATLAS formats.')

    # Define parameters
    trf.parser.add_argument('--inputBSFiles', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='input', type='BS'), help='RAW/BS inputs')
    trf.parser.add_argument('--inputAODFiles', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input', type='AOD'), help='AOD inputs')
    trf.parser.add_argument('--inputDAODFiles', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input', type='DAOD'), help='DAOD inputs')
    trf.parser.add_argument('--inputESDFiles', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input', type='ESD'), help='ESD inputs')
    trf.parser.add_argument('--inputEVNTFiles', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input', type='EVNT'), help='EVNT inputs')
    trf.parser.add_argument('--inputHITSFiles', nargs='+', type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='input', type='HITS'), help='HITS inputs')

    trf.parser.add_argument('--outputBS_PICKEDFile', nargs=1, type=trfArgClasses.argFactory(trfArgClasses.argBSFile, io='output', type='BS_PICKED'), help='RAW/BS output')
    trf.parser.add_argument('--outputAOD_PICKEDFile', nargs=1, type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output', type='AOD_PICKED'), help='AOD output')
    trf.parser.add_argument('--outputDAOD_PICKEDFile', nargs=1, type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output', type='DAOD_PICKED'), help='DAOD output')
    trf.parser.add_argument('--outputESD_PICKEDFile', nargs=1, type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output', type='ESD_PICKED'), help='ESD output')
    trf.parser.add_argument('--outputEVNT_PICKEDFile', nargs=1, type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output', type='EVNT_PICKED'), help='EVNT output')
    trf.parser.add_argument('--outputHITS_PICKEDFile', nargs=1, type=trfArgClasses.argFactory(trfArgClasses.argPOOLFile, io='output', type='HITS_PICKED'), help='HITS output')

    trf.parser.add_argument('--eventList', nargs=1, type=trfArgClasses.argFactory(trfArgClasses.argFile, io='input'), required=True, help='Event List text file')

    acmd_executor = AcmdFilterExecutor()
    trf.appendToExecutorSet(acmd_executor)

    trf.parseCmdLineArgs(sys.argv[1:])
    trf.execute()
    trf.generateReport()
    sys.exit(trf.exitCode)
