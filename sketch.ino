#define FLOW_PIN 34
#define TRIG_PIN 5
#define ECHO_PIN 18

#define GREEN_LED 23
#define YELLOW_LED 22
#define RED_LED 21

#define BUZZER 25
#define SERVO_PIN 26

// Set this to your max container depth in cm (e.g., 200cm or 400cm)
const float MAX_DEPTH = 200.0;

float previousLevel = 0;
unsigned long previousTime = 0;

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  ledcAttach(SERVO_PIN, 50, 16);
  servoWrite(0);

  previousTime = millis();
  Serial.println("SMART DRAINAGE MANAGEMENT SYSTEM");
}

void servoWrite(int angle) {
  int duty = map(angle, 0, 180, 1638, 8192);
  ledcWrite(SERVO_PIN, duty);
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return -1;

  return (duration * 0.0343) / 2.0;
}

void loop() {
  unsigned long currentTime = millis();
  
  // Runs update code once every 1 second cleanly
  if (currentTime - previousTime >= 1000) {
    float timeTaken = (currentTime - previousTime) / 1000.0;
    
    float distance = getDistance();

    if (distance < 0) {
      Serial.println("Ultrasonic Sensor Error: Check VCC Power Wire!");
      previousTime = currentTime;
      return;
    }

    // Calculate level percentage using MAX_DEPTH
    float waterLevel = ((MAX_DEPTH - distance) / MAX_DEPTH) * 100.0;
    waterLevel = constrain(waterLevel, 0, 100);

    int flowValue = analogRead(FLOW_PIN);
    int flow = map(flowValue, 0, 4095, 0, 100);

    float riseRate = (waterLevel - previousLevel) / timeTaken;

    previousLevel = waterLevel;
    previousTime = currentTime;

    bool blockage = (waterLevel >= 85 && flow <= 30);
    bool overflow = (waterLevel >= 65 && riseRate >= 1.0);

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);

    if (blockage) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(BUZZER, HIGH);
      servoWrite(90);
      Serial.println("STATUS: BLOCKAGE DETECTED");
    } 
    else if (overflow) {
      digitalWrite(YELLOW_LED, HIGH)
      digitalWrite(BUZZER, HIGH);
      servoWrite(60);
      Serial.println("STATUS: OVERFLOW PREDICTED");
    } 
    else if (waterLevel >= 85) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(BUZZER, HIGH);
      servoWrite(90);
      Serial.println("STATUS: CRITICAL WATER LEVEL");
    } 
    else if (waterLevel >= 65) {
      digitalWrite(YELLOW_LED, HIGH);
      servoWrite(30);
      Serial.println("STATUS: WARNING - HIGH WATER LEVEL");
    } 
    else {
      digitalWrite(GREEN_LED, HIGH);
      servoWrite(0);
      Serial.println("STATUS: NORMAL");
    }

    Serial.print("Water Level: ");
    Serial.print(waterLevel);
    Serial.println(" %");

    Serial.print("Water Flow: ");
    Serial.print(flow);
    Serial.println(" %");

    Serial.print("Level Rise: ");
    Serial.print(riseRate);
    Serial.println(" %/sec");
    Serial.println("---------------------------");
  }
}
