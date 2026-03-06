#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// Pinout
#define in1 32
#define in2 33
#define in3 26
#define in4 25

// Motor values
int left = 0;
int right = 0;

// Servers
WebServer server(80);
WebSocketsServer webSocket(81);

const char* htmlTemplate = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<style>
body { margin:0; background:#111; color:white; display:flex; flex-direction:column; justify-content:center; align-items:center; height:100vh; touch-action:none; overscroll-behavior:none; }
.container { display:flex; gap:80px; }
.slider { -webkit-appearance:none; appearance:none; width:300px; height:80px; background:transparent; transform:rotate(-90deg); }
.slider::-webkit-slider-runnable-track { width:350px; height:70px; background:linear-gradient(to right,#000000,#ddca3b); border-radius:10px; margin:auto; }
.slider::-webkit-slider-thumb { -webkit-appearance:none; appearance:none; width:70px; height:70px; background:#00ffcc; border-radius:50%; border:4px solid #00ffaa; box-shadow:0 0 20px #00ffaa; cursor:pointer; }
#values { font-size:25px; margin-top:30px; }
</style>
</head>
<body>

<div class="container">
<input type="range" min="-255" max="255" value="0" id="leftMotor" class="slider">
<input type="range" min="-255" max="255" value="0" id="rightMotor" class="slider">
</div>

<div id="values">L:0 | R:0</div>

<script>
const wsPort = 81;
let socket;

function startWS(ip){
    socket = new WebSocket("ws://" + ip + ":" + wsPort);
    socket.onopen = () => { console.log("WebSocket connected!"); };
    socket.onerror = (err) => { console.log("WebSocket error", err); };
    socket.onclose = () => { console.log("WebSocket closed!"); };
    socket.onmessage = (msg) => { console.log("From server: ", msg.data); };
}

const leftSlider = document.getElementById("leftMotor");
const rightSlider = document.getElementById("rightMotor");
const values = document.getElementById("values");

let lastLeft = 0;
let lastRight = 0;

function sendMotors(){
    let left = parseInt(leftSlider.value);
    let right = parseInt(rightSlider.value);
    values.innerText = "L: " + left + " | R: " + right;
    if(left !== lastLeft || right !== lastRight){
        if(socket.readyState === WebSocket.OPEN){
            socket.send(left + "," + right);
        }
        lastLeft = left;
        lastRight = right;
    }
}

leftSlider.addEventListener("input", sendMotors);
rightSlider.addEventListener("input", sendMotors);

// Inject IP dynamically
startWS("{{AP_IP}}");
</script>

</body>
</html>
)rawliteral";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if(type == WStype_TEXT){
    String msg = String((char*)payload);
    Serial.print("Message received: "); Serial.println(msg);
    int comma = msg.indexOf(',');
    if(comma > 0){
      left = msg.substring(0,comma).toInt();
      right = msg.substring(comma+1).toInt();
      Serial.print("WS Left: "); Serial.print(left);
      Serial.print(" | Right: "); Serial.println(right);
    }
  }
}

void setup(){
  Serial.begin(115200);

  // WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP("AMES-Roborace_Bot","Cecilia1");
  Serial.println("Access Point Started");
  Serial.print("IP Address: "); Serial.println(WiFi.softAPIP());

  // HTTP server
  server.on("/", [](){
    String page = htmlTemplate;
    page.replace("{{AP_IP}}", WiFi.softAPIP().toString());
    server.send(200,"text/html", page);
  });
  server.begin();

  // WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started on port 81");

  // Motor pins
  pinMode(in1,OUTPUT); pinMode(in2,OUTPUT);
  pinMode(in3,OUTPUT); pinMode(in4,OUTPUT);
}

void rightF(){ digitalWrite(in1,HIGH); digitalWrite(in2,LOW); digitalWrite(in3,LOW); digitalWrite(in4,LOW); }
void rightB(){ digitalWrite(in1,LOW); digitalWrite(in2,HIGH); digitalWrite(in3,LOW); digitalWrite(in4,LOW); }
void leftF(){ digitalWrite(in1,LOW); digitalWrite(in2,LOW); digitalWrite(in3,HIGH); digitalWrite(in4,LOW); }
void leftB(){ digitalWrite(in1,LOW); digitalWrite(in2,LOW); digitalWrite(in3,LOW); digitalWrite(in4,HIGH); }
void steerR(){ digitalWrite(in1,HIGH); digitalWrite(in2,LOW); digitalWrite(in3,LOW); digitalWrite(in4,HIGH); }
void steerL(){ digitalWrite(in1,LOW); digitalWrite(in2,HIGH); digitalWrite(in3,HIGH); digitalWrite(in4,LOW); }
void Forward(){ digitalWrite(in1,HIGH); digitalWrite(in2,LOW); digitalWrite(in3,HIGH); digitalWrite(in4,LOW); }
void Backward(){ digitalWrite(in1,LOW); digitalWrite(in2,HIGH); digitalWrite(in3,LOW); digitalWrite(in4,HIGH); }

void loop(){
  webSocket.loop();       
  server.handleClient();   

  // Right Motor
  if(right > 60){ rightF(); }
  else if(right < -60){ rightB(); }
  else{ digitalWrite(in1,LOW); digitalWrite(in2,LOW); }

  // Left Motor
  if(left > 60){ leftF(); }
  else if(left < -60){ leftB(); }
  else{ digitalWrite(in3,LOW); digitalWrite(in4,LOW); }

  // Tank Steering
  if(right > 60 && left < -60){ steerR(); }
  else if(right < -60 && left > 60){ steerL(); }
  else if(right > 60 && left > 60){ Forward(); }
  else if(right < -60 && left < -60){ Backward(); }
}
