## @file web/views.py
#  @brief main server interface to client

"""
main interface to server
"""

import django.http
import json
import traceback

#all modules should be imported here
import version
import version.views
import current
import current.views
import calcpy
import calcpy.views

## for test working server
def index(request):
    """for test working server"""
    return django.http.HttpResponse("MyApp server" )

def ajax(request, module, function):
    """dispatch ajax requests"""
    try:
        fun = getattr(getattr(globals()[str(module)], 'views'), str(function))
        data = json.dumps( fun(request.GET) )
        return django.http.HttpResponse(data, content_type='application/json')
    except Exception as e:
        return django.http.HttpResponseNotFound("myapp ajax error: " + str(traceback.format_exc()) )
    except:
        return django.http.HttpResponseNotFound("myapp ajax system error " )
