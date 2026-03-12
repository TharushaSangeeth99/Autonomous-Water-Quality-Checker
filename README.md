<p align="center">
  <img src="Media/banner_robot.jpg" width="900">
</p>

<h1 align="center">
Autonomous Portable Water Quality Monitoring Robot
</h1>

<p align="center">
An IoT-based autonomous robotic system designed for real-time water quality monitoring and environmental analysis.
</p>

---

# Project Overview

Water pollution monitoring is traditionally performed through manual sampling and laboratory testing, which can be time-consuming and expensive.  

This project introduces a **low-cost autonomous robotic system** capable of monitoring water quality parameters in real time and transmitting data to a cloud dashboard.

The robot can automatically collect and analyze environmental data from water bodies and visualize it through an IoT-based monitoring system.

### Parameters Measured

- Total Dissolved Solids (TDS)
- Water Temperature
- Ambient Humidity
- Electrical Conductivity
- Water Hardness
- Water Quality Index

---

# Complete Robot System

<p align="center">
<img src="images/robot_complete.jpg" width="700">
</p>

The final system consists of a floating robotic platform equipped with sensors, wireless communication modules, and an IoT dashboard for monitoring environmental data.

---

# System Architecture

<p align="center">
<img src="images/system_architecture.png" width="800">
</p>

The system architecture integrates hardware sensors, wireless communication, cloud data processing, and a real-time dashboard.

### Hardware Components

- ESP32 Microcontroller
- TDS Sensor
- DS18B20 Temperature Sensor
- DHT11 Humidity Sensor
- nRF24L01 Communication Module
- Motorized Depth Winch System

### Software Components

- MQTT Communication
- Telegraf Data Processing
- InfluxDB Time-Series Database
- React.js Web Dashboard
- Grafana Visualization

---

# Mechanical Design (SolidWorks)

<p align="center">
<img src="images/mechanical_model.png" width="700">
</p>

The mechanical structure was designed using **SolidWorks** with a **catamaran hull design** to provide stability in water.

### Key Mechanical Features

- Twin hull PVC floating structure  
- Lightweight PLA mounting platform  
- Sensor deployment mechanism  
- Integrated motorized winch system  

---

# Circuit Design

<p align="center">
<img src="images/circuit_diagram.png" width="750">
</p>

A custom electronic circuit was designed to integrate sensors, communication modules, and power management.

### Main Circuit Blocks

- ESP32 control unit  
- Sensor interface circuits  
- Motor driver circuits  
- Wireless communication module  
- Voltage regulation system  

---

# PCB Layout Design

### PCB Top Layer

<p align="center">
<img src="images/pcb_top.png" width="650">
</p>

### PCB Bottom Layer

<p align="center">
<img src="images/pcb_bottom.png" width="650">
</p>

The PCB was designed using **Altium Designer** with optimized routing and ground planes for stable sensor measurements.

---

# PCB Prototype

<p align="center">
<img src="images/pcb_prototype.jpg" width="650">
</p>

Initial testing was conducted using prototype PCBs to validate circuit functionality and sensor interfacing.

---

# Fabricated PCB

<p align="center">
<img src="images/pcb_fabricated.jpg" width="650">
</p>

The final PCB was fabricated using **FR4 double-layer boards**, ensuring improved durability and electrical reliability.

---

# IoT Monitoring Dashboard

<p align="center">
<img src="images/dashboard.png" width="850">
</p>

The system includes a **real-time IoT dashboard** displaying sensor readings and historical environmental data.

### Dashboard Features

- Live sensor monitoring  
- Historical data visualization  
- Water quality analysis  
- Remote monitoring capability  
- Data logging and reporting  

---

# Field Testing

<p align="center">
<img src="images/field_test.jpg" width="750">
</p>

The robot was tested in real water environments to validate sensor accuracy, wireless communication, and system stability.

---

# Key Features

- Autonomous water monitoring robot
- Real-time IoT data transmission
- Depth-based water sampling
- Custom PCB electronics
- Wireless sensor communication
- Cloud-based data visualization

---

# Technologies Used

### Hardware

- ESP32
- nRF24L01
- TDS Sensor
- DS18B20 Temperature Sensor
- DHT11 Sensor

### Software

- Arduino IDE
- MQTT
- Telegraf
- InfluxDB
- React.js
- Grafana

### Design Tools

- SolidWorks
- Altium Designer

---

# Project Report

Full research documentation:

📄 **Download Report**
