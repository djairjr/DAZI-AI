#include <WiFi.h>  
#include <ArduinoGPTChat.h>
#include "Audio.h"
#include "ESP_I2S.h"

// Define I2S pins for audio output
#define I2S_DOUT 47
#define I2S_BCLK 48
#define I2S_LRC 45

// Define INMP441 microphone input pins (I2S standard mode)
// INMP441 wiring:
// VDD -> 3.3V (DO NOT use 5V!)
// GND -> GND
// L/R -> GND (select left channel)
// WS  -> GPIO 25 (left/right clock)
// SCK -> GPIO 32 (serial clock)
// SD  -> GPIO 33 (serial data)
#define I2S_MIC_SERIAL_CLOCK 5    // SCK - serial clock
#define I2S_MIC_LEFT_RIGHT_CLOCK 4 // WS - left/right clock
#define I2S_MIC_SERIAL_DATA 6     // SD - serial data

// Define recording duration (seconds)
#define RECORDING_DURATION  5

// WiFi settings
const char* ssid     = "2nd-curv";
const char* password = "xbotpark";

// OpenAI API key
const char* apiKey = "sk-CkxIb6MfdTBgZkdm0MtUEGVGk6Q6o5X5BRB1DwE2BdeSLSqB";

// No longer create global I2S instance, will create local instance in recording function

// Global audio variable declaration for TTS playback
Audio audio;

// Initialize ArduinoGPTChat instance
ArduinoGPTChat gptChat(apiKey);

// Status flags
bool gettingResponse = false;
bool recordingMode = false;

void setup() {
  // Initialize serial port
  Serial.begin(115200);
  delay(1000); // Give serial port some time to initialize

  Serial.println("\n\n----- Voice Assistant System Starting -----");

  // Connect to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    Serial.print('.');
    delay(1000);
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Set I2S output pins (for TTS playback)
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    // Set volume
    audio.setVolume(100);

    // INMP441 microphone will be initialized in recording function (using standard I2S mode)

    Serial.println("\n----- System Ready -----");
    Serial.println("1. Enter text to chat directly with ChatGPT");
    Serial.println("2. Enter 'TTS:text content' for speech synthesis");
    Serial.println("3. Enter 'RECORD' to start voice recording and recognition");
  } else {
    Serial.println("\nFailed to connect to WiFi. Please check network credentials and retry.");
  }
}

void loop() {
  // Handle audio loop (TTS playback)
  audio.loop();

  // Handle serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.length() > 0) {
      if (command == "RECORD") {
        // Start voice recording and recognition
        startRecordingAndTranscription();
      } else if (command.startsWith("TTS:")) {
        // Text-to-speech command
        String ttsText = command.substring(4);
        ttsText.trim();
        if (ttsText.length() > 0) {
          ttsCall(ttsText);
        } else {
          Serial.println("TTS text is empty. Please use 'TTS:your text' format");
        }
      } else {
        // Regular text as ChatGPT input
        Serial.print("User: ");
        Serial.println(command);
        chatGptCall(command);
      }
    }
  }

  delay(10); // Small delay to prevent CPU overload
}

// Start recording and send recognition result to ChatGPT
void startRecordingAndTranscription() {
  Serial.println("\n----- Starting Recording -----");
  Serial.println("Please speak... (recording for " + String(RECORDING_DURATION) + " seconds)");

  // Create I2S instance - as local variable
  I2SClass i2s;

  // Set INMP441 microphone I2S pins (standard I2S mode)
  i2s.setPins(I2S_MIC_SERIAL_CLOCK, I2S_MIC_LEFT_RIGHT_CLOCK, -1, I2S_MIC_SERIAL_DATA);

  // Initialize I2S for recording (using standard I2S mode, compatible with INMP441)
  if (!i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("Failed to initialize I2S!");
    return;
  }

  // Create variables to store audio data
  uint8_t *wav_buffer;
  size_t wav_size;

  // Record audio
  wav_buffer = i2s.recordWAV(RECORDING_DURATION, &wav_size);

  if (wav_buffer == NULL || wav_size == 0) {
    Serial.println("Failed to record audio or buffer is empty!");
    i2s.end(); // Close I2S
    return;
  }

  // Close I2S immediately after recording to release resources
  i2s.end();

  Serial.println("Recording completed, size: " + String(wav_size) + " bytes");
  Serial.println("Converting speech to text...");

  // Convert speech to text
  String transcribedText = gptChat.speechToTextFromBuffer(wav_buffer, wav_size);

  // Free buffer memory
  free(wav_buffer);

  // Display conversion result
  if (transcribedText.length() > 0) {
    Serial.println("\nRecognition result: " + transcribedText);

    // Automatically send recognition result to ChatGPT
    Serial.println("\nSending recognition result to ChatGPT...");
    chatGptCall(transcribedText);
  } else {
    Serial.println("Failed to recognize text or an error occurred.");
    Serial.println("Clear speech may not have been detected, please try again.");
  }
}

// Send message to ChatGPT
void chatGptCall(String message) {
  Serial.println("Sending request to ChatGPT...");
  gettingResponse = true;

  String response = gptChat.sendMessage(message);

  if (response != "") {
    Serial.print("ChatGPT: ");
    Serial.println(response);

    // Automatically convert reply to speech
    if (response.length() > 0) {
      ttsCall(response);
    }
    gettingResponse = false;
  } else {
    Serial.println("Failed to get ChatGPT response");
    gettingResponse = false;
  }
}

// Text to speech
void ttsCall(String text) {
  Serial.println("Converting text to speech...");

  // Use GPTChat's textToSpeech method
  bool success = gptChat.textToSpeech(text);

  if (success) {
    Serial.println("TTS audio is playing through speaker");
  } else {
    Serial.println("Failed to play TTS audio");
  }
}
