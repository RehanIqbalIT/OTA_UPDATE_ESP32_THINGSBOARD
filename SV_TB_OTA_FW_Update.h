/*
  SV_TB_OTA_FW_Update.h - Library for flashing Morse code.
  Created by Rehan Iqbal, October 31, 2024.
*/
#ifndef SVTBOTAFWUPDATE_H
#define SVTBOTAFWUPDATE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32httpUpdate.h>

class SV_TB_OTA_FW_Update
{
  public:
    SV_TB_OTA_FW_Update(String deviceToken,String serverName,String Port,bool isSecureServer,String binaryName,String binaryVersion);
    void OTA_Update();
  private:
TaskHandle_t Task1;
  void gvsend_info(String tsTitle, String tsVersion, String State);
  int updateRetryCount = 3;
  String gsCurr_Title;
  String gsCurr_Version;
  String deviceCred;
  String giServerName;
  String giServerHttpPort;
};

#endif
