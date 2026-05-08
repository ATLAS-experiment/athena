# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Buffer extraction and reconstruction utilities for PythonToolHandle.
# Generalizes the pattern from test/muon_eff_sf_example_PHYSLITE.py into
# reusable functions that work with any columnar CP tool.

import itertools

import awkward as ak
import numpy as np

from ColumnarToolWrapperPython.python_tool_handle import ColumnAccessMode


def classify_columns(columns):
    """Group ColumnInfo objects by container and role.

    Parameters
    ----------
    columns:
        Iterable of ColumnInfo objects as returned by PythonToolHandle.columns.

    Returns
    -------
    dict
        Keyed by container offset name (e.g. "EventInfo", "Muons"). Each value
        is a dict with:

        - ``"offset"``: the ColumnInfo for this container's offset column
        - ``"inputs"``: list of input ColumnInfo belonging to this container
        - ``"outputs"``: list of output ColumnInfo belonging to this container
        - ``"nested_offsets"``: dict of name -> ColumnInfo for offset columns
          that are children of this container (e.g. "Muons.NumTrkPt500.offset")

    Notes
    -----
    Container offsets have ``is_offset=True`` and an ``offset_name`` of either
    ``''`` (root, e.g. "EventInfo") or the name of another container offset
    (e.g. "Muons" has ``offset_name="EventInfo"``). Nested-vector offsets also
    have ``is_offset=True`` but their name contains a dot; they are stored under
    ``"nested_offsets"`` of their parent container rather than as top-level keys.
    """
    # Separate offset columns from data columns
    offset_cols = {col.name: col for col in columns if col.is_offset}
    data_cols = [col for col in columns if not col.is_offset]

    # Determine which offset columns are "container" offsets vs nested-vector
    # offsets. A container offset is one whose offset_name is either '' (root)
    # or points to another container offset. For MuonEffSF: EventInfo
    # (offset_name='') and Muons (offset_name='EventInfo') are both containers.
    # A nested-vector offset would be something like
    # "Muons.NumTrkPt500.offset" (offset_name='Muons'), which belongs under
    # the "Muons" container entry.
    #
    # Detection: a nested-vector offset has offset_name pointing to a
    # container offset AND its name is not a plain container name (contains '.').
    container_offsets = {}
    nested_offsets_by_container = {}

    for name, col in offset_cols.items():
        parent = col.offset_name
        if parent == "" or parent in offset_cols:
            # This could be a container or a nested-vector offset. Distinguish
            # by checking whether the name contains a dot (nested) or not.
            if "." in name:
                # Nested-vector offset — goes under its parent container
                container = parent
                nested_offsets_by_container.setdefault(container, {})[name] = col
            else:
                container_offsets[name] = col
        else:
            # offset_name not found in any offset column — treat as root
            container_offsets[name] = col

    # Build the classified dict
    classified = {
        name: {
            "offset": col,
            "inputs": [],
            "outputs": [],
            "nested_offsets": nested_offsets_by_container.get(name, {}),
        }
        for name, col in container_offsets.items()
    }

    # Assign data columns to their container
    for col in data_cols:
        container = col.offset_name
        if container not in classified:
            # Shouldn't happen with well-formed tool output
            continue
        if col.access_mode == ColumnAccessMode.output:
            classified[container]["outputs"].append(col)
        else:
            classified[container]["inputs"].append(col)

    return classified


def resolve_optional_columns(classified, events):
    """Remove optional input columns absent from events.

    Checks each optional input column against ``ak.fields(events)`` and drops
    it if not present. Returns a new dict (shallow copy per container); the
    original classified dict is not modified.

    Parameters
    ----------
    classified:
        Output of ``classify_columns``.
    events:
        An ak.Array whose fields are checked for optional column presence.

    Returns
    -------
    dict
        Same structure as ``classify_columns`` output, with absent optional
        columns removed from each container's ``"inputs"`` list.
    """
    available = set(ak.fields(events))
    result = {}
    for container, info in classified.items():
        result[container] = dict(info)
        result[container]["inputs"] = [
            col
            for col in info["inputs"]
            if not col.is_optional or col.name in available
        ]
    return result


