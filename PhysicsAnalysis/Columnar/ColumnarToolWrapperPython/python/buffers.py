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


# the suffixes of the key and index columns of key+index links
_LINK_KEY_SUFFIX = ".m_persKey"
_LINK_INDEX_SUFFIX = ".m_persIndex"


def _inner_most_list_offset_array(array):
    """Return the inner-most singly-jagged ListOffsetArray of ``array``.

    For a ``var * var * T`` input (e.g. NumTrkPt500), returns a view whose
    layout is a ``ListOffsetArray`` with ``NumpyArray`` content — i.e. the
    per-particle list structure collapsed across the full event range.
    Use ``result.layout.offsets.data`` for cumulative per-particle inner
    offsets and ``result.layout.content.data`` for the flat numeric buffer.

    Solution from Peter Fackeldey: traverse the layout with ``ak.transform``
    and capture the deepest singly-jagged node.
    """
    layout = array.layout
    layout_depth = layout.purelist_depth

    is_branched, _ = layout.branch_depth
    if is_branched:
        raise ValueError(
            f"{layout} has branching, cannot extract inner-most ListOffsetArray"
        )

    def _is_singly_jagged(lay):
        return (
            isinstance(lay, ak.contents.ListOffsetArray)
            and isinstance(lay.content, ak.contents.NumpyArray)
        )

    found = None

    def _capture(lay, depth, **_kwargs):
        nonlocal found
        if depth == (layout_depth - 1) and _is_singly_jagged(lay):
            found = lay.materialize()

    ak.transform(_capture, layout, return_value="none")

    if found is None:
        raise ValueError(
            "Did not find a singly-jagged ListOffsetArray at the inner-most depth"
        )
    return ak.Array(found)


def _branch_name_for_column(name):
    """Strip the trailing column suffix to get the branch name.

    That is ``.data`` for nested-vector columns, and ``.m_persKey`` or
    ``.m_persIndex`` for the key and index columns of links.
    """
    for suffix in (".data", _LINK_KEY_SUFFIX, _LINK_INDEX_SUFFIX):
        if name.endswith(suffix):
            return name[: -len(suffix)]
    return name


def _is_key_index_column(col):
    """Whether a data column is the key or index column of a key+index link."""
    return col.name.endswith((_LINK_KEY_SUFFIX, _LINK_INDEX_SUFFIX))


def _key_index_field(events, col):
    """Return the jagged array for a key or index column of a link."""
    base = _branch_name_for_column(col.name)
    index, key = _link_index_and_key(events, base, col.name)
    return key if col.name.endswith(_LINK_KEY_SUFFIX) else index


def _input_field(events, col):
    """Return the jagged array for an input column."""
    if _is_key_index_column(col):
        return _key_index_field(events, col)
    return events[_branch_name_for_column(col.name)]


def _key_index_buffers(events, col):
    """Extract the data buffer of a key or index column, without copying.

    Returns ``(nested offsets, data)``, with the nested offsets being
    ``None`` unless this is a link inside a vector. The field only gets
    packed if it actually needs it (e.g. after an event selection). The
    type of the data has to match what the tool expects, so it is not
    converted here.
    """
    layout = ak.to_packed(_key_index_field(events, col)).layout
    if not isinstance(layout, ak.contents.ListOffsetArray):
        raise RuntimeError(f"cannot handle layout for link column '{col.name}'")
    nested_offsets = None
    content = layout.content
    if isinstance(content, ak.contents.ListOffsetArray):
        nested_offsets = np.asarray(content.offsets.data).astype(np.uint64)
        content = content.content
    if not isinstance(content, ak.contents.NumpyArray):
        raise RuntimeError(f"cannot handle layout for link column '{col.name}'")
    data = np.asarray(content.data)
    if data.dtype != col.dtype:
        raise RuntimeError(
            f"link column '{col.name}' has type {data.dtype}, expected {col.dtype}"
        )
    return nested_offsets, data


def _container_offsets_from_fields(events, name):
    """Read the offset map of a container with no input columns from the input.

    Any field of the container in the input has the same per-event
    structure. If there is none, then the tool asked for an offset map
    that is not available, and that is an error.
    """
    prefix = f"{name}."
    for field in ak.fields(events):
        if field.startswith(prefix):
            return _offsets_from_counts(ak.to_numpy(ak.num(events[field], axis=1)))
    raise RuntimeError(
        f"tool requires the offset map of container '{name}', but the input "
        f"has no fields starting with '{prefix}' (read at least one branch of "
        "that container with uproot)"
    )


