#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# CopyBlobFromCrest.py
"""
@author Laura.Sargsyan@cern.ch 2025-04-16
CopyBlobFromCrest.py - Tool for retrieving TileCal calibration data from CREST

Description:
------------
This script connects to the CREST service to fetch calibration data (blobs) for TileCal. Users can specify which calibration tag, run number, luminosity
block, and channels to retrieve for fine-grained control of the data. The fetched calibration data is then saved into a JSON file whose name typically
includes metadata such as the tag, run, and luminosity block.

Command-line Parameters:
-----------------------
-s, --server
    CREST server URL. Default: http://crest-j23.cern.ch:8080/api-v5.0

-t, --tag
    Calibration tag to fetch from CREST (e.g., TileOfl02CalibCes-RUN2-UPD4-29). Default: TileOfl02CalibCes-RUN2-UPD4-29)

-r, --run
    The run number.  Default: 2147483647 (retrieves the latest IOV).

-l, --lumi
    Luminosity block number.  Default: 0

-c, --channel
    Space or comma-separated list of COOL channel IDs to retrieve (e.g., 0,1,2). Default: [0..275] plus channel 1000.

-o, --output
    Output filename (JSON). If omitted, a default is generated, typically
    structured as: <tag>.<run>.<lumi>.json. For example: UPD4.2147483647.0.json

Usage Example:
--------------
python CopyBlobFromCrest.py --tag TileOfl02CalibCes-RUN2-UPD4-29 --channel 0,1,2 --output fetched_data.json

What the Script Does:
---------------------
1. Connects to the specified CREST service (-s/--server).
2. Fetches calibration data (blobs) for the given tag, run, lumi block,
   and channels.
3. Saves the data into a JSON file with a filename either provided via
   --output or auto-generated from the tag, run, and lumi.

The output JSON file stores the calibration data keyed by the channel IDs.
Each channel’s data includes the calibration blob(s) retrieved from CREST.
"""

import json
from TileCalibBlobPython import TileCalibCrest
from TileCalibBlobPython.TileCalibLogger import getLogger
import argparse
import logging

# Logger setup (reusing the framework's logger)
log = getLogger("CopyBlobFromCrest")
logLevel = logging.DEBUG
log.setLevel(logLevel)

log1 = getLogger("TileCalibTools")
log1.setLevel(logLevel)
schema = 'CREST'

def fetch_calib_data(server, tag, run, lumi, channels):
    """
    Fetch calibration data from TileBlobReaderCrest.

    Args:
        server (str): CREST server URL.
        tag (str): Tag identifying the calibration data.
        run (int): Run number to fetch data for.
        lumi (int): Lumi block to fetch data for.
        channels (list of int): List of COOL channel numbers.

    Returns:
        list: List of calibration data entries retrieved.
    """
    try:
        log.info("Initializing TileBlobReaderCrest...")
        folderTag = tag
        if folderTag.startswith("Tile") or folderTag.startswith("CALO"):
           folderPath = ""

        # Create the TileBlobReaderCrest object and pass the relevant arguments
        blob_reader = TileCalibCrest.TileBlobReaderCrest(schema, folderPath, tag, run, lumi,copyBlob=True)
        # blob_reader = TileCalibCrest.TileBlobReaderCrest(server, tag, run, lumi, min(channels), max(channels))


        blobs = blob_reader.payload  # Retrieve the calibration data
        log.info(f"Successfully fetched {len(blobs)} calibration entries.")
        return blobs

    except Exception as e:
        log.error(f"Error while retrieving calibration data: {e}")
        raise

def process_channel_list(channel_list):
    """
    Process a list of channel identifiers, handling comma-separated values.

    Args: channel_list (list): List of channel identifiers, which may contain comma-separated value
    Returns: list: Processed list of individual channel identifiers
    """
    processed_channels = []
    if channel_list:
        for item in channel_list:
            # If an item contains commas, split it and add each part
            if isinstance(item, str) and ',' in item:
                parts = [part.strip() for part in item.split(',')]
                processed_channels.extend(parts)
            else:
                processed_channels.append(str(item))

    return processed_channels


