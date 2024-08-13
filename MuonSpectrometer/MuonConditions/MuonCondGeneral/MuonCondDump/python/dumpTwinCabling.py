# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
def writeTwinTubeMap(mapName = "TwinTubeMap.json"):
    stName = "BOL"
    stPhi = 7
    ml = 1
    twinList = []

    for stEta in [-4, 4]:
        for layer in range (1,4):
            for tube in range (1, 73, 4):
                twinDict = {
                    "station" : stName,
                    "eta" : stEta,
                    "phi": stPhi,
                    "ml" : ml,
                    "layer": layer,
                    "tube": tube,
                    "layerTwin": layer,
                    "tubeTwin": tube + 2,
                }
                twinList+=[twinDict]
                twinDict = {
                    "station" : stName,
                    "eta" : stEta,
                    "phi": stPhi,
                    "ml" : ml,
                    "layer": layer,
                    "tube": tube+1,
                    "layerTwin": layer,
                    "tubeTwin": tube + 3,
                }
                twinList+=[twinDict]

    import json
    with open(mapName, "w") as js:
        json.dump(twinList, js, indent=4)

if __name__ == "__main__":
    writeTwinTubeMap()