from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status
from django.shortcuts import render
from .models import SensorReading
from .serializers import SensorReadingSerializer
import csv
from django.http import HttpResponse, JsonResponse
import random
import pandas as pd
from scipy.stats import linregress

class SensorReadingView(APIView):
    def post(self, request):
        serializer = SensorReadingSerializer(data=request.data)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data, status=status.HTTP_201_CREATED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

    def get(self, request):
        readings = SensorReading.objects.all().order_by('timestamp')
        serializer = SensorReadingSerializer(readings, many=True)
        return Response(serializer.data)
    

def dashboard(request):
    readings = SensorReading.objects.all().order_by('-timestamp')
    reading = SensorReading.objects.filter(
        ph__isnull=False, temperature__isnull=False
    ).order_by('-timestamp')

    # Analytics calculations
    total_measurements = reading.count()
    total_water_drained = readings.filter(activity_type="water_drained").count()
    total_water_filled = readings.filter(activity_type="water_filled").count()

    context = {
        'readings': readings,
        'total_measurements': total_measurements,
        'total_water_drained': total_water_drained,
        'total_water_filled': total_water_filled,
    }

    return render(request, 'sensors/dashboard.html', context)


def activities(request):
    total_activities = SensorReading.objects.exclude(activity_type__isnull=True).exclude(activity_type="").count()

    context = {
        'total_activities': total_activities
    }
    return render(request, 'sensors/activities.html', context) 


def get_activities_data(request):
    readings = SensorReading.objects.exclude(activity_type__isnull=True).exclude(activity_type="").order_by('-timestamp')[:50]
    processed_readings = []

    for reading in readings:
        description = []
        activity_type = "info"  # Default type

        if reading.activity_type == "measurement":
            description.append(f"Measurement taken: pH: {reading.ph}, Temp: {reading.temperature}°C")
            activity_type = "info"
        elif reading.activity_type == "water_draining":
            description.append("Water is currently draining...")
            activity_type = "warning"
        elif reading.activity_type == "water_drained":
            description.append("Water has been completely drained.")
            activity_type = "warning"
        elif reading.activity_type == "water_filling":
            description.append("Water is currently filling...")
            activity_type = "danger"
        elif reading.activity_type == "water_filled":
            description.append("Water has been completely refilled.")
            activity_type = "danger"

        if description:
            processed_readings.append({
                "description": " | ".join(description),
                "timestamp": reading.timestamp.strftime("%b %d, %Y %I:%M %p"),
                "type": activity_type,
            })

    return JsonResponse({"readings": processed_readings})


def table(request):
    total_activities = SensorReading.objects.exclude(activity_type__isnull=True).exclude(activity_type="").count()

    context = {
        'total_activities': total_activities
    }
    return render(request, 'sensors/table.html', context)


def get_table_data(request):
    readings = SensorReading.objects.exclude(activity_type__isnull=True).exclude(activity_type="").order_by('-timestamp')
    processed_readings = []

    for reading in readings:
        activity_type = None
        if reading.activity_type == "measurement":
            activity_type = "Measurement Taken"
        elif reading.activity_type == "water_draining":
            activity_type = "Water Draining"
        elif reading.activity_type == "water_drained":
            activity_type = "Water Drained"
        elif reading.activity_type == "water_filling":
            activity_type = "Water Filling"
        elif reading.activity_type == "water_filled":
            activity_type = "Water Filled"

        processed_readings.append({
            "timestamp": reading.timestamp.strftime("%b %d, %Y %I:%M %p"),
            "ph": reading.ph if reading.ph is not None else "-",
            "temperature": reading.temperature if reading.temperature is not None else "-",
            "activity": activity_type,
        })

    return JsonResponse({"readings": processed_readings})


