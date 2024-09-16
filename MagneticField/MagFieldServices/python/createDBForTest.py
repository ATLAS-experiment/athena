#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def createDB(folder, sqlite, currents):
    """Create sqlite file with DCS currents"""
    import os
    os.environ['CLING_STANDARD_PCH'] = 'none'  # See bug ROOT-10789
    from PyCool import cool
    from CoolConvUtilities import AtlCoolLib, AtlCoolTool

    # Cleanup previous file
    if os.path.isfile(sqlite):
        os.remove(sqlite)

    db = cool.DatabaseSvcFactory.databaseService().createDatabase(
        f'sqlite://;schema={sqlite};dbname=CONDBR2')
    spec = cool.RecordSpecification()
    spec.extend("value", cool.StorageType.Float)
    spec.extend("quality_invalid", cool.StorageType.Bool)
    f = AtlCoolLib.ensureFolder(
        db, folder, spec, AtlCoolLib.athenaDesc(True, 'CondAttrListCollection'))

    for v in currents:
        sol = cool.Record(spec)
        sol['value'] = v[1]
        sol['quality_invalid'] = False
        tor = cool.Record(spec)
        tor['value'] = v[2]
        tor['quality_invalid'] = False
        f.storeObject(v[0], cool.ValidityKeyMax, sol, 1)  # channel 1
        f.storeObject(v[0], cool.ValidityKeyMax, tor, 3)  # channel 3

    # print database content
    act = AtlCoolTool.AtlCoolTool(db)
    print(act.more(folder))