def _offsets_from_counts(counts):
    """Build a cumulative uint64 offset array from per-row counts."""
    offsets = np.zeros(len(counts) + 1, dtype=np.uint64)
    offsets[1:] = np.cumsum(counts)
    return offsets


def _link_index_and_key(events, base, column_name):
    """Return the (m_persIndex, m_persKey) jagged arrays for a link branch.

    PHYSLITE ElementLink branches read with uproot appear either as two
    top-level fields ``{base}.m_persIndex`` / ``{base}.m_persKey`` (when the
    sub-branches were requested directly) or as a single record-typed field
    ``{base}`` whose subfields are named either ``m_persIndex`` /
    ``m_persKey`` (vector-of-links branches) or with the full dotted prefix
    (scalar link parent branches).
    """
    fields = set(ak.fields(events))
    index_field = f"{base}.m_persIndex"
    key_field = f"{base}.m_persKey"
    if index_field in fields and key_field in fields:
        return events[index_field], events[key_field]
    if base in fields:
        subfields = set(ak.fields(events[base]))
        for prefix in ("", f"{base}."):
            if f"{prefix}m_persIndex" in subfields and f"{prefix}m_persKey" in subfields:
                return (
                    events[base][f"{prefix}m_persIndex"],
                    events[base][f"{prefix}m_persKey"],
                )
    raise RuntimeError(
        f"Cannot find link fields for column '{column_name}': expected "
        f"'{index_field}' and '{key_field}' fields in the input array "
        f"(read branch '{base}' with uproot)"
    )


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
        - ``"event_indices"``: list of event-index ColumnInfo for this
          container's offset map. These are generated rather than read from
          the input, so they are kept separate from ``"inputs"``. Nested
          offsets have the same list for their own offset map.

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
    # A nested-vector offset (e.g. "Particles.NumTrkPt500.offset") has a dotted
    # name and its offset_name points to a container; it is stored under that
    # container's "nested_offsets" as a dict with offset/inputs/outputs.
    container_offsets = {}
    nested_offsets_by_container = {}

    for name, col in offset_cols.items():
        parent = col.offset_name
        if parent == "" or parent in offset_cols:
            if "." in name:
                # Nested-vector offset — goes under its parent container
                nested_offsets_by_container.setdefault(parent, {})[name] = {
                    "offset": col,
                    "inputs": [],
                    "outputs": [],
                    "event_indices": [],
                }
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
            "event_indices": [],
        }
        for name, col in container_offsets.items()
    }

    # Reverse map: nested_offset_name -> parent container name, for routing
    # data columns whose offset_name points to a nested offset.
    nested_to_container = {
        nested_name: container
        for container, nested_map in nested_offsets_by_container.items()
        for nested_name in nested_map
    }

    # Assign data columns to their container or nested-offset bucket
    for col in data_cols:
        target = col.offset_name
        is_output = col.access_mode == ColumnAccessMode.output

        if target in classified:
            bucket = classified[target]
        elif target in nested_to_container:
            container = nested_to_container[target]
            bucket = classified[container]["nested_offsets"][target]
        else:
            # Shouldn't happen with well-formed tool output
            continue

        if col.is_event_index:
            bucket["event_indices"].append(col)
        elif is_output:
            bucket["outputs"].append(col)
        else:
            bucket["inputs"].append(col)

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
            if not col.is_optional or _column_is_present(col, available)
        ]
    return result


