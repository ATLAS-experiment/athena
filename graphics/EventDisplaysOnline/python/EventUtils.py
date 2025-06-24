# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import os
import re
import time
import glob
import shutil
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from AthenaCommon.Logging import logging

"""
Event file cleanup and transfer preparation utility for JiveXML and VP1 outputs.
Handles pairing, validation, pruning, and transfer staging of event files
based on run and event numbers, with special support for beam splash events.
"""

def cleanDirectory(directory, max_pairs, check_pair, is_beam_splash_mode):

    """
    Main routine for managing cleanup and preparation of JiveXML/VP1 event files.

    Parameters:
    - directory (str): Path where event files are stored.
    - max_pairs (int): Maximum number of event pairs to keep in the directory.
    - check_pair (bool): If True, remove unpaired files before pruning.
    - is_beam_splash_mode (bool): If True, disables pruning and pairing to preserve rare beam splash data.
    """

    msg = logging.getLogger('EventUtils')
    msg.info('%s: Starting to clean directory %s', time.ctime(time.time()), directory)

    """
    We want to transfer everything for beam splash events are they are rare,
    so we don't want to miss them
    """

    if is_beam_splash_mode:
        check_pair = False
    try:
        file_pairs = getEventlist(directory)
        if check_pair:
            file_pairs = checkPairs(file_pairs, directory)
        if not is_beam_splash_mode:
            prune(file_pairs, max_pairs,directory)
            writeEventlist(directory,file_pairs)
            prepareFilesForTransfer(directory, file_pairs, timeinterval=60)
        else:
            prepareALLFilesForTransfer(directory, file_pairs)
    except Exception as e:
        msg.error('Error occurred while cleaning directory %s: %s', directory, str(e))

    msg.info('%s: Finished cleaning directory %s', time.ctime(time.time()), directory)


def getEventlist(directory):
    """
    Retrieves a list of paired files (JiveXML and VP1) from the specified directory.
    """
    msg = logging.getLogger('EventUtils')
    msg.info('%s: Starting to get event list from directory %s', time.ctime(time.time()), directory)

    # Find all relevant files in the directory
    jive_files = [os.path.basename(f) for f in glob.glob(f"{directory}/JiveXML*.xml")]
    vp1_files = [os.path.basename(f) for f in glob.glob(f"{directory}/vp1*CEST.pool.root")]

    # Compile regex patterns for matching run and event numbers in filenames
    vp1_pattern = re.compile(r'vp1_r(\d+)_ev(\d+)_')
    jive_pattern = re.compile(r'JiveXML_(\d+)_(\d+)\.xml')

    # Dictionary to store VP1 files by (run, event) tuple
    vp1_dict = {}
    for vp1_file in vp1_files:
        match = vp1_pattern.search(vp1_file)
        if match:
            run, event = match.groups()
            vp1_dict[(run, event)] = vp1_file

    # Generate file pairs
    file_pairs = []
    for jive_file in jive_files:
        match = jive_pattern.search(jive_file)
        if match:
            run, event = match.groups()
            # Get corresponding VP1 file
            vp1_file = vp1_dict.get((run, event))
            file_pairs.append((jive_file, vp1_file))
        else:
            # Add jive file without a matching VP1 file
            file_pairs.append((jive_file, None))

    # Add any VP1 files that don't have a matching JiveXML file
    for vp1_file in vp1_files:
        if not any(vp1_file in pair for pair in file_pairs):
            file_pairs.append((None, vp1_file))

    # Sort file pairs by last modified time (newest last)
    file_pairs.sort(key=lambda pair: (
        max(os.path.getmtime(os.path.join(directory, f)) if f else 0 for f in pair)
    ), reverse=False)

    msg.info('%s: Event list retrieved with %d pairs', time.ctime(time.time()), len(file_pairs))

    return file_pairs

def checkPairs(file_pairs, directory):
    """
    Ensures only valid JiveXML-VP1 pairs remain in the list, removing unmatched files.
    """
    msg = logging.getLogger('EventUtils')
    updated_file_pairs = []

    for jive_file, vp1_file in file_pairs[:-1]:  # Ignore last entry
        if jive_file and vp1_file:
            updated_file_pairs.append((jive_file, vp1_file))
        else:
            file_to_remove = jive_file or vp1_file
            if file_to_remove:
                file_path = os.path.join(directory, file_to_remove)
                remove_file(file_path)
                msg.info('Removed unmatched file: %s', file_path)

    return updated_file_pairs

