import json 
# This script converts the geometry.dat file output obtained from the run of RunPrintSiDetElements.py into a JSON file.
identifier_hash_map = {}

with open("PixelGeometry.dat") as f:
    for i, l in enumerate(f):
            
        if l.startswith('#'):
            continue
        bec, ld, phi, eta, side, ID = l.split()[2:8]
        if int(bec) == abs(4): continue
        identifier_hash_map[i-5] = (int(bec), int(ld), int(phi), int(eta))

with open("pixWafer_id_hash_map.json", "w") as f:
    json.dump(identifier_hash_map, f)