def _column_is_present(col, available):
    """Whether a column's branch fields are present in the input array.

    Link columns are stored as ``m_persKey``/``m_persIndex`` branch fields
    rather than under the column name itself.
    """
    if _is_key_index_column(col):
        base = _branch_name_for_column(col.name)
        return base in available or f"{base}.m_persIndex" in available
    return col.name in available


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
        nested_offsets = info["nested_offsets"]

        # Nested-vector inputs: extract inner offsets and data via the
        # inner-most ListOffsetArray helper before processing flat inputs.
        for nested_offset_name, nested in nested_offsets.items():
            for col in nested["inputs"]:
                if _is_key_index_column(col):
                    buffers[nested_offset_name], buffers[col.name] = (
                        _key_index_buffers(events, col)
                    )
                    continue
                base = _branch_name_for_column(col.name)
                inner = _inner_most_list_offset_array(events[base])
                raw_offsets = np.asarray(inner.layout.offsets.data)
                start = int(raw_offsets[0])
                end = int(raw_offsets[-1])
                # ROOT baskets can provide a larger raw buffer than the
                # offsets reference — normalize to [0, end-start] range.
                buffers[nested_offset_name] = np.ascontiguousarray(
                    raw_offsets - start, dtype=np.uint64
                )
                buffers[col.name] = np.ascontiguousarray(
                    inner.layout.content.data[start:end]
                )

        key_index_inputs = [col for col in info["inputs"] if _is_key_index_column(col)]
        for col in key_index_inputs:
            _, buffers[col.name] = _key_index_buffers(events, col)
        flat_inputs = [col for col in info["inputs"] if not _is_key_index_column(col)]

        if not flat_inputs:
            any_nested_input = next(
                (col for nested in nested_offsets.values() for col in nested["inputs"]),
                None,
            )
            if info["offset"].offset_name not in classified:
                # The event range itself is defined by the number of events, it
                # is the one offset map that doesn't get read from the input.
                buffers[container_name] = np.array([0, num_events], dtype=np.uint64)
            elif any_nested_input is not None:
                buffers[container_name] = _offsets_from_counts(
                    ak.to_numpy(ak.num(_input_field(events, any_nested_input), axis=1))
                )
            elif key_index_inputs:
                buffers[container_name] = _offsets_from_counts(
                    ak.to_numpy(ak.num(_key_index_field(events, key_index_inputs[0]), axis=1))
                )
            else:
                buffers[container_name] = _container_offsets_from_fields(
                    events, container_name
                )
            continue

        # Group flat input columns by their offset_name, then zip + to_buffers
        # each group. This is the same pattern as the original example script.
        sorted_cols = sorted(flat_inputs, key=lambda c: c.offset_name)

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
                buffers[container_name] = np.array(
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
                buffers[container_name] = np.asarray(
                    raw_buffers[offset_key]
                ).astype(np.uint64)

                # Data buffers from the inner RecordForm
                inner = form.content
                for field in inner.fields:
                    buffers[field] = np.asarray(
                        raw_buffers[f"{inner.content(field).form_key}-data"]
                    )
            else:
                raise RuntimeError(
                    f"Cannot handle form {type(form)} for "
                    f"container {container_name}"
                )

    _fill_event_index_columns(classified, buffers, num_events)

    return buffers


def _fill_event_index_columns(classified, buffers, num_events):
    """Fill the event-index columns, once all offsets are known.

    Each event-index column has one entry per entry of its offset map,
    holding the index of the event that entry belongs to. For a nested
    offset (per-object vector) that is the event of the parent object.
    """
    container_event_index = {}

    def event_index_for_container(name):
        if name not in container_event_index:
            parent = classified[name]["offset"].offset_name
            if parent not in classified:
                # the root container (EventInfo), one entry per event
                result = np.arange(num_events, dtype=np.uint64)
            else:
                result = _repeat_for_offsets(
                    event_index_for_container(parent), buffers[name], name
                )
            container_event_index[name] = result
        return container_event_index[name]

    for container_name, info in classified.items():
        if info["event_indices"]:
            event_index = event_index_for_container(container_name)
            for col in info["event_indices"]:
                buffers[col.name] = event_index
        for nested_offset_name, nested in info["nested_offsets"].items():
            if not nested["event_indices"]:
                continue
            event_index = _repeat_for_offsets(
                event_index_for_container(container_name),
                buffers[nested_offset_name],
                nested_offset_name,
            )
            for col in nested["event_indices"]:
                buffers[col.name] = event_index


def _repeat_for_offsets(parent_values, offsets, offset_name):
    """Repeat one value per parent entry for each entry in its offset range."""
    counts = np.diff(np.asarray(offsets).astype(np.int64))
    if len(counts) != len(parent_values):
        raise RuntimeError(
            f"offset column '{offset_name}' has {len(counts)} entries, "
            f"but its parent has {len(parent_values)}"
        )
    return np.ascontiguousarray(np.repeat(parent_values, counts), dtype=np.uint64)


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
    nested_offset_names = {
        nested_name
        for info in classified.values()
        for nested_name in info["nested_offsets"]
    }
    output_buffers = {}
    for _container_name, info in classified.items():
        for col in info["outputs"]:
            if col.offset_name in nested_offset_names:
                raise NotImplementedError(
                    f"Nested-vector output columns are not supported "
                    f"(column '{col.name}' has nested offset '{col.offset_name}')"
                )
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