def extract_buffers(events, classified):
    """Extract flat numpy buffers from an awkward array.

    Returns a dict mapping column name -> numpy array, covering all container
    offsets, nested-vector offsets, and input data columns. Output column
    buffers are not included (allocate_outputs handles those).

    Parameters
    ----------
    events:
        An ak.Array (real or zero-length after typetracer conversion).
    classified:
        Output of classify_columns or resolve_optional_columns.
    """
    buffers = {}
    num_events = int(ak.num(events, axis=0))

    for container_name, info in classified.items():
        input_cols = info["inputs"]

        if not input_cols:
            # No inputs — synthesize an offset if outputs need it later
            buffers[container_name] = np.ascontiguousarray(
                [0, num_events], dtype=np.uint64
            )
            continue

        # Group input columns by their offset_name, then zip + to_buffers
        # each group. This is the same pattern as the original example script.
        sorted_cols = sorted(input_cols, key=lambda c: c.offset_name)

        for offset_name, cols_iter in itertools.groupby(
            sorted_cols, key=lambda c: c.offset_name
        ):
            cols = list(cols_iter)
            unzipped = {col.name: events[col.name] for col in cols}
            zipped = ak.zip(unzipped)

            # NB: form_key not crucial, but helpful for debugging
            form, length, raw_buffers = ak.to_buffers(
                zipped, form_key=f"{offset_name}{{id}}"
            )

            if isinstance(form, ak.forms.RecordForm):
                # EventInfo-like: one record per event.
                # Use ak.to_numpy per field instead of walking raw_buffers:
                # when events is masked/indexed, form.content(field) is an
                # IndexedForm and the "-data" key doesn't exist in raw_buffers.
                buffers[container_name] = np.ascontiguousarray(
                    [0, length], dtype=np.uint64
                )
                for col in cols:
                    buffers[col.name] = ak.to_numpy(events[col.name])
            elif isinstance(form, ak.forms.ListOffsetForm):
                # Particle container: extract offsets, cast to uint64
                # for the C++ side
                offset_key = next(
                    key for key in raw_buffers if key.endswith("-offsets")
                )
                buffers[container_name] = np.ascontiguousarray(raw_buffers[offset_key]).astype(
                    np.uint64
                )

                # Data buffers from the inner RecordForm
                inner = form.content
                for field in inner.fields:
                    buffers[field] = np.ascontiguousarray(
                        raw_buffers[f"{inner.content(field).form_key}-data"]
                    )
            else:
                raise RuntimeError(
                    f"Cannot handle form {type(form)} for container {container_name}"
                )

    # TODO: nested vector offset extraction not yet implemented

    return buffers


def allocate_outputs(classified, buffer_dict):
    """Allocate zero-filled numpy arrays for each output column.

    Sizes each output array using ``offsets[-1]`` of the referenced offset
    buffer. Arrays are added into ``buffer_dict`` in-place and also returned.

    Parameters
    ----------
    classified:
        Output of ``classify_columns`` or ``resolve_optional_columns``.
    buffer_dict:
        Dict of column name -> numpy array, as returned by ``extract_buffers``.
        Modified in-place to include the newly allocated output arrays.

    Returns
    -------
    dict
        Mapping of output column name -> zero-filled numpy array (same objects
        also inserted into ``buffer_dict``).
    """
    output_buffers = {}
    for _container_name, info in classified.items():
        for col in info["outputs"]:
            offset_data = buffer_dict.get(col.offset_name)
            if offset_data is None:
                msg = (
                    f"Cannot find offset buffer '{col.offset_name}' "
                    f"needed for output column '{col.name}'"
                )
                raise RuntimeError(msg)
            size = int(offset_data[-1])
            arr = np.zeros(size, dtype=col.dtype)
            output_buffers[col.name] = arr
            buffer_dict[col.name] = arr
    return output_buffers


def reconstruct_output(classified, buffer_dict, num_events):
    """Build an awkward array from output column buffers.

    Parameters
    ----------
    classified:
        Output of ``classify_columns`` or ``resolve_optional_columns``.
    buffer_dict:
        Dict of column name -> numpy array, containing both offset buffers and
        the output arrays populated by ``allocate_outputs`` and ``call()``.
    num_events:
        Number of events (outer axis length of the returned array).

    Returns
    -------
    ak.Array
        Record array with one field per output column, each a variable-length
        list of per-particle values (i.e. ``var * dtype``).
    """
    form_fields = []
    form_contents = []
    out_buffers = {}

    # node0 = RecordArray; each output column needs a pair of nodes
    node_index = 1
    for _container_name, info in classified.items():
        for col in info["outputs"]:
            node_offset = f"node{2 * node_index}"
            node_data = f"node{2 * node_index + 1}"
            node_index += 1

            form_fields.append(col.name)
            form_contents.append(
                {
                    "class": "ListOffsetArray",
                    "offsets": "i64",
                    "content": {
                        "class": "NumpyArray",
                        "primitive": col.dtype,
                        "form_key": node_data,
                    },
                    "form_key": node_offset,
                }
            )

            out_buffers[f"{node_data}-data"] = buffer_dict[col.name]
            out_buffers[f"{node_offset}-offsets"] = buffer_dict[col.offset_name]

    form = {
        "class": "RecordArray",
        "fields": form_fields,
        "contents": form_contents,
        "form_key": "node0",
    }

    return ak.from_buffers(form, num_events, out_buffers)
