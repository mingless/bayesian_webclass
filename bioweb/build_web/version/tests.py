## @file version/tests.py
#  @brief version module unit testing

import datetime
import django.test
import models
import views

class VersionModelTestCase(django.test.TestCase):
    """test version model"""

    def test01getVersionString(self):
        """test if getVersion return non-empty string"""
        self.assertTrue( len(models.getVersionString()) > 0 )

    def test02getDBName(self):
        """test if getDBName return non-empty string"""
        self.assertTrue( len(models.getDBName()) > 0)

    def test03getDBUser(self):
        """test if getDBUser return non-empty string"""
        self.assertTrue( len(models.getDBUser()) > 0)

    def test04getDBPassword(self):
        """test if getDBPassword return non-empty string"""
        self.assertTrue( len(models.getDBPassword()) > 0)

    def test05getDBVersionString(self):
        """test if getDBPassword return any output"""
        v = models.getDBVersionString()
        self.assertTrue( len(v) > 0)
        self.assertNotEqual( str(v), 'unknown' )

    def test06versionFromRow(self):
        """check if database version string has 'ver'"""
        self.assertEqual( models._versionFromRow(''), 'unknown' )
        t = ('ver,xxx',)
        self.assertEqual( models._versionFromRow(t), 'ver')

class VersionViewTestCase(django.test.TestCase):
    """test version interface"""

    def test01get(self):
        """check if get service return non-empty dict"""
        d = views.get({})
        self.assertEqual( len(d), 3)



