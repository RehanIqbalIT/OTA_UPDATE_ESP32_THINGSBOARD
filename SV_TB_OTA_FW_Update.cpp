#include "SV_TB_OTA_FW_Update.h"

//#define USE_SERIAL Serial

SV_TB_OTA_FW_Update::SV_TB_OTA_FW_Update(String deviceToken,String serverName,String Port,bool isSecureServer,String binaryName,String binaryVersion){
    gsCurr_Title=binaryName;
    gsCurr_Version=binaryVersion;
    deviceCred=deviceToken;
    giServerName=(isSecureServer?"https://":"http://")+serverName;
    giServerHttpPort=(Port==""?"":":")+Port;
	Serial.print("gsCurr_Title: ");
    Serial.println(gsCurr_Title);
    Serial.print("gsCurr_Version: ");
    Serial.println(gsCurr_Version);
    Serial.print("deviceCred: ");
    Serial.println(deviceCred);
    Serial.print("giServerName: ");
    Serial.println(giServerName);
    Serial.print("giServerHttpPort: ");
    Serial.println(giServerHttpPort);

}
void SV_TB_OTA_FW_Update::OTA_Update() {
	Serial.print("gsCurr_Title: ");
    Serial.println(gsCurr_Title);
    Serial.print("gsCurr_Version: ");
    Serial.println(gsCurr_Version);
    Serial.print("deviceCred: ");
    Serial.println(deviceCred);
    Serial.print("giServerName: ");
    Serial.println(giServerName);
    Serial.print("giServerHttpPort: ");
    Serial.println(giServerHttpPort);

  String tsPayload;
  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;
    String tsServerPath_getinfo = giServerName + giServerHttpPort + "/api/v1/" + deviceCred + "/attributes?sharedKeys=fw_title,fw_version,fw_location";
    Serial.print("getInfo: "+tsServerPath_getinfo);
    http.begin(tsServerPath_getinfo.c_str());

    int tiHttpResponseCode = http.GET();

    if (tiHttpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(tiHttpResponseCode);
      tsPayload = http.getString();
    } else {
      Serial.print("Error code: ");
      Serial.println(tiHttpResponseCode);
      return;
    }
    http.end();

    StaticJsonDocument<2048> doc;
    deserializeJson(doc, tsPayload);

    String tsTitle = doc["shared"]["fw_title"];
    String tsVersion = doc["shared"]["fw_version"];

    Serial.println(tsTitle);
    Serial.println(tsVersion);
    if (tsTitle == "null" || tsVersion == "null" || tsTitle == NULL || tsVersion == NULL)
      return;
    String tsServerPath_Firmware = giServerName + giServerHttpPort + "/api/v1/" + deviceCred + "/firmware?title=" + tsTitle + "&version=" + tsVersion;

    if (gsCurr_Version != tsVersion || gsCurr_Title != tsTitle) {
      t_httpUpdate_return ret = ESPhttpUpdate.update(tsServerPath_Firmware.c_str());

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          //USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          Serial.println("");
          gvsend_info(tsTitle, tsVersion, "FAILED");
          if (updateRetryCount > 0) {
            updateRetryCount--;
            //USE_SERIAL.printf("Retrying update, attempts left: %d", updateRetryCount);
            delay(5000);  // Add a delay before retrying (adjust as needed)
            OTA_Update();
          } else {
            //USE_SERIAL.println("Max retry attempts reached. Giving up.");
          }
          break;

        case HTTP_UPDATE_OK:
          //USE_SERIAL.println("HTTP_UPDATE_OK");
          gvsend_info(tsTitle, tsVersion, "UPDATED");

          ESP.restart();
          break;
      }
    } else {
      gvsend_info(gsCurr_Title, gsCurr_Version, "UPDATED");
    }
  }
}
//-------------------------- Update OTA telemetry ----------------
void SV_TB_OTA_FW_Update::gvsend_info(String tsTitle, String tsVersion, String State) {

  HTTPClient http;
  String tsServerPath_sendinfo = giServerName + giServerHttpPort + "/api/v1/" + deviceCred + "/telemetry";
  http.begin(tsServerPath_sendinfo.c_str());
  DynamicJsonDocument doc_SendInfo(2048);

  if (State == "UPDATED") {
    doc_SendInfo["current_fw_title"] = tsTitle;
    doc_SendInfo["current_fw_version"] = tsVersion;
  } else {
    doc_SendInfo["fw_error"] = "Firmwate Update Failed";
  }

  doc_SendInfo["fw_state"] = State;

  char buff[256];
  serializeJson(doc_SendInfo, buff);

  int tiHttpResponseCode_send = http.POST(buff);

  Serial.print("HTTP_send Response code: ");
  Serial.println(tiHttpResponseCode_send);
  Serial.print("Sending buffer: ");
  Serial.println(buff);

  http.end();
}