def export_readings_csv(request):
    # Retrieve readings where activity_type is not blank or null
    readings = SensorReading.objects.exclude(activity_type__isnull=True).exclude(activity_type="").order_by('-timestamp')

    # Create the HTTP response with the correct content type
    response = HttpResponse(content_type='text/csv')
    response['Content-Disposition'] = 'attachment; filename="sensor_readings.csv"'

    # Create the CSV writer
    writer = csv.writer(response)
    writer.writerow(['Timestamp', 'pH', 'Temperature (°C)', 'Activity'])  # Header row

    # Process and write rows
    for reading in readings:
        activity_type = None
        if reading.activity_type == "measurement":
            activity_type = "Measurement Taken"
        elif reading.activity_type == "water_draining":
            activity_type = "Water Draining"
        elif reading.activity_type == "water_drained":
            activity_type = "Water Drained"
        elif reading.activity_type == "water_filling":
            activity_type = "Water Filling"
        elif reading.activity_type == "water_filled":
            activity_type = "Water Filled"

        # Write the row for the reading
        writer.writerow([
            reading.timestamp,
            reading.ph if reading.ph is not None else "-",
            reading.temperature if reading.temperature is not None else "-",
            activity_type
        ])

    return response


def generate_dynamic_insights(request):
    # Retrieve recent sensor readings
    readings = SensorReading.objects.filter(ph__isnull=False, temperature__isnull=False).order_by('-timestamp')[:100]

    if not readings:
        return JsonResponse({"insights": "No data available for analysis."})

    # Convert readings to a DataFrame for easier analysis
    data = pd.DataFrame(list(readings.values('timestamp', 'ph', 'temperature', 'water_level')))

    # Ensure required columns are present
    if data.empty or 'ph' not in data or 'temperature' not in data or 'water_level' not in data:
        return JsonResponse({"insights": "Insufficient data for generating insights."})

    # Ensure data is sorted by timestamp (ascending order)
    data = data.sort_values(by='timestamp')

    # Statistical analysis
    avg_ph = data['ph'].mean()
    avg_temp = data['temperature'].mean()

    # Get the most recent water level (current state)
    current_water_level = data['water_level'].iloc[-1]  # Last reading in sorted data

    # Detect trends using linear regression
    timestamps = range(len(data))  # Use sequential numbers as x-axis for trend detection
    ph_slope, _, _, _, _ = linregress(timestamps, data['ph'])
    temp_slope, _, _, _, _ = linregress(timestamps, data['temperature'])

    # Trend determination with stability threshold
    trend_threshold = 0.01  # Define a threshold for "stable" trend
    if abs(ph_slope) < trend_threshold:
        ph_trend = "stable"
    else:
        ph_trend = "increasing" if ph_slope > 0 else "decreasing"

    if abs(temp_slope) < trend_threshold:
        temp_trend = "stable"
    else:
        temp_trend = "increasing" if temp_slope > 0 else "decreasing"

    # Template-based insights generation
    insights_templates = []

    if avg_ph < 6.5:
        insights_templates.append("The average pH level is too low, which could harm aquatic life.")
    elif avg_ph > 8.5:
        insights_templates.append("The average pH level is too high, which might lead to water alkalinity issues.")
    else:
        insights_templates.append("The pH level is stable and within the optimal range.")

    if avg_temp > 28:
        insights_templates.append("The water temperature is quite high, which could reduce oxygen levels.")
    elif avg_temp < 20:
        insights_templates.append("The water temperature is low, which could slow down fish metabolism.")
    else:
        insights_templates.append("The water temperature is within an ideal range.")

    if current_water_level < 50:
        insights_templates.append("The water level is critically low; consider refilling the tank.")
    else:
        insights_templates.append("The current water level is sufficient and stable.")

    # Adding trends to the insights
    insights_templates.append(f"Recent trends indicate the pH level is {ph_trend}.")
    insights_templates.append(f"Recent trends indicate the temperature is {temp_trend}.")

    # Select a random insight
    random_insight = random.choice(insights_templates)

    return JsonResponse({"insights": random_insight})

