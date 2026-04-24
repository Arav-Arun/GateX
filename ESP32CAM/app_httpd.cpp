#include <Arduino.h>
#include <string.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "camera_index.h"

// Handler for the root URI ("/") to serve the main HTML UI
static esp_err_t index_handler(httpd_req_t *req)
{
  // Send the index_html (defined in camera_index.h) to the client
  return httpd_resp_send(req, index_html, strlen(index_html));
}

// Handler for the "/capture" URI to take and serve a single photo
static esp_err_t capture_handler(httpd_req_t *req)
{
  // Capture a frame from the camera
  camera_fb_t * fb = esp_camera_fb_get();

  // If frame buffer is empty, return failure
  if (!fb) return ESP_FAIL;

  // Set the response type to JPEG image
  httpd_resp_set_type(req, "image/jpeg");
  
  // Send the frame buffer data to the client
  httpd_resp_send(req, (const char *)fb->buf, fb->len);

  // Return the frame buffer to be reused
  esp_camera_fb_return(fb);
  return ESP_OK;
}

// Function to configure and start the HTTP server
void startCameraServer()
{
  // Use default HTTP server configuration
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;

  // Start the HTTP server with the specified configuration
  httpd_start(&server, &config);

  // Define URI handlers for index page and image capture
  httpd_uri_t index_uri = { "/", HTTP_GET, index_handler };
  httpd_uri_t capture_uri = { "/capture", HTTP_GET, capture_handler };

  // Register the URI handlers to the server
  httpd_register_uri_handler(server, &index_uri);
  httpd_register_uri_handler(server, &capture_uri);
}