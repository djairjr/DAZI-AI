#include <WiFi.h>
#include <ArduinoGPTChat.h>
#include "Audio.h"
#include "ESP_I2S.h"
#include <vector>

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

// Define boot button pin (GPIO0 is the boot button on most ESP32 boards)
#define BOOT_BUTTON_PIN 0

// Define recording duration (seconds)
#define RECORDING_DURATION  2

// WiFi settings
const char* ssid     = "zh";
const char* password = "66666666";

// OpenAI API key
const char* apiKey = "sk-I9DfTMZFWroj7eCIq0xuFl9uQZYNIludoEyt9pCQk3rMCNaY";

// No longer create global I2S instance, will create local instance in recording function

// Global audio variable declaration for TTS playback
Audio audio;

// Initialize ArduinoGPTChat instance
ArduinoGPTChat gptChat(apiKey);

// Status flags
bool gettingResponse = false;
bool buttonPressed = false;
bool wasButtonPressed = false;
bool isRecording = false;

// Recording variables
I2SClass recordingI2S;
std::vector<int16_t> audioBuffer;
const int SAMPLE_RATE = 8000;
const int BUFFER_SIZE = 512;

void setup() {
  // Initialize serial port
  Serial.begin(115200);
  delay(1000); // Give serial port some time to initialize

  Serial.println("\n\n----- Voice Assistant System Starting -----");

  // Initialize boot button
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

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
    Serial.println("Hold BOOT button to record speech, release to send to ChatGPT");
  } else {
    Serial.println("\nFailed to connect to WiFi. Please check network credentials and retry.");
  }
}

void loop() {
  // Handle audio loop (TTS playback)
  audio.loop();

  // Handle boot button for push-to-talk
  buttonPressed = (digitalRead(BOOT_BUTTON_PIN) == LOW); // LOW when pressed (pull-up resistor)

  // Button pressed - start recording
  if (buttonPressed && !wasButtonPressed && !gettingResponse && !isRecording) {
    Serial.println("\n----- Recording Started (Hold button) -----");
    startRealTimeRecording();
    wasButtonPressed = true;
    isRecording = true;
  }
  // Button released - stop recording and process
  else if (!buttonPressed && wasButtonPressed && isRecording) {
    Serial.println("\n----- Recording Stopped -----");
    stopRecordingAndProcess();
    wasButtonPressed = false;
    isRecording = false;
  }
  // Continue recording while button is held
  else if (buttonPressed && isRecording) {
    continueRecording();
  }
  // Update button state
  else if (!buttonPressed && !isRecording) {
    wasButtonPressed = false;
  }

  delay(10); // Small delay to prevent CPU overload
}

// Start real-time recording
void startRealTimeRecording() {
  Serial.println("Speak now... (release button to stop)");

  // Clear audio buffer
  audioBuffer.clear();

  // Set INMP441 microphone I2S pins
  recordingI2S.setPins(I2S_MIC_SERIAL_CLOCK, I2S_MIC_LEFT_RIGHT_CLOCK, -1, I2S_MIC_SERIAL_DATA);

  // Initialize I2S for recording
  if (!recordingI2S.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("Failed to initialize I2S!");
    isRecording = false;
    return;
  }
}

// Continue recording while button is held
void continueRecording() {
  if (!isRecording) return;

  int16_t samples[BUFFER_SIZE];

  // Read audio samples
  size_t bytesRead = recordingI2S.readBytes((char*)samples, BUFFER_SIZE * sizeof(int16_t));

  if (bytesRead > 0) {
    // Add samples to buffer
    size_t samplesRead = bytesRead / sizeof(int16_t);
    for (size_t i = 0; i < samplesRead; i++) {
      audioBuffer.push_back(samples[i]);
    }
  }
}

// Stop recording and process audio
void stopRecordingAndProcess() {
  if (!isRecording) return;

  // Stop I2S
  recordingI2S.end();

  if (audioBuffer.empty()) {
    Serial.println("No audio data recorded!");
    return;
  }

  Serial.println("Recording completed, samples: " + String(audioBuffer.size()));
  Serial.println("Converting speech to text...");

  // Convert audio buffer to WAV format
  uint8_t* wavBuffer = createWAVBuffer(audioBuffer.data(), audioBuffer.size());
  size_t wavSize = calculateWAVSize(audioBuffer.size());

  if (wavBuffer == nullptr) {
    Serial.println("Failed to create WAV buffer!");
    return;
  }

  // Convert speech to text
  String transcribedText = gptChat.speechToTextFromBuffer(wavBuffer, wavSize);

  // Free buffer memory
  free(wavBuffer);

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

// Create WAV buffer from audio samples
uint8_t* createWAVBuffer(int16_t* samples, size_t numSamples) {
  size_t wavSize = calculateWAVSize(numSamples);
  uint8_t* wavBuffer = (uint8_t*)malloc(wavSize);

  if (wavBuffer == nullptr) {
    return nullptr;
  }

  // WAV header
  uint8_t header[44] = {
    'R','I','F','F',  // ChunkID
    0,0,0,0,          // ChunkSize (will be filled)
    'W','A','V','E',  // Format
    'f','m','t',' ',  // Subchunk1ID
    16,0,0,0,         // Subchunk1Size
    1,0,              // AudioFormat (PCM)
    1,0,              // NumChannels (Mono)
    0,0,0,0,          // SampleRate (will be filled)
    0,0,0,0,          // ByteRate (will be filled)
    2,0,              // BlockAlign
    16,0,             // BitsPerSample
    'd','a','t','a',  // Subchunk2ID
    0,0,0,0           // Subchunk2Size (will be filled)
  };

  // Fill in the values
  uint32_t chunkSize = wavSize - 8;
  uint32_t sampleRate = SAMPLE_RATE;
  uint32_t byteRate = sampleRate * 2; // 16-bit mono
  uint32_t dataSize = numSamples * 2;

  memcpy(&header[4], &chunkSize, 4);
  memcpy(&header[24], &sampleRate, 4);
  memcpy(&header[28], &byteRate, 4);
  memcpy(&header[40], &dataSize, 4);

  // Copy header
  memcpy(wavBuffer, header, 44);

  // Copy audio data
  memcpy(wavBuffer + 44, samples, numSamples * 2);

  return wavBuffer;
}

// Calculate WAV buffer size
size_t calculateWAVSize(size_t numSamples) {
  return 44 + (numSamples * 2); // Header + 16-bit samples
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
