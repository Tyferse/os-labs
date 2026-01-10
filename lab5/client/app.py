from flask import Flask, render_template, jsonify, request
import requests
import threading
import time
from datetime import datetime, timedelta


app = Flask("temperature_client", template_folder='.\client')

SERVER_URL = "http://localhost:8080"

curr_temper = None
temper_history = []
hour_avg_today = {}
curr_hour_key = None


def get_current_temp():
    global curr_temper
    try:
        response = requests.get(f"{SERVER_URL}/current")
        if response.status_code == 200:
            data = response.json()
            curr_temper = (data["timestamp"], data["temperature"])

    except Exception as e:
        print(f"Error getting current temperature: {e}")

def get_history():
    global temper_history
    try:
        now = datetime.now()
        start_time = (now - timedelta(minutes=3)).strftime("%Y-%m-%d%%20%H:%M:%S")
        end_time = now.strftime("%Y-%m-%d%%20%H:%M:%S")
        response = requests.get(f"{SERVER_URL}/history?start={start_time}&end={end_time}")
        if response.status_code == 200:
            data = response.json()
            temper_history = [(item["timestamp"], item["temperature"]) for item in data]

    except Exception as e:
        print(f"Error getting history: {e}")

def update_hour_avg():
    global hour_avg_today, curr_hour_key
    try:
        now = datetime.now()
        today_start = (
            now.replace(minute=0, second=0, microsecond=0) 
            if hour_avg_today 
            else now.replace(hour=0, minute=0, second=0, microsecond=0) 
        )
        today_end = now.replace(minute=59, second=59, microsecond=999999)
        start_time = today_start.strftime("%Y-%m-%d %H:%M:%S") #.replace(' ', '%20')
        end_time = today_end.strftime("%Y-%m-%d %H:%M:%S") #.replace(' ', '%20')
        response = requests.get(f"{SERVER_URL}/history?start={start_time}&end={end_time}")
        if response.status_code == 200:
            data = response.json()
            # Группируем по часам
            hour_data = {}
            for item in data:
                ts = datetime.strptime(item["timestamp"], "%Y-%m-%d %H:%M:%S")
                hour_key = ts.strftime("%Y-%m-%d %H:00:00")
                if hour_key not in hour_data:
                    hour_data[hour_key] = []

                hour_data[hour_key].append(item["temperature"])
            
            # Вычисляем среднее за последний час или сутки
            for key, temps in hour_data.items():
                avg = sum(temps) / len(temps)
                hour_avg_today[key] = round(avg, 2)
            
            curr_hour_key = now.strftime("%Y-%m-%d %H:00:00")

    except Exception as e:
        print(f"Error updating hourly average: {e}")

def background_updater():
    while True:
        get_current_temp()
        get_history()
        update_hour_avg()
        time.sleep(3)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/data')
def api_data():
    global curr_temper, temper_history, hour_avg_today, curr_hour_key
    return jsonify({
        "current_temperature": curr_temper,
        "temperature_history": temper_history,
        "hour_average_today": hour_avg_today,
        "current_hour_key": curr_hour_key
    })

if __name__ == '__main__':
    # Запускаем фоновый поток для обновления данных
    updater_thread = threading.Thread(target=background_updater, daemon=True)
    updater_thread.start()
    app.run(debug=True, host='0.0.0.0', port=5000)