def prune(file_pairs, max_pairs, directory):
    """
    Removes the oldest event pairs to keep only the latest `max_pairs`.

    Parameters:
    - file_pairs (list): List of (JiveXML, VP1) file pairs sorted by timestamp.
    - max_pairs (int): Maximum number of file pairs to retain.
    - directory (str): Directory where files are located.

    Returns:
    - list: The pruned list of `max_pairs` most recent file pairs.
    """

    if len(file_pairs) <= max_pairs:
        return []  # No files removed

    msg = logging.getLogger('EventUtils')
    msg.info('Pruning file list: Keeping latest %d out of %d entries.', max_pairs, len(file_pairs))

    # Determine files to remove (oldest entries)
    removed_pairs = file_pairs[:-max_pairs]

    # Remove old files from the directory
    for jive_file, vp1_file in removed_pairs:
        for file in (jive_file, vp1_file):
            if file:
                file_path = os.path.join(directory, file)
                remove_file(file_path)

    msg.info('Removed %d file pairs.', len(removed_pairs))

    # Return the remaining pairs (latest `max_pairs`)
    return file_pairs[-max_pairs:]


def writeEventlist(directory, file_pairs, listname='event'):
    msg = logging.getLogger('EventUtils')
    msg.info('%s begin write event list', time.ctime())

    pid = os.getpid()
    temp_filename = os.path.join(directory, f"{listname}.{pid}")
    final_filename = os.path.join(directory, f"{listname}.list")

    try:
        with open(temp_filename, 'w') as file:
            for jive_file, vp1_file in file_pairs:
                run_number, event_number = None, None

                # Extract run and event numbers from the available file
                if jive_file:
                    run_number, event_number = extract_numbers(jive_file)
                elif vp1_file:
                    run_number, event_number = extract_numbers(vp1_file)

                msg.info(f"JiveXML: {jive_file}, VP1: {vp1_file}, Run: {run_number}, Event: {event_number}")

                # Write to file, replacing None values with 'N/A' for clarity
                file.write(f"run:{run_number or 'N/A'},event:{event_number or 'N/A'},"
                           f"atlantis:{jive_file or 'N/A'},vp1:{vp1_file or 'N/A'}\n")

    except IOError as err:
        msg.warning(f"Could not write event list: {err}")
        return  # Exit early if writing fails

    # Perform atomic rename operation
    try:
        os.rename(temp_filename, final_filename)
    except OSError as err:
        msg.warning(f"Could not rename {temp_filename} to {final_filename}: {err}")

    msg.info('%s end write event list', time.ctime())

def prepareFilesForTransfer(directory, file_pairs, timeinterval):
    msg = logging.getLogger( 'EventUtils' )
    msg.info('%s begin prepare files for transfer', time.ctime(time.time()))

    ready_jive = glob.glob(f"{directory}/*.zip") # atlantis files ready for transfer
    copied_jive = glob.glob(f"{directory}/*.zip.COPIED") # CastorScript bookkeeping files indicating the transfer is done
    ready_vp1 = glob.glob(f"{directory}/*.online.pool.root") # VP1 files ready for transfer
    copied_vp1 = glob.glob(f"{directory}/*.online.pool.root.COPIED") # CastorScript bookkeeping files indicating the transfer is done

    if len(ready_jive)>len(copied_jive) or len(ready_vp1)>len(copied_vp1):
        msg.info("There are files about to be transferred. Do not attempt to add new files to be transferred.")
        return

    latest_ready_jive_age = time.time() - get_latest_file_timestamp(ready_jive)
    latest_ready_vp1_age = time.time() - get_latest_file_timestamp(ready_vp1)

    # If files are too recent, wait before adding new files, but only if there are existing files
    if (latest_ready_jive_age < timeinterval) or (latest_ready_vp1_age < timeinterval):
        msg.info("Wait for %ds before adding new events to the transfer queue. Last jive event in the queue was added %ds ago, last vp1 event in the queue was added %ds ago", timeinterval, latest_ready_jive_age, latest_ready_vp1_age)
        return

    #if the last but one pair is already ready for transfer, return, otherwise prepare it for transfer
    if len(file_pairs) > 1:  # Ensure there are at least two elements to be able to get last but one element
        second_last_element = file_pairs[-2]
        jive_file, vp1_file = second_last_element

        jive_without_extension = os.path.splitext(jive_file)[0] if jive_file else None

        # Extract relevant part from VP1 file using regex
        vp1_match = re.match(r"(vp1_r\d+_ev\d+_u\d+)", vp1_file) if vp1_file else None
        vp1_without_extension = vp1_match.group(1) if vp1_match else None

        has_match = any(jive_without_extension in os.path.basename(f) for f in ready_jive) or any(vp1_without_extension in os.path.basename(f) for f in ready_vp1)
        if has_match:
            return
        if jive_file:
            msg.info('%s going to zip file %s ready for transfer to eos', time.ctime(time.time()), jive_file)
            zipXMLFile(directory, jive_file)
        if vp1_file:
            msg.info('%s going to rename ESD file %s ready for transfer to eos', time.ctime(time.time()), vp1_file)
            renameESDFile(directory, vp1_file)

