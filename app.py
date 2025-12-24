

from flask import Flask, request, jsonify, render_template
import numpy as np
import tensorflow as tf
import joblib
from twilio_alert import send_sms_alert

app = Flask(__name__, template_folder='templates')

# Load model and encoders
model = tf.keras.models.load_model("sym/activity_anomaly_model.h5")
scaler = joblib.load("sym/scaler.pkl")
activity_encoder = joblib.load("sym/activity_encoder.pkl")
anomaly_encoder = joblib.load("sym/anomaly_encoder.pkl")

# Human-readable activity labels
activity_labels = {
    0: "Sitting",
    1: "Standing",
    2: "Walking",
    3: "Running",
    4: "Sleeping"
}

# Store the latest data for front-end display
latest_data = {
    "heart_rate": "--",
    "spo2": "--",
    "activity": "--",
    "anomaly": "--",
    "suggestion": "--"
}

# Suggestion logic
def get_suggestion(anomaly):
    return {
        "Tachycardia": "High heart rate detected. Please relax or rest.",
        "Bradycardia": "Low heart rate detected. If dizzy, seek help.",
        "Critical Hypoxia": "Severely low oxygen. Seek emergency care now.",
        "Compensated Hypoxia": "Low oxygen levels. Try resting and deep breaths.",
        "Possible Sensor Error": "Please adjust the sensor and try again.",
        "Unusual (rare)": "Unusual pattern detected. Recheck or consult doctor.",
        "Normal": "Vitals are within a healthy range. Keep monitoring."
    }.get(anomaly, "No suggestion available.")

@app.route('/')
def index():
    return render_template("index.html")

@app.route('/predict', methods=['POST'])
def predict():
    global latest_data
    data = request.get_json()

    try:
        features = np.array([[data['Ax1'], data['Ay1'], data['Az1'],
                              data['Ax2'], data['Ay2'], data['Az2'],
                              data['BPM'], data['SPO2']]])
    except KeyError:
        return jsonify({"error": "Missing input fields"}), 400

    # Scale the input
    features_scaled = scaler.transform(features)

    # Model predictions
    act_pred, anom_pred = model.predict(features_scaled)
    activity_index = int(np.argmax(act_pred))
    anomaly_index = int(np.argmax(anom_pred))

    # Get readable labels
    activity = activity_labels.get(activity_index, "Unknown")
    anomaly = str(anomaly_encoder.inverse_transform([anomaly_index])[0])
    suggestion = get_suggestion(anomaly)

    # Update latest data
    latest_data = {
        "heart_rate": int(data['BPM']),
        "spo2": int(data['SPO2']),
        "activity": activity,
        "anomaly": anomaly,
        "suggestion": suggestion
    }

    # Twilio alert if not normal
    if anomaly != "Normal":
        alert_msg = f"🚨 Anomaly Detected: {anomaly}\nActivity: {activity}\nHeart Rate: {data['BPM']}, SpO2: {data['SPO2']}%"
        send_sms_alert(alert_msg)

    return jsonify(latest_data)

@app.route('/latest', methods=['GET'])
def latest():
    return jsonify(latest_data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)