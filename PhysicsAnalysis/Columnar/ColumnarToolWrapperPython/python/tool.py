# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# High-level Tool wrapper around PythonToolHandle.
# Automates buffer extraction, column setting, calling, and output
# reconstruction using ColumnInfo metadata.

import awkward as ak
import numpy as np

from ColumnarToolWrapperPython.buffers import (
    allocate_outputs,
    classify_columns,
    extract_buffers,
    reconstruct_output,
    resolve_optional_columns,
)
from ColumnarToolWrapperPython.python_tool_handle import PythonToolHandle


class Tool:
    """High-level wrapper around PythonToolHandle.

    Handles the boilerplate of classifying columns, extracting awkward-array
    buffers, allocating outputs, setting columns on the handle, calling the
    tool, and reconstructing the output awkward array.

    Parameters
    ----------
    type_and_name:
        Tool type and instance name, e.g. "CP::MuonEfficiencyScaleFactors/myTool".
    properties:
        Optional dict of tool properties to set before initialization.
    rename_containers:
        Optional mapping from canonical container names (e.g. "Muons") to the
        branch-name prefix used in the input arrays (e.g. "AnalysisMuonsAuxDyn").
        This is passed to PythonToolHandle.rename_containers *after* initialize
        so that ColumnInfo.name values match input array fields.
    """

    def __init__(self, type_and_name, properties=None, rename_containers=None):
        self._handle = PythonToolHandle()
        self._handle.set_type_and_name(type_and_name)
        self._properties = dict(properties) if properties else {}

        for key, value in self._properties.items():
            self._handle.set_property(key, value)

        self._handle.initialize()

        if rename_containers:
            self._handle.rename_containers(rename_containers)

        # Cache classified column topology once after initialization
        self._classified = classify_columns(self._handle.columns)

    @property
    def properties(self):
        """Dict of properties set on this tool at construction time."""
        return self._properties

    @property
    def columns(self):
        """All ColumnInfo objects reported by the tool."""
        return self._handle.columns

    @property
    def input_columns(self):
        """Input (non-offset) ColumnInfo objects."""
        cols = []
        for info in self._classified.values():
            cols.extend(info["inputs"])
        return cols

    @property
    def output_columns(self):
        """Output ColumnInfo objects."""
        cols = []
        for info in self._classified.values():
            cols.extend(info["outputs"])
        return cols

    @property
    def recommended_systematics(self):
        """List of recommended systematic variation names."""
        return self._handle.get_recommended_systematics()

    def apply_systematic_variation(self, sys_name):
        """Apply a systematic variation by name.

        Parameters
        ----------
        sys_name:
            Systematic variation name, e.g. ``"MUON_EFF_RECO_SYS__1up"``.
            Pass ``""`` to reset to the nominal.
        """
        self._handle.apply_systematic_variation(sys_name)

    def __call__(self, events, systematic=None):
        """Run the tool on events and return output columns as an ak.Array.

        Parameters
        ----------
        events:
            An ak.Array with fields matching the tool's input column names
            (after any rename_containers mapping).
        systematic:
            Optional systematic variation name (e.g. "MUON_EFF_RECO_SYS__1up").
            If provided, applied before running the tool and reset to nominal
            after execution.

        Returns
        -------
        ak.Array
            Record array with one field per output column, each a
            variable-length list over the per-particle values.
        """
        if systematic is not None:
            self.apply_systematic_variation(systematic)

        try:
            num_events = int(ak.num(events, axis=0))

            # Resolve optional columns against the actual fields present
            effective = resolve_optional_columns(self._classified, events)

            # Extract flat buffers from the awkward array
            buffer_dict = extract_buffers(events, effective)

            # Allocate zero-filled output arrays (added into buffer_dict in-place)
            allocate_outputs(effective, buffer_dict)

            # Set all columns on the handle
            for container_name, info in effective.items():
                # Container offset (always immutable)
                self._handle[container_name] = np.asarray(buffer_dict[container_name])

                # Nested-vector offsets (immutable)
                for nested_name in info["nested_offsets"]:
                    if nested_name in buffer_dict:
                        self._handle[nested_name] = np.asarray(buffer_dict[nested_name])

                # Input data columns (immutable)
                for col in info["inputs"]:
                    self._handle[col.name] = np.asarray(buffer_dict[col.name])

                # Output data columns (mutable)
                for col in info["outputs"]:
                    self._handle.set_column_void(col.name, buffer_dict[col.name], False)

            self._handle.call()

            return reconstruct_output(effective, buffer_dict, num_events)
        finally:
            # Reset to nominal if systematic was applied
            if systematic is not None:
                self.apply_systematic_variation("")
