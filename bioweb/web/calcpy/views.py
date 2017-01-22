## @file calcpy/views.py
#  @brief calculation library interface to client

"""
calc library interface to client

export calculation results to client
"""
import calc

def classify(params):
    """params is instance of querydict"""
    query = dict(params.iterlists())

    return{
        "classification":calc.classify(query['word'][0].encode('ascii', 'ignore'))
    }
