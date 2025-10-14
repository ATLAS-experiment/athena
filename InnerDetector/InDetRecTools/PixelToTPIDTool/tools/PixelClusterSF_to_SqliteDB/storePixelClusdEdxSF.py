#!/bin/env python
#Utilized InnerDetector/InDetRecTools/TRT_ElectronPidTools/DatabaseTools/WritePyCoolAll.py and /afs/cern.ch/user/a/alhroob/public/UpdateChargeCalibration.py as a reference
from CoolConvUtilities import AtlCoolLib
import collections
import sys
import os
from PyCool import cool
import uproot
import numpy as np
import json
import logging

#Constants
FOLDER_DB_NAME = "/PIXEL/test" 
FIELD_NAMES = ["data_array"]
FIELD_TYPES = [cool.StorageType.String16M]

def prepare_output_database(new_db_name, output_db_file):
    # Remove the output database file if it exists
    try:
        os.remove(output_db_file)
    except OSError:
        pass

    # Create a new database
    db_service = cool.DatabaseSvcFactory.databaseService()
    db_string = "sqlite://;schema=%s;dbname=%s" % (output_db_file, new_db_name)
    try:
        db = db_service.createDatabase(db_string)
    except Exception as e:
        logging.error("Error creating database:", e)
        sys.exit(-1)

    logging.info("Created database: {}".format(db_string))

    # Define record specification
    record_spec = cool.RecordSpecification()
    record_spec.extend(FIELD_NAMES[0], FIELD_TYPES[0])

    # folder meta-data - note for Athena this has a special meaning
    folder_description = '<timeStamp>run-lumi</timeStamp><addrHeader><address_header service_type="71" clid="1238547719" /></addrHeader><typeName>CondAttrListCollection</typeName>'
    folder_spec = cool.FolderSpecification(cool.FolderVersioning.MULTI_VERSION, record_spec)

    # Create folder in the database
    # last argument is createParents - if true, automatically creates parent folders if needed
    # note this will not work if the database already exists - delete mycool.db first
    folder = db.createFolder(FOLDER_DB_NAME, folder_spec, folder_description, True)
    data = cool.Record(record_spec)

    return folder, data, db

def parse_pixelClusSF_file_data(input_file_path):
    with uproot.open(input_file_path) as file:
        tree = file["SFs_TTree"]
        runNumbers = tree["f_runNumber"].array(library="np")
        bec_values = tree["f_bec"].array(library="np")
        layerID_values = tree["f_layerID"].array(library="np")
        etaM_values = tree["f_etaM"].array(library="np")
        sf_values = tree["f_SF"].array(library="np")
        #Iterate through all run numbers in the file
        #Must track if we are on a unique run number. When the run number changes, we need a new payload.
        runNumbersUnique = {}
        runNumberCurrent = runNumbers[0]
        runNumberMin = 0
        runNumberMax = 0
        for i, entry in enumerate(runNumbers):
            if runNumberCurrent != entry:
                runNumberCurrent = entry
                runNumberMin = runNumberMax
                runNumberMax = i
                runNumbersUnique[entry] = [runNumberMin, runNumberMax]
            if i == len(runNumbers): #last entry
                runNumberCurrent = entry
                runNumberMin = runNumberMax
                runNumberMax = i
                runNumbersUnique[entry] = [runNumberMin, runNumberMax]

        #Get map between wafer id and hash id
        #Values are in order: bec, ld, phi, eta, side, ID
        with open("../data/pixel_wafer_id_hash_map.json", "r") as map_file:
            hash_id_map = json.load(map_file)

        #Grab approrpriate bec, layer, eta, sf for the run
        run_data_pairs = {}
        for key in runNumbersUnique.keys():
            min_indx = runNumbersUnique[key][0]
            max_indx = runNumbersUnique[key][1]
            bec = bec_values[min_indx:max_indx].tolist()
            layerID = layerID_values[min_indx:max_indx].tolist()
            etaM = etaM_values[min_indx:max_indx].tolist()
            sf = sf_values[min_indx:max_indx].tolist()
            coordinates = list(zip(bec,layerID, etaM))
            coordinate_sf_pairs = dict(zip(coordinates, sf))
            payload = {}
            #SF data needs to be reorganized so that a sf is associated with each corresponding wafer
            for hash_val in hash_id_map.keys():
                waferID = list(hash_id_map[hash_val])
                waferID.pop(2) #Remove Phi coordinate
                if abs(waferID[0]) == 4: continue
                #SF are grouped in particular way in IBL
                if ((waferID[0] == 0) & (waferID[1] == 0)) :
                    if waferID[2] < 0:
                        etaSlice = abs(waferID[2] + 1)
                        if etaSlice > 5: etaSlice = 6
                        waferID_val = (waferID[0], waferID[1], etaSlice)
                    else:
                        etaSlice = waferID[2]
                        if waferID[2] > 5: etaSlice = 6
                        waferID_val = (waferID[0], waferID[1], etaSlice)
                else:
                    waferID_val = tuple([abs(i) for i in waferID])
                payload[hash_val] = coordinate_sf_pairs[tuple(waferID_val)]
                run_data_pairs[key] = payload
    return run_data_pairs     
 

def payload_to_json_string_converter(payload_data):
    # Convert payload_data to JSON string with specified formatting
    json_string = json.dumps(payload_data, sort_keys=True, separators=(",", ":"))
    json_string = json_string.replace('","', ",").replace('["', "[").replace('"]', "]")
    return json_string

if __name__ == "__main__":
    #Define input parameters
    local_db_file = "TEST.db"
    local_db_name = "CONDBR2" #Must match what sample meta-data expects for testing
    tag = "PixelTest"

    #Prepare output database   
    output_folder, output_data, output_db = prepare_output_database(local_db_name, local_db_file)

    #Parse data from pixel cluster scale factors input file
    pixelClusSF_input_file_path = "sf_input_files/data23_sf_flat.root"
    pixelClusSF_data_pairs = parse_pixelClusSF_file_data(pixelClusSF_input_file_path)

    #Define validity keys
    #Structure is int(run_number) << 32 | int(lumi_block)
    validity_key_min = 0 << 32 | 0

    for i, run in enumerate(pixelClusSF_data_pairs.keys()): 

        pixelClusSF_data = pixelClusSF_data_pairs[run] 

        validity_key_max = int(run) << 32 | int(2**32 -1)

        #Sort the payload data
        sorted_payload_data = collections.OrderedDict(sorted(pixelClusSF_data.items()))

        #Convert payload data to JSON string
        json_string_payload = payload_to_json_string_converter(sorted_payload_data)

        #Store data in the output folder
        output_data[FIELD_NAMES[0]] = json_string_payload
        output_folder.storeObject(validity_key_min, validity_key_max, output_data, 0, tag, True)
        
        validity_key_min = int(run) << 32 | 0


    output_db.closeDatabase()

