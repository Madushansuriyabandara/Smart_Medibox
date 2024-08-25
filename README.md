

## Overview

This repository contains two projects developed for the EN2853: Embedded Systems and Applications course. Both projects simulate a Medibox using the ESP32 microcontroller. The Medibox is designed to remind users to take their medication on time and monitor environmental conditions such as temperature and humidity.

- **Medibox_Simulation_ESP32**: This folder covers the core functionalities including time management, alarm setting, and environmental monitoring.
- **Advanced_Features_&_NodeRED_Dashboard**: This folder builds on the basic functionality by adding more features, improved user experience, and an integration with a Node-RED dashboard.

## Project Contents

### 1. Basic Medibox Simulation 
**Folder Name:** `Medibox_Simulation_ESP32`

**Features:**
- **Time Management**: Fetches current time from an NTP server and displays it on an OLED screen.
- **Alarm System**: Allows setting up to 3 alarms with customizable alerts (buzzer, LED, OLED messages).
- **Environmental Monitoring**: Monitors temperature and humidity, providing warnings if they go outside healthy ranges.
- **Menu Navigation**: Provides a simple menu system for setting time zones, configuring alarms, and disabling alarms.

### 2. Advanced Medibox Simulation
**Folder Name:** `Advanced_Features_&_NodeRED_Dashboard`

**Enhanced Features:**
- **Expanded Time Management**: Includes advanced time zone support and improved alarm settings.
- **Enhanced Alarm System**: Features customization options, snooze functionality, and more alert mechanisms.
- **Environmental Monitoring**: Adds data logging and more detailed alerts for temperature and humidity.
- **Advanced Menu Navigation**: Offers an intuitive interface with additional settings and customization.
- **Node-RED Dashboard Integration**: Provides a visual interface for monitoring and controlling the Medibox via Node-RED.

## Node-RED Dashboard

The advanced Medibox project includes a Node-RED dashboard to visually monitor and interact with the Medibox. The dashboard provides:

- **Live Monitoring**: Displays current temperature, humidity, and alarm status.
- **Alarm Management**: Allows setting and disabling alarms directly from the dashboard.
- **Environmental Data Logging**: Visualizes temperature and humidity trends over time.
- **Control Panel**: Offers buttons to simulate interactions such as stopping the alarm or snoozing.

### Node-RED Setup Instructions

1. **Install Node-RED**:
   - Follow the official [Node-RED installation guide](https://nodered.org/docs/getting-started/).

2. **Import the Flow**:
   - Import the provided `flows.json` file into your Node-RED instance.

3. **Connect to ESP32**:
   - Ensure your ESP32 is configured to send data to the Node-RED server.
   - Update the Node-RED flow with the correct IP address and port if necessary.

4. **Run the Dashboard**:
   - Access the Node-RED dashboard via your browser (usually at `http://localhost:1880/ui`).
   - Monitor and control the Medibox from the dashboard.

## How to Use

### For Both Projects:
1. **Simulation**: Open the respective project in Wokwi.
2. **Components Used**:
   - ESP32 microcontroller
   - OLED display
   - DHT22 sensor (for temperature and humidity)
   - Buzzer
   - LEDs
   - Push buttons
3. **Follow On-Screen Instructions**: Use the OLED display and buttons to navigate the menu, set alarms, and monitor conditions.

### Node-RED Dashboard:
- **Monitor**: View real-time data and interact with the Medibox via the Node-RED dashboard.

## Demonstrations

Each project includes a video walkthrough that explains the code and demonstrates how the system works:

- **Basic Medibox Simulation**: A video detailing the core features and functionality.
- **Advanced Medibox Simulation**: A video covering the additional features, improvements, and the Node-RED dashboard.