def prepareALLFilesForTransfer(directory, file_pairs):
    msg = logging.getLogger('EventUtils')
    beamsplash_file = os.path.join(directory, 'beamsplash.list')
    files_in_beamsplash_file = set()

    # Check if beamsplash.list exists and is older than 1 day
    if os.path.exists(beamsplash_file):
        file_mod_time = os.path.getmtime(beamsplash_file)
        if (time.time() - file_mod_time) > 86400:  # 1 day = 86400 seconds
            msg.info(f"{beamsplash_file} is older than 1 day. Recreating...")
            open(beamsplash_file, "w").close()  # Recreate the file (empty)
        else:
            with open(beamsplash_file, "r") as f:
                files_in_beamsplash_file = set(f.read().splitlines())  # Read existing lines into a set
    else:
        msg.info(f"{beamsplash_file} does not exist. Creating a new one...")
        open(beamsplash_file, "w").close()  # Create the file

    files_to_transfer = []
    for jive_file, vp1_file in file_pairs:
        if jive_file and jive_file not in files_in_beamsplash_file:
            files_to_transfer.append(jive_file)
            zipXMLFile(directory, jive_file)
        if vp1_file and vp1_file not in files_in_beamsplash_file:
            files_to_transfer.append(vp1_file)
            renameESDFile(directory, vp1_file)

    if files_to_transfer:
        try:
            with open(beamsplash_file, "a+") as f:
                f.seek(0)
                existing_data = f.read().strip()
                existing_files = set(existing_data.split(",")) if existing_data else set()

                # Determine new files to add
                new_files = set(files_to_transfer) - existing_files
                if new_files:
                    separator = "," if existing_files else ""
                    f.write(separator + ",".join(new_files))
        except IOError as e:
            msg.error(f"Error handling file {beamsplash_file}: {e}")

def extract_numbers(filename):
    # Try to match the pattern "r<run>_ev<event>" (for VP1 files)
    match_vp1 = re.search(r'r(\d+)_ev(\d+)', filename)
    if match_vp1:
        return match_vp1.group(1), match_vp1.group(2)

    # Try to match the pattern "JiveXML_<run>_<event>" (for JiveXML files)
    match_jivexml = re.search(r'JiveXML_(\d+)_(\d+)', filename)
    if match_jivexml:
        return match_jivexml.group(1), match_jivexml.group(2)

    return None, None

def remove_file(file_path):
    msg = logging.getLogger('EventUtils')
    try:
        if os.path.exists(file_path):
            os.unlink(file_path)
            msg.info('Removed file: %s', file_path)
        else:
            msg.warning('File not found, skipping: %s', file_path)
    except Exception as e:
        msg.error('Error removing file %s: %s', file_path, str(e))

def get_latest_file_timestamp(file_list):
    """Returns the latest modified file timestamp from a list of files."""
    if not file_list:
        return (time.time() - 120)
    latest_file = max(file_list, key=os.path.getmtime)
    return os.path.getmtime(latest_file)

def zipXMLFile(directory, filename):
    msg = logging.getLogger( 'EventUtils' )
    """Zip the JiveXML file for the specified event.

    Looks for a JiveXML file with the required filename in the given directory,
    and if one is found, zip it. The original file is not deleted.
    Zip the file to .tmp first, and then rename to .zip
    to avoid triggering the transfer before the zip file is closed.
    """
    msg.info('%s begin zipXMLFile', time.ctime(time.time()))
    if Path(filename).suffix != '.xml':
        msg.warning("Unexpected Atlantis file name: %s", filename)
        return
    matchingFiles = glob.glob(f"{directory}/{filename}")
    if len(matchingFiles) == 1: # Only proceed if exactly one matching file found, for safety
        msg.info('exactly one matching file found')
        matchingFilePath = Path(matchingFiles[0])
        tmpFilePath      = matchingFilePath.with_suffix('.tmp')
        zipFilePath      = matchingFilePath.with_suffix('.zip')
        matchingFilePath = Path(matchingFilePath)
        matchingFileName = matchingFilePath.name
        msg.info('Zipping %s to %s', matchingFileName, zipFilePath.name)
        try:
            with ZipFile(tmpFilePath,'w', compression=ZIP_DEFLATED) as z:
                z.write(matchingFilePath.as_posix(), arcname=matchingFileName)
            os.rename(f'{directory}/{tmpFilePath.name}', f'{directory}/{zipFilePath.name}')
        except OSError as err:
            msg.warning("Could not zip %s: %s", filename, err)
    msg.info('%s end of zipXMLFile', time.ctime(time.time()))

def renameESDFile(directory, filename):
    msg = logging.getLogger( 'EventUtils' )
    """Rename the ESD for the specified event.

    Looks for an ESD file with the required filename in the given directory,
    and if one is found, rename it to .online.pool.root. The original file is not deleted.
    """
    msg.info('Begin renaming VP1 file %s for transfer', filename)
    if Path(filename).suffixes != ['.pool', '.root']:
        msg.warning("Unexpected VP1 file name: %s", filename)
        return
    orgname = f'{directory}/{filename}'
    newname = orgname.replace('.pool.root', '.online.pool.root')
    try:
        shutil.copyfile(Path(orgname), Path(newname))
    except OSError as err:
        msg.warning("Could not copy %s to %s: %s", orgname, newname, err)
