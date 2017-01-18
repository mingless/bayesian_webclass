## @file calcpy/tests.py
#  @brief c++ calculation library Python API unit testing

import django.test
import calc
import views
import time

class CalcPyLibraryTestCase(django.test.TestCase):
    """integration test, call C++ library interface from Python"""

    def test01getNumber(self):
        """test the return number"""
        self.assertEqual( calc.getNumber(), 1234 )

    def test02command(self):
        """start/stop tick command test"""
        cmdmgr = calc.CommandManager()
        cmd_id = cmdmgr.start()
        self.assertNotEqual( cmdmgr.getState(cmd_id), calc.DONE )
        for i in range(100):
            time.sleep(0.1)
            if cmdmgr.getState(cmd_id) == calc.DONE:
                break
        self.assertEqual( cmdmgr.getState(cmd_id), calc.DONE )
        self.assertEqual( len(cmdmgr.getIds()), 1 )


class CalcPyViewTestCase(django.test.TestCase):
    """module view test"""

    def test01getNumber(self):
        """check if service return proper dict"""
        self.assertEqual( views.getNumber({}), {'number': 1234})

    def test02getCommands(self):
        """check if service return proper dict"""
        self.assertEqual( views.getCommands({}),
                          {1: {'progress': 0.995, 'state': 'DONE'}})
        dict1 = views.getCommands({})
        views.startCommand({});
        dict2 = views.getCommands({})
        self.assertEqual( len(dict1) + 1, len(dict2) )







