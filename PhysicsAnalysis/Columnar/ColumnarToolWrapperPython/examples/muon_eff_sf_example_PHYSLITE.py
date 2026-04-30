# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# This follows the code Nils wrote in the ColumnarTests at:
# https://gitlab.cern.ch/atlas-asg/columnar-athena/-/blob/84feea5559c07a6a67233ab5465f90fb6f862509/PhysicsAnalysis/Columnar/ColumnarTests/test/gt_fullTools.cxx#L405-442
import ColumnarToolWrapperPython as ctw
import uproot as up
import sys


def main(path):
    print("# MuonEfficiencyScaleFactors")
    tool = ctw.Tool(
        "CP::MuonEfficiencyScaleFactors/unique0",
        properties={"WorkingPoint": "Tight"},
        rename_containers={
            "EventInfo": "EventInfoAuxDyn",
            "Muons": "AnalysisMuonsAuxDyn",
        },
    )

    print("columns:")
    for column in tool.columns:
        print(f" - {column}")

    print("recommended systematics:")
    for systematic in tool.recommended_systematics:
        print(f" - '{systematic}'", "(nominal)" if not systematic else "")

    with up.open(path) as fp:
        tree = fp["CollectionTree"]
        input_column_names = [c.name for c in tool.input_columns]
        events = tree.arrays(virtual=True, filter_name=lambda x: x in input_column_names)

    result = tool(events)
    print(result["AnalysisMuonsAuxDyn.sfOut"].to_list())
    print(result["AnalysisMuonsAuxDyn.validOut"].to_list())


if __name__ == "__main__":
    # Example path on UChicago: /data/krumnack/DAOD_PHYSLITE_DEV_V3.root
    main(sys.argv[1])
