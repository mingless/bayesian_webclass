## @file current/tests.py
#  @brief current server status unit testing

import datetime
import django.test
import models
import views

class CurrentModelTestCase(django.test.TestCase):
    """test module model"""

    pass

class CurrentViewTestCase(django.test.TestCase):
    """test module interface"""

    def test01time(self):
        """check server current time"""
        self.assertEqual( views.time({}),
                          str(datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S")) )

    def test02get(self):
        """check if service return non-empty dict"""
        d = views.get({})
        self.assertEqual( len(d), 2)



