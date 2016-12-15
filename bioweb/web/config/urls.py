## @file web/urls.py
#  @brief Django urls

from django.conf.urls import patterns, include, url
#from django.conf import settings

import views
import version.models

web_srv_prefix = version.models.getWebSrvPrefix()

urlpatterns = patterns('',
                       url(r'^' + web_srv_prefix + '/ajax/(?P<module>\w+)/(?P<function>\w+)/', views.ajax, name='ajax'),
                       url(r'^$', views.index, name='index')
)