def output_filename(args):
    if not args.output:
        # Format run and lumi as strings to ensure they're properly handled
        run_str = str(args.run)
        lumi_str = str(args.lumi)

        # Construct filename from components
        tag = args.tag if args.tag else "default"

        # Add channel numbers in name if they were defined in args
        if hasattr(args, 'channel') and args.channel:
            # Process the channel list using the utility function
            processed_channels = args.channel[0].split(',')
            channel_str = '-'.join(processed_channels)
            return f"{tag}.{run_str}.{lumi_str}.ch{channel_str}.json"
        else:
            # Return filename without channel information if default value is used
            return f"{tag}.{run_str}.{lumi_str}.json"

    return args.output

def save_calib_data_to_file(blobs, output_file, requested_channels=None):
    """
    Save fetched calibration data to a file.
    Parameters:
    -----------
    blobs : dict containing calibration data blobs
    output_file : str Path to the output file where data will be saved
    requested_channels : list, optional
        List of specific channels to include in the output file.
        If None, all channels from the blobs will be included.
    """
    try:
        log.info(f"Saving calibration data to {output_file}")

        if requested_channels is not None:
            # Process the channel list using the utility function
            processed_channels = set(process_channel_list(requested_channels))

            # Filter blobs to only include requested channels
            filtered_blobs = {channel: data for channel, data in blobs.items()
                              if channel in processed_channels}

            blobs_to_save = filtered_blobs
        else:
            # Save all blobs if no channel filtering is requested
            blobs_to_save = blobs

        with open(output_file, "w") as f:
            json.dump(blobs_to_save, f)

        log.info("Calibration data successfully written to output file.")

    except Exception as e:
        log.error(f"Error while saving calibration data: {e}")
        raise
def setup_argparser():
    """
    Set up command-line argument parser.

    Returns:
        argparse.ArgumentParser: Configured argument parser
    """
    parser = argparse.ArgumentParser(description="Read TileCal blobs from CREST and convert them to JSON format.")

    parser = argparse.ArgumentParser(description="Read TileCal blobs from CREST and convert them to JSON format.")
    parser.add_argument(
        "-s", "--server",
        default="http://crest-j23.cern.ch:8080/api-v5.0",
        help="Specify CREST server URL (default is %(default)s)."
    )

    parser.add_argument(
        "-t", "--tag",
        default='TileOfl02CalibCes-RUN2-UPD4-29',
        help="Specify the tag to use (e.g., RUN2-UPD4-04). Default is %(default)s."
    )
    parser.add_argument(
        "-r", "--run",
        type=int,
        default=2147483647,  # Largest IOV value (default for latest run)
        help="Specify the run number (default is the latest IOV, %(default)s)."
    )
    parser.add_argument(
        "-l", "--lumi",
        type=int,
        default=0,
        help="Specify the lumi block number (default is %(default)s)."
    )
    parser.add_argument(
        "-c", "--channel",
        type=str,  # Changed to str to accept comma values
        nargs="+",
        help="Specify COOL channels (default includes all channels from 0-275 and 1000). Use space or comma-separated values."
    )

    parser.add_argument(
        "-o", "--output",
        default=None,
        help="Specify the output JSON file for saving calibration data. If not provided, will use '[tag].[run].[lumi].json'"
    )

    # Parse arguments from the command line
    args = parser.parse_args()

    return args

if __name__ == "__main__":

    args = setup_argparser()
    try:
        log.info("Starting the calibration data fetching process.")

        # Fetch calibration data using provided CLI arguments
        payload_data = fetch_calib_data(
            server=args.server,
            tag=args.tag,
            run=args.run,
            lumi=args.lumi,
            channels=args.channel,
        )

        # Save fetched calibration data to the output file
        ofile = output_filename(args)
        save_calib_data_to_file(payload_data, ofile, args.channel)

        log.info(f"Finished fetching and saving calibration data in {ofile} successfully.")
    except Exception as ex:
        log.error(f"An error occurred during execution: {ex}")
