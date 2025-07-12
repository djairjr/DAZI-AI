#include <WiFi.h>  
#include <ArduinoGPTChat.h>
#include "Audio.h"
#include "ESP_I2S.h"

// 定义I2S引脚（用于音频输出）
#define I2S_DOUT 47
#define I2S_BCLK 48
#define I2S_LRC 45

// 定义INMP441麦克风输入引脚 (I2S标准模式)
// INMP441连接方式:
// VDD -> 3.3V (不要使用5V!)
// GND -> GND
// L/R -> GND (选择左声道)
// WS  -> GPIO 25 (左右声道时钟)
// SCK -> GPIO 32 (串行时钟)
// SD  -> GPIO 33 (串行数据)
#define I2S_MIC_SERIAL_CLOCK 5    // SCK - 串行时钟
#define I2S_MIC_LEFT_RIGHT_CLOCK 4 // WS - 左右声道时钟
#define I2S_MIC_SERIAL_DATA 6     // SD - 串行数据

// 定义录音时长（秒）
#define RECORDING_DURATION  15

// WiFi设置
const char* ssid     = "2nd-curv";  
const char* password = "xbotpark";  

// OpenAI API密钥
const char* apiKey = "sk-CkxIb6MfdTBgZkdm0MtUEGVGk6Q6o5X5BRB1DwE2BdeSLSqB";  

// 不再创建全局I2S实例，将在录音函数内创建局部实例

// 全局声明audio变量，用于TTS播放
Audio audio;

// 初始化ArduinoGPTChat实例
ArduinoGPTChat gptChat(apiKey);

// 状态标志
bool gettingResponse = false;
bool recordingMode = false;

void setup() {
  // 初始化串口
  Serial.begin(115200);
  delay(1000); // 给串口一些时间来初始化
  
  Serial.println("\n\n----- 语音助手系统启动 -----");
  
  // 连接WiFi网络
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("正在连接WiFi...");
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    Serial.print('.');
    delay(1000);
    attempt++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi连接成功！");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());
    
    // 设置I2S输出引脚（用于TTS播放）
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    // 设置音量
    audio.setVolume(100);
    
    // INMP441麦克风将在录音函数中初始化 (使用标准I2S模式)
    
    Serial.println("\n----- 系统就绪 -----");
    Serial.println("1. 输入文本直接与ChatGPT对话");
    Serial.println("2. 输入'TTS:文本内容'进行语音合成");
    Serial.println("3. 输入'RECORD'开始语音录制并识别");
  } else {
    Serial.println("\n连接WiFi失败。请检查网络凭据并重试。");
  }
}

void loop() {
  // 处理音频循环（TTS播放）
  audio.loop();
  
  // 处理串口命令
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.length() > 0) {
      if (command == "RECORD") {
        // 开始语音录制和识别
        startRecordingAndTranscription();
      } else if (command.startsWith("TTS:")) {
        // 文本转语音命令
        String ttsText = command.substring(4);
        ttsText.trim();
        if (ttsText.length() > 0) {
          ttsCall(ttsText);
        } else {
          Serial.println("TTS文本为空。请使用'TTS:你的文本'格式");
        }
      } else {
        // 普通文本作为ChatGPT输入
        Serial.print("用户: ");
        Serial.println(command);
        chatGptCall(command);
      }
    }
  }
  
  delay(10); // 小延迟以防止CPU过载
}

// 开始录音并将识别结果发送给ChatGPT
void startRecordingAndTranscription() {
  Serial.println("\n----- 开始录音 -----");
  Serial.println("请说话...(录音" + String(RECORDING_DURATION) + "秒)");
  
  // 创建I2S实例 - 作为局部变量
  I2SClass i2s;
  
  // 设置INMP441麦克风I2S引脚 (标准I2S模式)
  i2s.setPins(I2S_MIC_SERIAL_CLOCK, I2S_MIC_LEFT_RIGHT_CLOCK, -1, I2S_MIC_SERIAL_DATA);

  // 初始化I2S进行录音 (使用标准I2S模式，适配INMP441)
  if (!i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("初始化I2S失败！");
    return;
  }
  
  // 创建变量存储音频数据
  uint8_t *wav_buffer;
  size_t wav_size;
  
  // 录制音频
  wav_buffer = i2s.recordWAV(RECORDING_DURATION, &wav_size);
  
  if (wav_buffer == NULL || wav_size == 0) {
    Serial.println("录制音频失败或缓冲区为空！");
    i2s.end(); // 关闭I2S
    return;
  }
  
  // 录音完成后立即关闭I2S，释放资源
  i2s.end();
  
  Serial.println("录音完成，大小: " + String(wav_size) + " 字节");
  Serial.println("正在将语音转换为文本...");
  
  // 将语音转换为文本
  String transcribedText = gptChat.speechToTextFromBuffer(wav_buffer, wav_size);
  
  // 释放缓冲区内存
  free(wav_buffer);
  
  // 显示转换结果
  if (transcribedText.length() > 0) {
    Serial.println("\n识别结果: " + transcribedText);
    
    // 自动将识别结果发送给ChatGPT
    Serial.println("\n正在将识别结果发送给ChatGPT...");
    chatGptCall(transcribedText);
  } else {
    Serial.println("未能识别到文本或发生错误。");
    Serial.println("可能没有检测到清晰的语音，请重试。");
  }
}

// 发送消息到ChatGPT
void chatGptCall(String message) {
  Serial.println("正在向ChatGPT发送请求...");
  gettingResponse = true;
  
  String response = gptChat.sendMessage(message);
  
  if (response != "") {
    Serial.print("ChatGPT: ");
    Serial.println(response);
    
    // 自动将回复转换为语音
    if (response.length() > 0) {
      ttsCall(response);
    }
    gettingResponse = false;
  } else {
    Serial.println("获取ChatGPT响应失败");
    gettingResponse = false;
  }
}

// 文本转语音
void ttsCall(String text) {
  Serial.println("正在将文本转换为语音...");
  
  // 使用GPTChat的textToSpeech方法
  bool success = gptChat.textToSpeech(text);
  
  if (success) {
    Serial.println("TTS音频正在通过扬声器播放");
  } else {
    Serial.println("播放TTS音频失败");
  }
}
