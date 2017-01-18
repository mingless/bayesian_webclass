## @file wsgi.py
#  @brief Django wsgi interface

"""
Django wsgi interface, internal django file
"""

import os
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "config.settings")

from django.core.wsgi import get_wsgi_application
application = get_wsgi_application()
