# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Initial version: Feb 2025, Riccardo Maria BIANCHI 
#                            <riccardo.maria.bianchi@cern.ch>

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
 
def createDumpGeoConfigFlags(gcf):

    # Show the Treetop content
    gcf.addFlag("GeoModel.DumpGeo.ShowTreetopContent", False, help = "Show the content of the Treetops --- (by default, only the list of Treetops is shown)")

    # Force overwrite the output .db file, if already existing
    gcf.addFlag("GeoModel.DumpGeo.ForceOverwrite", False, help = "Force to overwrite an existing SQLite output file with the same name, if any")
    
    # Set a list of Detector Managers to dump.
    gcf.addFlag("GeoModel.DumpGeo.FilterDetManagers", [], type = list, help = "Only output the GeoModel Detector Managers specified in the list")

    gcf.addFlag('GeoModel.DumpGeo.OutputFileName','', help='Provide a user defined filename for the output .db file, and override the file name built automatically from the geometry tag and the filtered Detector Managers (if any filter is applied)')

    return gcf

if __name__ == "__main__":
    dcf = AthConfigFlags()
    flags = createDumpGeoConfigFlags(dcf)
    flags.dump()