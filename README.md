# 🩺 VitalSense – Smart Wearable Health Monitoring System

VitalSense is an IoT and Machine Learning–based wearable health monitoring system that tracks vital parameters in real time, detects anomalies, and sends instant alerts. The system integrates sensors, a Flask backend, an ML model, a web dashboard, and SMS notifications for emergency situations.

---

## 📌 Features

* Real-time monitoring of **Heart Rate (BPM)** and **SpO₂**
* **Activity recognition** using motion sensor data
* **ML-based anomaly detection**
* Live **web dashboard** for visualization
* Automatic **SMS alerts** using Twilio
* REST APIs for easy integration
* Scalable and modular architecture

---

## 🛠️ Hardware Requirements

* ESP32 / ESP8266
* MAX30102 (Heart Rate & SpO₂ Sensor)
* MPU6050 (Accelerometer & Gyroscope)
* Power supply / wearable setup
---

## 🔌 Hardware Design & Connections

VitalSense uses an ESP-based wearable hardware setup to acquire physiological and motion data. The sensors communicate with the controller using **I2C** and **Wi-Fi**, ensuring low power consumption and real-time data transfer.

---

### 🧠 Microcontroller

* **ESP32 / ESP8266**
* Acts as the central controller
* Handles sensor interfacing, data preprocessing, and communication with backend

---

### ❤️ MAX30102 – Heart Rate & SpO₂ Sensor (I2C)

Used to measure pulse rate and blood oxygen saturation.

**Connections (ESP8266 / ESP32):**

| MAX30102 Pin | ESP Pin                            | Description  |
| ------------ | ---------------------------------- | ------------ |
| VCC          | 3.3V                               | Power supply |
| GND          | GND                                | Ground       |
| SDA          | GPIO 4 (ESP8266) / GPIO 21 (ESP32) | I2C Data     |
| SCL          | GPIO 5 (ESP8266) / GPIO 22 (ESP32) | I2C Clock    |

📌 Operates strictly at **3.3V logic**

---

### 🏃 MPU6050 – Accelerometer & Gyroscope (Motion Sensor)

Used for **activity recognition** (sitting, walking, running, etc.).

Two MPU6050 sensors can be used for enhanced motion accuracy.

**Communication Method:**

* MPU6050 data is collected by a **separate ESP module**
* Data is transmitted wirelessly to the main ESP using **Wi-Fi UDP**

**MPU6050 (Sender ESP) Connections:**

| MPU6050 Pin | ESP Pin |
| ----------- | ------- |
| VCC         | 3.3V    |
| GND         | GND     |
| SDA         | I2C SDA |
| SCL         | I2C SCL |

---

### 🌐 Wireless Communication

* **Wi-Fi (UDP Protocol)**
* Enables fast, low-latency transfer of motion data
* Main ESP receives MPU data on a defined UDP port

```cpp
const int udpPort = 1234;
```

---

### 🔋 Power Supply

* USB / Li-ion battery
* Onboard 3.3V regulation for sensors
* Suitable for wearable form factor

---

### 🧩 Hardware Data Flow

```
MAX30102  ──I2C──▶ ESP
MPU6050s ──ESP──UDP──▶ ESP
ESP ──Wi-Fi──▶ Flask Backend
```

---

### ⚠️ Important Hardware Notes

* Use **short wires** for I2C stability
* Avoid 5V supply to MAX30102 / MPU6050
* Ensure common ground across all modules
* Keep wearable sensors firmly attached for accurate readings


## 💻 Software & Technologies Used

* **Python**
* **Flask** (Backend API)
* **TensorFlow / Keras** (ML Model)
* **HTML, CSS, JavaScript** (Frontend Dashboard)
* **Chart.js** (Data visualization)
* **Twilio API** (SMS alerts)
* **SQLite** (Optional data storage)

---

## 🧠 System Architecture (Workflow)

1. Sensors collect real-time health data
2. ESP sends data to Flask backend (JSON format)
3. Data is scaled and passed to ML model
4. Model predicts **activity + anomaly**
5. Results displayed on web dashboard
6. If anomaly detected → **SMS alert sent**

---

## 📂 Project Structure

```
VitalSense/
│
├── app.py                  # Flask backend
├── twilio_alert.py         # SMS alert logic
├── templates/
│   └── index.html          # Frontend dashboard
├── sym/
│   ├── activity_anomaly_model.h5
│   ├── scaler.pkl
│   ├── activity_encoder.pkl
│   └── anomaly_encoder.pkl
├── vitalsense.db           # Database (optional)
├── requirements.txt
└── README.md
```

---

## ⚙️ How the Project Works (Step by Step)

### 1️⃣ Sensor Data Collection

* ESP reads:

  * Accelerometer values (Ax, Ay, Az)
  * Heart rate (BPM)
  * SpO₂ (%)
* Data is sent to backend as JSON.

### 2️⃣ Backend Processing (Flask)

* Flask receives data at `/predict`
* Input is scaled using `scaler.pkl`
* ML model predicts:

  * **Activity** (Sitting, Walking, Running, etc.)
  * **Anomaly type** (Normal, Tachycardia, Hypoxia, etc.)

### 3️⃣ Anomaly Detection Logic

If anomaly ≠ `Normal`, system:

* Generates health suggestion
* Triggers SMS alert via Twilio

### 4️⃣ Frontend Dashboard

* Fetches live data from `/latest`
* Displays:

  * Heart Rate
  * SpO₂
  * Activity
  * Anomaly status
* Shows alerts and charts in real time

---

## 🚨 SMS Alert System

* Uses **Twilio API**
* Sends emergency SMS when abnormal vitals detected
* Example alert:

```
🚨 Anomaly Detected: Critical Hypoxia
Activity: Walking
Heart Rate: 115
SpO₂: 85%
```

---

## ▶️ How to Run the Project

### Step 1: Install Dependencies

```bash
pip install flask tensorflow joblib numpy twilio
```

### Step 2: Run Backend

```bash
python app.py
```

### Step 3: Open Dashboard

```
http://localhost:5000
```

---

## 📊 API Endpoints

| Endpoint   | Method | Description       |
| ---------- | ------ | ----------------- |
| `/`        | GET    | Web dashboard     |
| `/predict` | POST   | Send sensor data  |
| `/latest`  | GET    | Get latest vitals |

---

## 📈 Future Enhancements

* Mobile app integration
* Cloud database (Firebase/AWS)
* Doctor dashboard & patient history
* Wearable miniaturization
* AI health chatbot integration

---

## 👨‍💻 Author

**Pavan K**
Electronics & Communication Engineer
IoT | Embedded Systems | Machine Learning



