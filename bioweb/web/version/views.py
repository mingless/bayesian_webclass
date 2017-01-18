## @file version/views.py
#  @brief server version interface to client

"""
version interface module. Return database version, database connecting strings and application build version
"""

import datetime
import models

def get(params):
    """versions"""
    return {
        "paramsVer" : 1,
        "server": models.getVersionString(),
        "database": models.getDBVersionString(),
    }
