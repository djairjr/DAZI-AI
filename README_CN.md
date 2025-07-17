<div align="center">

<img src="img/logo.png" alt="DAZI-AI Logo" width="200"/>

# 🤖 DAZI-AI

[![Arduino](https://img.shields.io/badge/Arduino-ESP32-blue.svg)](https://github.com/arduino/arduino-esp32)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://www.espressif.com/)

**无服务器 AI 语音助手 | ESP32 平台 | 纯 Arduino 开发**

[English](./README.md) | 简体中文

</div>

## ✨ 目录

- [项目简介](#-项目简介)
- [主要特性](#-主要特性)
- [系统架构](#-系统架构)
- [代码说明](#-代码说明)
- [硬件需求](#-硬件需求)
- [快速开始](#-快速开始)
- [示例项目](#-示例项目)
- [社区](#-社区)

## 📝 项目简介

DAZI-AI是一个完全基于ESP32平台开发的无服务器AI语音助手，采用纯Arduino开发环境。它允许您直接在ESP32设备上运行AI语音交互，无需额外的服务器支持。系统提供完整的语音交互功能，包括语音识别、AI处理和文本转语音输出。

## 🚀 主要特性

✅ **无服务器设计**：
- 更灵活的二次开发
- 更高的自由度（自定义提示词或模型）
- 更简单的部署（无需额外服务器）

✅ **完整的语音交互**：
- 通过INMP441麦克风进行语音输入
- 通过OpenAI API进行AI处理
- 通过MAX98357A I2S音频放大器进行语音输出

## 🔧 系统架构

系统采用模块化设计，主要包含以下核心组件：
- **语音输入**：INMP441麦克风，采用I2S接口
- **AI处理**：OpenAI ChatGPT API进行对话处理
- **语音输出**：MAX98357A I2S音频放大器进行TTS播放
- **连接性**：WiFi用于API通信

## 💻 代码说明

### GPTChatLib库
核心库文件，需要拷贝到Arduino的libraries文件夹中。

| 功能 | 描述 |
|------|------|
| ChatGPT通信 | 与OpenAI API进行通信，处理请求和响应 |
| TTS | 文本转语音功能，将AI回复转为语音 |
| STT | 语音转文本功能，将用户输入转为文本 |
| 音频处理 | 处理和转换音频数据格式 |

### 代码结构
```
├── examples/                  # 示例项目
│   └── chat/                  # 语音聊天示例
│       └── chat.ino           # 支持INMP441的主程序
└── GPTChatLib/                # 核心功能库
    ├── ArduinoGPTChat.cpp     # 库实现
    └── ArduinoGPTChat.h       # 库头文件
```

## 🔌 硬件需求

### 推荐硬件
- **主控板**：ESP32开发板（推荐ESP32-S3）
- **音频放大器**：MAX98357A或类似的I2S放大器
- **麦克风**：INMP441 I2S MEMS麦克风
- **扬声器**：4Ω 3W扬声器或耳机

### INMP441引脚连接

| INMP441引脚 | ESP32引脚 | 描述 |
|-------------|-----------|------|
| VDD | 3.3V | 电源（请勿使用5V！） |
| GND | GND | 地线 |
| L/R | GND | 左声道选择 |
| WS | GPIO 4 | 左/右时钟 |
| SCK | GPIO 5 | 串行时钟 |
| SD | GPIO 6 | 串行数据 |

### MAX98357A I2S音频输出引脚连接

| 功能 | ESP32引脚 | 描述 |
|------|-----------|------|
| I2S_DOUT | GPIO 47 | 音频数据输出 |
| I2S_BCLK | GPIO 48 | 位时钟 |
| I2S_LRC | GPIO 45 | 左/右时钟 |

## 🚀 快速开始

1. **环境准备**
   - 安装 Arduino IDE
   - 安装 ESP32 开发板支持
   - 安装必要的库：ArduinoJson, WiFi等

2. **库文件安装**
   - 将GPTChatLib文件夹复制到Arduino的libraries目录
   - 安装必要的库：
     - ArduinoWebsoket (v0.5.4)
     - ESP32-audioI2S-master (v3.0.13)
       - **注意**：如果您没有OpenAI API，需要修改ESP32-audioI2S-master库文件：
       - 打开 `./ESP32-audioI2S-master/src/Audio.cpp`
       - 查找：`char host[] = "api.openai.com"`
       - 将其修改为：`char host[] = "api.chatanywhere.tech"`
     - TFT_eSPI（用于显示屏支持）
     - SPIFFS（用于文件系统支持）

3. **配置API密钥**
   - 在代码中填入您的OpenAI API密钥
   - 更新WiFi凭据（SSID和密码）

4. **硬件连接**
   - 根据上述引脚表连接INMP441麦克风
   - 连接MAX98357A I2S音频放大器用于扬声器输出

5. **编译上传**
   - 选择合适的ESP32开发板
   - 编译并上传代码到设备

6. **测试使用**
   - 打开串口监视器
   - 按提示进行语音交互：
     - 输入文本与ChatGPT聊天
     - 使用"TTS:文本"进行文本转语音测试
     - 使用"RECORD"开始语音录制和识别

## 📚 示例项目

### 语音聊天助手 (examples/chat)
这个示例展示了与ChatGPT的完整语音交互系统。

**功能特性：**
- 使用INMP441麦克风进行语音录制
- 使用OpenAI Whisper进行语音转文本
- ChatGPT对话处理
- 自然语音的文本转语音输出
- 用于测试的串口命令界面

**使用方法：**
- 在串口监视器中输入"RECORD"开始语音录制
- 说话5秒（可配置）
- 系统将转录您的语音并发送给ChatGPT
- ChatGPT的回复将以语音形式播放

## 💬 社区

加入我们的Discord社区，与其他开发者分享经验、提问和协作：

[![Discord](https://img.shields.io/badge/Discord-加入社区-7289da?style=for-the-badge&logo=discord&logoColor=white)](https://discord.gg/RFPwfhTM)

**Discord服务器**: https://discord.gg/RFPwfhTM

---

<div align="center">
  <b>开源协作，共同进步！</b><br>
  如果您觉得这个项目有帮助，请给它一个⭐️
</div>
