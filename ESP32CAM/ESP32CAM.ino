#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Wi-Fi Credentials for ESP32-CAM module
const char* ssid = "Galaxy A32 EB94";
const char* password = "fare1840";

// Function declaration for starting the camera HTTP server
void startCameraServer();

// OV3660 / AI Thinker camera module pin configuration
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

// Y-series pins for camera data transmission
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  Serial.begin(115200);
  
  // Disable brownout detector to prevent reset during high power draw (e.g., when initializing the camera or Wi-Fi)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Initialize Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  // Print the assigned IP address once connected
  Serial.println(WiFi.localIP());

  // Set up the camera configuration structure
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  // Assign data pins
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  // Assign clock and sync pins
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  // Assign I2C communication pins for the camera sensor
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  // Assign power and reset pins
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  // Set the camera operational parameters
  config.xclk_freq_hz = 10000000;      // 10 MHz XCLK frequency
  config.pixel_format = PIXFORMAT_JPEG; // Capture images in JPEG format

  // Define resolution and quality settings
  config.frame_size = FRAMESIZE_QVGA;  // QVGA resolution (320x240)
  config.jpeg_quality = 12;            // JPEG quality (lower number = higher quality)
  config.fb_count = 1;                 // Number of frame buffers

  // Initialize the camera hardware with the defined configuration
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera fail");
    return;
  }

  // Start the HTTP server to stream the camera feed
  startCameraServer();
}

void loop() {
  // The camera server handles requests asynchronously, so loop remains empty
}