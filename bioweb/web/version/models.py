## @file version/models.py
#  @brief server version model

"""
version state module. Return database version, database connecting strings and application build version
"""

from django.db import connection

import version_gen

def getWebSrvPrefix():
    return str(version_gen.web_srv_prefix)

def getVersionString():
    """version string, for displaying in client"""
    return str(version_gen.major) + "." + str(version_gen.minor) + "." + str(version_gen.compilation)

def getDBName():
    """database name"""
    return version_gen.DB_NAME

def getDBUser():
    """database user"""
    return version_gen.DB_USER

def getDBPassword():
    """database password"""
    return version_gen.DB_PASSWORD

def _versionFromRow(row):
    """helping function - parse row to return the correct version"""
    ver = 'unknown'
    try:
        ver = str(row[0].split(',')[0])
    except:
        pass
    return ver

def getDBVersionString():
    """database version"""
    cursor = connection.cursor()
    cursor.execute("select version();")
    row = cursor.fetchone()
    return _versionFromRow(row)



