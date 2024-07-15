#!/usr/bin/env python3

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
import logging
import argparse
from PyUtils.MetaReader import read_metadata
from typing import Dict, List

logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")


def filter_metadata(
    metadata: Dict[str, any], keys_prefixes: List[str]
) -> Dict[str, any]:
    """
    Filters a dictionary to include only keys that start with any of the specified prefixes.

    Args:
        metadata (dict): The dictionary to filter.
        keys_prefixes (list): A list of prefixes to include in the filtered dictionary.

    Returns:
        dict: The filtered dictionary.
    """
    filtered_metadata: Dict[str, any] = {}
    for key, value in metadata.items():
        for prefix in keys_prefixes:
            if key.startswith(prefix):
                filtered_metadata[prefix] = value
                break
    return filtered_metadata


def compare_metadata(
    ttree_file_path: str,
    rntuple_file_path: str,
    keys_to_compare: List[str],
    fmd_keys_to_compare: List[str],
) -> int:
    """
    Compares metadata from two files to check if they are the same.

    Args:
        ttree_file_path (str): The path to the first metadata file.
        rntuple_file_path (str): The path to the second metadata file.
        keys_to_compare (list): List of keys to compare in the metadata.
        fmd_keys_to_compare (list): List of keys to compare in the 'FileMetaData' section.

    Returns:
        int: 0 if metadata is the same, 1 if metadata is different.
    """
    try:
        # Read metadata from the ttree file
        ttree_metadata_full: Dict[str, any] = read_metadata(
            ttree_file_path, mode="full"
        )[ttree_file_path]
        ttree_metadata_peeker: Dict[str, any] = read_metadata(
            ttree_file_path, mode="peeker"
        )[ttree_file_path]

        # Read metadata from the rntuple file
        rntuple_metadata_full: Dict[str, any] = read_metadata(rntuple_file_path)[
            rntuple_file_path
        ]

    except KeyError as e:
        logging.error(f"Error accessing metadata for file: {e}")
        return 1

    # Filter metadata dictionaries based on the required keys
    ttree_event_format_metadata: Dict[str, any] = filter_metadata(
        ttree_metadata_full, ["EventFormat"]
    )
    ttree_peeker_metadata: Dict[str, any] = filter_metadata(
        ttree_metadata_peeker, keys_to_compare
    )
    rntuple_metadata: Dict[str, any] = filter_metadata(
        rntuple_metadata_full, keys_to_compare
    )
    rntuple_event_format_metadata: Dict[str, any] = filter_metadata(
        rntuple_metadata_full, ["EventFormat"]
    )

    # Further filter the 'FileMetaData' section of the metadata
    ttree_peeker_metadata["FileMetaData"] = filter_metadata(
        ttree_peeker_metadata.get("FileMetaData", {}), fmd_keys_to_compare
    )
    rntuple_metadata["FileMetaData"] = filter_metadata(
        rntuple_metadata.get("FileMetaData", {}), fmd_keys_to_compare
    )

    # Compare the filtered metadata
    if (
        ttree_peeker_metadata == rntuple_metadata
        and ttree_event_format_metadata == rntuple_event_format_metadata
    ):
        logging.info(
            f"Selected metadata keys ({keys_to_compare=}, {fmd_keys_to_compare=}) are the same"
        )
        return 0
    else:
        logging.info(
            f"Selected metadata keys ({keys_to_compare=}, {fmd_keys_to_compare=}) are different"
        )
        return 1


def main():
    parser = argparse.ArgumentParser(description="Compare metadata between two files.")
    parser.add_argument(
        "--ttree-file-path",
        type=str,
        required=True,
        help="Path to the TTree metadata file.",
    )
    parser.add_argument(
        "--rntuple-file-path",
        type=str,
        required=True,
        help="Path to the RNTuple metadata file.",
    )
    parser.add_argument(
        "--keys-to-compare",
        type=str,
        nargs="*",
        default=[],
        help="List of keys to compare in the metadata. Provide as space-separated values.",
    )
    parser.add_argument(
        "--fmd-keys-to-compare",
        type=str,
        nargs="*",
        default=[],
        help="List of keys to compare in the 'FileMetaData' section. Provide as space-separated values.",
    )

    args = parser.parse_args()

    sys.exit(
        compare_metadata(
            args.ttree_file_path,
            args.rntuple_file_path,
            args.keys_to_compare,
            args.fmd_keys_to_compare,
        )
    )


if __name__ == "__main__":
    main()
