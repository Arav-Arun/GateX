#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// --- WIFI CONFIGURATION ---
const char* ssid = "Galaxy A32 EB94";
const char* password = "fare1840";

// Initialize WebServer on port 80
WebServer server(80);

// --- RFID CONFIGURATION ---
#define SS_PIN 5
#define RST_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);

// --- IR SENSOR CONFIGURATION ---
#define IR1 27

// --- SERVO MOTOR CONFIGURATION ---
Servo gate;

// --- DATA VARIABLES ---
String uid="Waiting...";
String vehicle="-";
String toll="-";
String message="Waiting...";

String entryTime="-";
String gateTime="-";
String exitTime="-";

// --- HISTORY STORAGE ---
String historyHTML = "";

// --- STATE FLAGS ---
bool carPresent = false;
bool rfidDone = false;
bool gateOpen = false;

// --- TIMER VARIABLES ---
unsigned long gateOpenTime = 0;

// Helper function to return current uptime in seconds
String nowTime(){
  return String(millis()/1000) + "s";
}

// API Handler to serve current toll data as JSON
void handleData(){
  String json="{";
  json+="\"uid\":\""+uid+"\",";
  json+="\"vehicle\":\""+vehicle+"\",";
  json+="\"toll\":\""+toll+"\",";
  json+="\"message\":\""+message+"\",";
  json+="\"entry\":\""+entryTime+"\",";
  json+="\"gate\":\""+gateTime+"\",";
  json+="\"exit\":\""+exitTime+"\"";
  json+="}";
  server.send(200,"application/json",json);
}

// Handler to serve the history HTML fragment
void handleHistory(){
  server.send(200,"text/html",historyHTML);
}

// UI
void handleUI(){

String html = R"rawliteral(
<!DOCTYPE html>
<html>
<body style="background:#0f172a;color:white;text-align:center;font-family:sans-serif">

<h1>GateX Control Panel</h1>

<div style="border:2px solid white;padding:20px;margin:20px">
<p id="msg">Waiting...</p>
<p id="entry">Entry: -</p>
<p id="uid">UID: -</p>
<p id="vehicle">Vehicle: -</p>
<p id="toll">Toll: -</p>
<p id="gate">Gate: -</p>
<p id="exit">Exit: -</p>
</div>

<h2>History</h2>
<div id="history"></div>

<script>
async function update(){
  const res = await fetch('/data');
  const d = await res.json();

  document.getElementById("msg").innerText = d.message;
  document.getElementById("entry").innerText = "Entry: " + d.entry;
  document.getElementById("uid").innerText = "UID: " + d.uid;
  document.getElementById("vehicle").innerText = "Vehicle: " + d.vehicle;
  document.getElementById("toll").innerText = "Toll: " + d.toll;
  document.getElementById("gate").innerText = "Gate: " + d.gate;
  document.getElementById("exit").innerText = "Exit: " + d.exit;

  const hist = await fetch('/history?t=' + Date.now()); // prevent cache
  const h = await hist.text();
  document.getElementById("history").innerHTML = h;
}

setInterval(update,1000);
</script>

</body>
</html>
)rawliteral";

server.send(200,"text/html",html);
}

// SETUP
void setup(){

  Serial.begin(115200);
  delay(1000);
  Serial.println("🚀 START");

  pinMode(IR1, INPUT_PULLUP);

  SPI.begin(18,19,23,5);
  rfid.PCD_Init();

  gate.attach(25);
  gate.write(0);

  WiFi.begin(ssid,password);

  Serial.print("Connecting WiFi");
  int retry = 0;

  while(WiFi.status()!=WL_CONNECTED && retry < 20){
    delay(500);
    Serial.print(".");
    retry++;
  }

  if(WiFi.status()==WL_CONNECTED){
    Serial.println("\n✅ WiFi Connected");
    Serial.print("🌐 URL: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi Failed");
  }

  server.on("/",handleUI);
  server.on("/data",handleData);
  server.on("/history",handleHistory);

  server.begin();
}

// LOOP
void loop(){

  server.handleClient();

  // ENTRY DETECTION
  if(digitalRead(IR1)==LOW && !carPresent){
    delay(100);
    if(digitalRead(IR1)==LOW){
      carPresent = true;
      rfidDone = false;

      entryTime = nowTime();
      message = "Vehicle Detected";

      Serial.println("ENTRY DETECTED");
    }
  }

  // RFID
  if(carPresent && !rfidDone && rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){

    String u="";
    for(byte i=0;i<rfid.uid.size;i++){
      if(rfid.uid.uidByte[i]<0x10) u+="0";
      u+=String(rfid.uid.uidByte[i],HEX);
    }
    u.toUpperCase();

    uid=u;
    Serial.println("UID: " + u);

    if(u=="FD5CC098"){
      vehicle="Light";
      toll="₹50";
      message="Light Vehicle";
    }
    else if(u=="8D613698"){
      vehicle="Heavy";
      toll="₹150";
      message="Heavy Vehicle";
    }
    else if(u=="BD20DD98"){
      vehicle="Emergency";
      toll="FREE";
      message="Emergency Vehicle";
    }
    else{
      vehicle="Blocked";
      toll="Denied";
      message="Access Denied";
    }

    if(vehicle != "Blocked"){
      gate.write(90);
      gateTime = nowTime();
      Serial.println("GATE OPEN");

      gateOpen = true;
      gateOpenTime = millis();
    }

    rfidDone = true;
  }

  // AUTO CLOSE (NON-BLOCKING)
  if(gateOpen && millis() - gateOpenTime > 6000){

    gate.write(0);
    Serial.println("GATE CLOSED");

    exitTime = nowTime();
    message="Vehicle Passed";

    historyHTML =
      String("<div style='border:1px solid white;margin:10px;padding:10px'>") +
      "<b>" + uid + "</b><br>" +
      vehicle + " | " + toll + "<br>" +
      "Entry: " + entryTime + "<br>" +
      "Gate: " + gateTime + "<br>" +
      "Exit: " + exitTime +
      "</div><hr>" + historyHTML;

    // limit memory
    if(historyHTML.length() > 3000){
      historyHTML = historyHTML.substring(0, 3000);
    }

    uid="Waiting...";
    vehicle="-";
    toll="-";
    entryTime="-";
    gateTime="-";
    exitTime="-";

    carPresent=false;
    rfidDone=false;
    gateOpen=false;
  }
}