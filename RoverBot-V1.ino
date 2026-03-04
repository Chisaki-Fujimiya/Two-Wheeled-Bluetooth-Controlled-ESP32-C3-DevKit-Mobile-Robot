#include <WiFi.h>
#include <WebServer.h>

//Pinout
#define in1 32
#define in2 33
#define in3 26
#define in4 25
#define sensorR 23
#define sensorL 22

//Global Variable
int sensorR_value = 0;
int sensorL_value = 0;
int left = 0;
int right = 0;

  WebServer server(80);
  const char* html = R"rawliteral(
  <!DOCTYPE html>
    <html>
    <head>
    <meta name="viewport" 
        content="width=device-width, initial-scale=1.0, 
                maximum-scale=1.0, user-scalable=no">

    <style>
    body {
        margin: 0;
        background: #111;
        color: white;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        height: 100vh;
        touch-action: none;
        overscroll-behavior: none;
    }

    .container {
        display: flex;
        gap: 80px;
    }

  .slider {
      -webkit-appearance: none;
      appearance: none;
      width: 300px;   /* length */
      height: 80px;   /* thickness */
      background: transparent;
      transform: rotate(-90deg);
  }

  /* TRACK */
  .slider::-webkit-slider-runnable-track {
      width: 350px;
      height: 70px;
      background: linear-gradient(to right, #000000, #ddca3b);
      border-radius: 10px;
      margin: auto;
  }

  /* THUMB */
  .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 70px;
      height: 70px;
      background: #00ffcc;
      border-radius: 50%;
      border: 4px solid #00ffaa;
      box-shadow: 0 0 20px #00ffaa;
      cursor: pointer;
  }

    #values {
        font-size: 25px;
        margin-top: 30px;
    }
    </style>
    </head>

    <body>

    <div class="container">
        <input type="range" min="-255" max="255" value="0" id="leftMotor" class="slider">
        <input type="range" min="-255" max="255" value="0" id="rightMotor" class="slider">
    </div>

    <div id="values">L: 0 | R: 0</div>

    <script>
    const leftSlider = document.getElementById("leftMotor");
    const rightSlider = document.getElementById("rightMotor");
    const values = document.getElementById("values");

    let lastLeft = 0;
    let lastRight = 0;

    function sendMotors() {
        let left = parseInt(leftSlider.value);
        let right = parseInt(rightSlider.value);

        values.innerText = "L: " + left + " | R: " + right;

        if (left !== lastLeft || right !== lastRight) {
            fetch("/Motors?left=" + left + "&right=" + right);
            lastLeft = left;
            lastRight = right;
        }
    }

    leftSlider.addEventListener("input", sendMotors);
    rightSlider.addEventListener("input", sendMotors);
    </script>

    </body>
    </html>
  )rawliteral";

  void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_AP);
    WiFi.softAP("AMES-Roborace_Bot", "Cecilia1");

    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", [](){
      server.send(200, "text/html", html);
    });

    server.on("/Motors", [](){

      left = server.arg("left").toInt();
      right = server.arg("right").toInt();

      Serial.print("Left: ");
      Serial.print(left);
      Serial.print(" | Right: ");
      Serial.println(right);

      server.send(200, "text/plain", "OK");

      // Left Motor
      if(left > 0){ Serial.println("Left Forward"); }
      else if(left < 0){ Serial.println("Left Backward"); }
      else { serial.println("Left Stop"); }

      // Right Motor
      if(right > 0){ Serial.println("Right Forward"); }
      else if(right < 0){ Serial.println("Right Backward"); }
      else { Serial.println("Right Stop"); }
      
    });

    server.begin();
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT);
    pinMode(in4, OUTPUT);
    pinMode(sensorR, INPUT);
    pinMode(sensorL, INPUT);

    pinMode(sensorR, INPUT);
    pinMode(sensorL, INPUT);
  }

  void rightF() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
  }

  void rightB() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
  }

  void leftF() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }

  void leftB() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }

  void steerR() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }

  void steerL() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }
  
  void Forward() {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
  }
  
  void Backward() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
  }

  void loop() {
    server.handleClient();
    sensorR_value = digitalRead(sensorR);
    sensorL_value = digitalRead(sensorL);
    delay(200);

    //Sensor Tracking
    Serial.print("Right: ");
    Serial.print(sensorR_value); 
    Serial.print(" | ");
    Serial.print("Left: "); 
    Serial.println(sensorL_value);

    //Right Motor Single Steering
    if (right > 60) {
      rightF();
    } else if (right < -60) {
      rightB();
    } else {
      digitalWrite(in1, LOW); digitalWrite(in2, LOW);
    }

    //Left Motor Single Steering
    if (left > 60) {
      leftF();
    } else if (left < -60) {
      leftB();
    } else {
      digitalWrite(in3, LOW); digitalWrite(in4, LOW);
    }

    //Tank Track Steering Adjusr
    if (right > 60 && left < -60) {
      steerR();   
    } else if (right < -60 && left > 60) {
      steerL();
    } else if (right > 60 && left > 60) {
      Forward();
    } else if (right < -60 && left < -60) {
      Backward();
    }

  }