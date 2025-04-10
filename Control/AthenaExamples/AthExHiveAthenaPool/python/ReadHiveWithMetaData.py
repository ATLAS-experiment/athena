# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from PyUtils import MetaReader


def is_metadata_content_expected(file_name):
    tested_metadata = MetaReader.read_metadata([file_name], mode="full")[file_name]

    expected_metadata = {
        "metadata_items": {
            "xAOD::FileMetaDataAuxInfo_v1_FileMetaDataAux.": "xAOD::FileMetaDataAuxInfo_v1",
            "xAOD::FileMetaData_v1_FileMetaData": "FileMetaData",
            "xAOD::EventFormat_v1_EventFormatStreamTestStream": "xAOD::EventFormat_v1",
            "xAOD::FileMetaDataAuxInfo_v1_FileMetaDataAuxDyn.mcProcID": "Float_t",
        },
        "FileMetaData": {
            "mcProcID": 0.0,
            "productionRelease": "",
            "dataType": "StreamTestStream",
            "runNumbers": [284600],
            "lumiBlocks": [0],
        },
        "xAOD::EventFormat_v1_EventFormatStreamTestStream": {
            b"C1": "HiveDataObj",
            b"V1": "HiveDataObj",
            b"V2": "HiveDataObj",
            b"V3": "HiveDataObj",
            b"a1": "HiveDataObj",
            b"a2": "HiveDataObj",
            b"b1": "HiveDataObj",
            b"c2": "HiveDataObj",
            b"d1": "HiveDataObj",
            b"e1": "HiveDataObj",
            b"g1": "HiveDataObj",
        },
        "nentries": 20,
    }

    return tested_metadata == expected_metadata


if __name__ == "__main__":
    import sys

    sys.exit(is_metadata_content_expected("TestStream.pool.root"))
