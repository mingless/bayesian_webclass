## @file calcpy/views.py
#  @brief calculation library interface to client

"""
calc library interface to client

export calculation results to client
"""
import calc

def getNumber(params):
    """the calculation from C++ library"""
    return {
        "number" : calc.getNumber()
    }

def getCommands(params):
    """return the commands descriptors"""
    cmdmgr = calc.CommandManager()
    ids = cmdmgr.getIds()
    out = dict()
    for i in ids:
        out[int(i)] = { "state" : str(cmdmgr.getState(i)), "progress": float(cmdmgr.getProgress(i)) }
    return out

def startCommand(params):
    """start new tick command"""
    cmdmgr = calc.CommandManager()
    cmd_id = cmdmgr.start()
    return cmd_id

