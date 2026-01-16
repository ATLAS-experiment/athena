# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import typing as t

JSONValue = t.Union[str, int, float, bool, None, t.Dict[str, t.Any], t.List[t.Any]]
JSONType = t.Union[t.Dict[str, JSONValue], t.List[JSONValue]]