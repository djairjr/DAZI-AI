# Agent Automation Configuration Guide - DAZI-AI Project

## Execution Process Instructions
This guide is designed specifically for programming agents. Please execute the following steps in order:
Run only one command at a time

### 1. Environment Setup
```bash
# First verify if Arduino CLI is installed
arduino-cli version
# If not downloaded, download Arduino CLI (Windows)
curl -L -o arduino-cli_latest_Windows_64bit.zip https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_64bit.zip

# Extract and prepare
tar -xf arduino-cli_latest_Windows_64bit.zip
mkdir arduino-cli && move arduino-cli.exe arduino-cli/

# Verify installation
arduino-cli/arduino-cli.exe version

# Initialize configuration
arduino-cli/arduino-cli.exe config init
```

### 2. Install Board Support
```bash
# Check if ESP32 core is installed (any 3.x.x version can skip to next step)
arduino-cli core list

# If version is not 3.x.x (Windows)
arduino-cli core install esp32:esp32@3.0.11

```

### 3. Install Required Libraries
```bash
# Install ArduinoWebsockets v0.5.4
arduino-cli lib install "ArduinoWebsockets@0.5.4"

# Install TFT_eSPI library
arduino-cli lib install "TFT_eSPI"

# Install ArduinoJson library (required dependency)
arduino-cli lib install "ArduinoJson"

# Extract ESP32-audioI2S library (needs extraction before use)
tar -xf ESP32-audioI2S-master.zip
```

### 4. Board Selection (Critical Interactive Step)
```markdown
**Please ask the user for their board model:**
- Common ESP32 boards:
  - XIAO ESP32S3 → FQBN: `esp32:esp32:XIAO_ESP32S3`
  - ESP32-S3 Dev Module → FQBN: `esp32:esp32:esp32s3`
  - NodeMCU-32S → FQBN: `esp32:esp32:nodemcu-32s`
- Use command to get complete list: `arduino-cli board listall esp32:esp32`
```

### 5. Compilation Preparation
```markdown
**Important Reminder:**
- First compilation takes about 5 minutes, please inform user to be patient

```

### 6. Execute Compilation
```bash
# Use user-provided board FQBN
# Enable PSRAM: add :PSRAM=opi after FQBN
arduino-cli compile \
  --fqbn "USER_PROVIDED_FQBN:PSRAM=opi" \
  --library "./GPTChatLib" \
  --library "./ESP32-audioI2S-master" \
  examples/chat/chat.ino
```

### 7. Upload Program
```bash
# Get device serial port
arduino-cli board list

# Use user-provided serial port
arduino-cli upload -p "USER_PROVIDED_PORT" --fqbn "USER_PROVIDED_FQBN:PSRAM=opi" examples/chat/chat.ino

# Upload failure solutions:
# 1. Force release serial port resources
taskkill /F /IM arduino-cli.exe

# 2. Manually reset the board:
#    - Hold BOOT button
#    - Press RESET button briefly
#    - Execute upload command within 1 second

# 3. Use esptool direct flashing (ultimate solution):
esptool.py --port [PORT] --baud 921600 write_flash 0x1000 examples/chat/.pio/build/XIAO_ESP32S3/firmware.bin
```

### 8. Read Serial Output
```bash
# Start serial monitor (default baud rate 115200)
arduino-cli monitor -p [PORT] --config baudrate=115200

# Common baud rate options: 9600, 19200, 38400, 57600, 115200
# If connection fails:
# 1. Check if board is powered
# 2. Confirm correct port number
# 3. Try reconnecting USB cable
# 4. Check if program has serial output code
```

## Key Considerations
1. **Board Selection**: Must clearly identify the specific board model used by the user
2. **First Compilation**: Emphasize it takes about 5 minutes to avoid user interruption
3. **Serial Port Permissions**: Linux/Mac may require sudo permissions
4. **Upload Preparation**: Hold BOOT button before inserting USB for upload
5. **Serial Monitoring**: Use the following command to view device output:
   ```bash
   arduino-cli monitor -p [PORT]
   ```

## Project Structure Update
This guide has been updated for the current DAZI-AI project structure:
- Main example: `examples/chat/chat.ino`
- Core library: `GPTChatLib/`
- Audio library: `ESP32-audioI2S-master/`
- Hardware: INMP441 microphone + MAX98357A amplifier
