import json
from channels.generic.websocket import AsyncWebsocketConsumer

class SensorDataConsumer(AsyncWebsocketConsumer):
    async def connect(self):
        await self.accept()

    async def disconnect(self, close_code):
        pass

    async def send_sensor_data(self, event):
        # Send data to the WebSocket
        await self.send(text_data=json.dumps(event["data"]))
