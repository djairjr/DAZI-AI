<div align="center">

<img src="img/logo.png" alt="DAZI-AI Logo" width="200"/>

# 🤖 DAZI-AI

[![Arduino](https://img.shields.io/badge/Arduino-ESP32-blue.svg)](https://github.com/arduino/arduino-esp32)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://www.espressif.com/)

**Serverless AI Voice Assistant | ESP32 Platform | Pure Arduino Development**

English | [简体中文](./README_CN.md)

</div>

## ✨ Table of Contents

- [Project Introduction](#-project-introduction)
- [Key Features](#-key-features)
- [System Architecture](#-system-architecture)
- [Code Description](#-code-description)
- [Hardware Requirements](#-hardware-requirements)
- [Quick Start](#-quick-start)
- [Example Projects](#-example-projects)
- [Community](#-community)

## 📝 Project Introduction

DAZI-AI is a serverless AI voice assistant developed entirely on the ESP32 platform using the Arduino environment. It allows you to run AI voice interactions directly on ESP32 devices without the need for additional server support. The system provides complete voice interaction capabilities including speech recognition, AI processing, and text-to-speech output.

## 🚀 Key Features

✅ **Serverless Design**:
- More flexible secondary development
- Higher degree of freedom (customize prompts or models)
- Simpler deployment (no additional server required)

✅ **Complete Voice Interaction**:
- Voice input via INMP441 microphone
- AI processing through OpenAI API
- Voice output via MAX98357A  I2S audio amplifier


## 🔧 System Architecture

The system uses a modular design with the following key components:
- **Voice Input**: INMP441 microphone with I2S interface
- **AI Processing**: OpenAI ChatGPT API for conversation
- **Voice Output**: MAX98357A  I2S audio amplifier for TTS playback
- **Connectivity**: WiFi for API communication

## 💻 Code Description

### GPTChatLib Library
Core library files that need to be copied to Arduino's libraries folder.

| Feature | Description |
|---------|-------------|
| ChatGPT Communication | Communicates with OpenAI API, handles requests and responses |
| TTS | Text-to-Speech functionality, converts AI replies to voice |
| STT | Speech-to-Text functionality, converts user input to text |
| Audio Processing | Processes and converts audio data formats |

### Code Structure
```
├── examples/                  # Example projects
│   └── chat/                  # Voice chat example
│       └── chat.ino           # Main program with INMP441 support
└── GPTChatLib/                # Core functionality library
    ├── ArduinoGPTChat.cpp     # Library implementation
    └── ArduinoGPTChat.h       # Library header
```

## 🔌 Hardware Requirements

### Recommended Hardware
- **Controller**: ESP32 development board (ESP32-S3 recommended)
- **Audio Amplifier**: MAX98357A or similar I2S amplifier
- **Microphone**: INMP441 I2S MEMS microphone
- **Speaker**: 4Ω 3W speaker or headphones

### INMP441 Pin Connections

| INMP441 Pin | ESP32 Pin | Description |
|-------------|-----------|-------------|
| VDD | 3.3V | Power (DO NOT use 5V!) |
| GND | GND | Ground |
| L/R | GND | Left channel select |
| WS | GPIO 4 | Left/Right clock |
| SCK | GPIO 5 | Serial clock |
| SD | GPIO 6 | Serial data |

### MAX98357A  I2S Audio Output Pin Connections

| Function | ESP32 Pin | Description |
|----------|-----------|-------------|
| I2S_DOUT | GPIO 47 | Audio data output |
| I2S_BCLK | GPIO 48 | Bit clock |
| I2S_LRC | GPIO 45 | Left/Right clock |

## 🚀 Quick Start

1. **Environment Setup**
   - Install Arduino IDE
   - Install ESP32 board support

2. **Library Installation**
   - Copy the following folders to Arduino's libraries directory:
     - Copy `GPTChatLib` folder to `Arduino/libraries/`
     - Copy `ESP32-audioI2S-master` folder to `Arduino/libraries/`
   - Install additional required libraries through Arduino Library Manager:
     - ArduinoWebsoket (v0.5.4)
     - ArduinoJson (v7.4.1)

3. **API Key Configuration**
   - Open `examples/chat/chat.ino`
   - Replace `"your-api-key"` with your actual OpenAI API key (line 50)
   - Replace `"your-wifi-ssid"` and `"your-wifi-password"` with your WiFi credentials (lines 36-37)
   - Optionally modify the system prompt to customize AI behavior (line 55)

4. **Hardware Wiring**
   - Connect INMP441 microphone according to pin table above
   - Connect MAX98357A I2S audio amplifier for speaker output


5. **Compile and Upload**
   - Select the appropriate ESP32 development board
     - This project has been tested on ESP32S3 Dev Module and XIAO ESP32S3
     - Requirements: Flash Size >8M and PSRAM >4Mb
   - In Arduino IDE, configure board settings:
     - Partition Scheme: Select "8M with spiffs"
     - PSRAM: Select "OPI PSRAM"
   - Compile and upload the code to your device

6. **Testing**
   - Open the serial monitor
   - Hold the BOOT button on your ESP32 to start recording
   - Speak your question or command while holding the button
   - Release the button to send the recording to ChatGPT
   - Listen to the AI response through your connected speaker

## 📚 Example Projects

### Voice Chat Assistant (examples/chat)
This example demonstrates a complete voice interaction system with ChatGPT.

**Features:**
- Push-to-talk voice recording with INMP441 microphone using BOOT button
- Speech-to-text conversion using OpenAI Whisper API
- ChatGPT conversation processing with customizable system prompts
- Text-to-speech output with natural voice playback
- Real-time audio processing and I2S audio output
- Configurable API endpoints for different OpenAI-compatible services

**Usage:**
- Hold the BOOT button to start voice recording
- Speak while holding the button
- Release the button to stop recording and send to ChatGPT
- The system will transcribe your speech and send it to ChatGPT
- ChatGPT's response will be played back as speech through the speaker

**Push-to-Talk Control:**
- The system uses the ESP32's built-in BOOT button (GPIO 0) for voice control
- Press and hold to record, release to process
- No need to type commands in serial monitor - just use the button!

## 💬 Community

Join our Discord community to share development experiences, ask questions, and collaborate with other developers:

[![Discord](https://img.shields.io/badge/Discord-Join%20Community-7289da?style=for-the-badge&logo=discord&logoColor=white)](https://discord.gg/RFPwfhTM)

**Discord Server**: https://discord.gg/RFPwfhTM

---

<div align="center">
  <b>Open source collaboration for shared progress!</b><br>
  If you find this project helpful, please give it a ⭐️
</div>
