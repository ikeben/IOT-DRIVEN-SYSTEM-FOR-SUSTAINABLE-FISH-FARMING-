from django.urls import path
from .views import *

urlpatterns = [
    # API
    path('api/sensor-readings/', SensorReadingView.as_view(), name='sensor-readings'),
    
    path('', dashboard, name='dashboard'),
    path('activities/', activities, name='activities'),
    path('table/', table, name='table'),

    # JSON endpoint
    path('get_activities_data/', get_activities_data, name='get_activities_data'),  
    path('get_table_data/', get_table_data, name='get_table_data'),  
    path('get_bert_insights/', generate_dynamic_insights, name='get_bert_insights'),
    
    # Export to CSV
    path('export/csv/', export_readings_csv, name='export_readings_csv'),
]
