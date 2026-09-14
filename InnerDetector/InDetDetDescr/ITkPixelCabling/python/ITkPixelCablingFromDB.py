#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

import json
import requests

def ITkPixelCablingFromCREST(url : str = 'https://atlas-crest-dev.cern.ch/api-v6.4', tag : str = 'ITkPixModIDMap-RUN4-00-01-TEST', output_file : str = 'cabling.json'):
    
    payload_hash = requests.get(
        f"{url}/iovs",
        params={"tagname": tag, "sort": "id.since:DESC", "page": 0, "size": 1},
    ).json()["resources"][0]["payloadHash"]

    doc = requests.get(
        f"{url}/payloads/data",
        params={"hash": payload_hash},
    ).json()

    doc["side-A"] = doc.pop("0")
    doc["side-C"] = doc.pop("1")

    for side in ("side-A", "side-C"):
        doc[side][0] = json.loads(doc[side][0][2:-1])

    with open(output_file, "w") as f:
        json.dump(doc, f, indent=2)