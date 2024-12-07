from django.db import models

class SensorReading(models.Model):
    ACTIVITY_CHOICES = [
    ("measurement", "Measurement"),
    ("water_draining", "Water Draining"),
    ("water_drained", "Water Drained"),
    ("water_filling", "Water Filling"),
    ("water_filled", "Water Filled"),
]

    timestamp = models.DateTimeField(auto_now_add=True)
    ph = models.FloatField(blank=True, null=True)
    temperature = models.FloatField(blank=True, null=True)
    distance = models.IntegerField(blank=True, null=True)
    water_level = models.IntegerField(blank=True, null=True)  # Water level as percentage
    arm_position = models.JSONField(blank=True, null=True)  # Store arm positions as JSON
    activity_type = models.CharField(
        max_length=20, choices=ACTIVITY_CHOICES, blank=True, null=True
    )  # Type of activity

    def __str__(self):
        return f"{self.activity_type} | PH: {self.ph}, Temp: {self.temperature}, Water Level: {self.water_level}"

    class Meta:
        ordering = ['-timestamp']  # Order by timestamp in descending order
