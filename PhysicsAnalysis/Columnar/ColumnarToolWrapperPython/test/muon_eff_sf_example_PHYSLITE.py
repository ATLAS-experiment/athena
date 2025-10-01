# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# This follows the code Nils wrote in the ColumnarTests at:
# https://gitlab.cern.ch/atlas-asg/columnar-athena/-/blob/84feea5559c07a6a67233ab5465f90fb6f862509/PhysicsAnalysis/Columnar/ColumnarTests/test/gt_fullTools.cxx#L405-442
import python_tool_handle
import numpy as np
import uproot as up
import awkward as ak
import sys
import itertools


def main(path):
    print("# MuonEfficiencyScaleFactors")
    muon_eff_sf_tool_handle = python_tool_handle.PythonToolHandle()

    # Call set_type_and_name with a string
    muon_eff_sf_tool_handle.set_type_and_name("CP::MuonEfficiencyScaleFactors/unique0")
    # can use .set_property() or __setattr__
    muon_eff_sf_tool_handle.WorkingPoint = "Tight"

    muon_eff_sf_tool_handle.initialize()

    # needed at the uproot-level, not at the coffea-level
    muon_eff_sf_tool_handle.rename_containers(
        dict(
            [
                ("EventInfo", "EventInfoAuxDyn"),
                ("Muons", "AnalysisMuonsAuxDyn"),
            ]
        )
    )

    print("columns:")
    for column in muon_eff_sf_tool_handle.columns:
        print(f" - {column}")

    print("recommended systematics:")
    for systematic in muon_eff_sf_tool_handle.get_recommended_systematics():
        print(f" - '{systematic}'", "(nominal)" if not systematic else "")

    # pre-processing needed to determine inputs and outputs
    column_data = {}
    input_columns = []
    output_columns = []
    for column in muon_eff_sf_tool_handle.columns:
        column_data[column.name] = {"info": column, "data": None}
        if column.is_offset:
            continue
        if column.access_mode == python_tool_handle.ColumnAccessMode.input:
            input_columns.append(column)
        if column.access_mode == python_tool_handle.ColumnAccessMode.output:
            output_columns.append(column)

    with up.open(path) as fp:
        # note: only need to remove columns for uproot, at the dask-level one uses `ak.fields(events)`
        tree = fp["CollectionTree"]
        remove_columns = set([c.name for c in input_columns]) - set(tree.keys())
        column_data = {k: v for k, v in column_data.items() if k not in remove_columns}
        input_columns = [
            column for column in input_columns if column.name not in remove_columns
        ]

        events = tree.arrays([column.name for column in input_columns])

        # group input/output columns by their offset
        for offset_name, columns in itertools.groupby(
            (
                col
                for col in sorted(
                    input_columns + output_columns, key=lambda col: col.offset_name
                )
            ),
            key=lambda col: col.offset_name,
        ):
            unzipped_data = {
                column.name: events[column.name]
                for column in columns
                if column in input_columns
            }
            zipped_data = ak.zip(unzipped_data)

            # NB: form_key not crucial, but helpful to include for debugging
            form, length, buffers = ak.to_buffers(
                zipped_data, form_key=f"{offset_name}{{id}}"
            )

            offsets = None

            def get_form_key(form, field):
                return RuntimeError("not implemented")

            if isinstance(form, ak.forms.RecordForm):
                # once per event, likely EventInfo-like
                # special case of EventInfo being (0, nevents)
                offsets = np.array([0, length], dtype=np.uint64)
                get_form_key = lambda form, field: form.content(field).form_key
            elif isinstance(form, ak.forms.ListOffsetForm):
                offset_key = next(key for key in buffers if key.endswith("-offsets"))
                offsets = buffers[offset_key]
                get_form_key = lambda form, field: form.content.content(field).form_key
            else:
                msg = f"Cannot handle form {type(form)}"
                RuntimeError(msg)

            # setting offset
            column_data[offset_name]["data"] = offsets.astype(np.uint64)
            # setting input columns
            for field in form.fields:
                column_data[field]["data"] = buffers[
                    f"{get_form_key(form, field)}-data"
                ]
            # creating output columns
            for column in output_columns:
                if column.offset_name != offset_name:
                    continue
                # TODO: better way to grab an "adjacent" field with same offset_name?
                column_data[column.name]["data"] = np.zeros_like(
                    column_data[form.fields[0]]["data"], dtype=column.dtype
                )

        for column_name, column in column_data.items():
            if column["data"] is None:
                msg = f"This is unexpected, but {column_name} has empty data."
                raise RuntimeError(msg)
            if column["info"] in output_columns:
                # output columns are mutable
                muon_eff_sf_tool_handle.set_column_void(
                    column_name, column["data"], False
                )
            else:
                # set immutable column using __setitem__
                muon_eff_sf_tool_handle[column_name] = column["data"]

        muon_eff_sf_tool_handle.call()

        # build the output (form=structure and buffers=data)
        form = {
            "class": "RecordArray",
            "fields": [],
            "contents": [],
            "form_key": "node0",
        }
        # TODO: check works ok in dask-awkward? maybe len(events) is fine
        length = int(ak.num(events, axis=0))
        buffers = {}

        # start at node1, each column needs two nodes (offset/data)
        for index, column in enumerate(output_columns, start=1):
            node_offset = f"node{2 * index}"
            node_data = f"node{2 * index + 1}"

            form["fields"].append(column.name)
            form["contents"].append(
                {
                    "class": "ListOffsetArray",
                    "offsets": "i64",
                    "content": {
                        "class": "NumpyArray",
                        "primitive": column.dtype,
                        "form_key": node_data,
                    },
                    "form_key": node_offset,
                }
            )

            buffers[f"{node_data}-data"] = column_data[column.name]["data"]
            buffers[f"{node_offset}-offsets"] = column_data[column.offset_name]["data"]

        result = ak.from_buffers(form, length, buffers)

        print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
        print(result["AnalysisMuonsAuxDyn.validOut"].to_list())


if __name__ == "__main__":
    # Example path on UChicago: /data/krumnack/DAOD_PHYSLITE_DEV_V3.root
    main(sys.argv[1])
