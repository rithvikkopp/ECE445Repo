#define DT A1  // Data pin (DOUT) of HX711
#define SCK A0 // Clock pin (SCK) of HX711

float calibration_factor = 2000; // Adjust based on your setup
unsigned long duration = 60000;   // One minute in milliseconds
unsigned long start_time;

void setup() {
  pinMode(SCK, OUTPUT);
  pinMode(DT, INPUT);
  Serial.begin(9600);
  Serial.println("Load Cell Measurement Starting...");
  delay(1000); // Allow setup stabilization
}

long readHX711() {
  long count = 0;
  while (digitalRead(DT)); // Wait until DOUT goes low
  
  for (int i = 0; i < 24; i++) {
    digitalWrite(SCK, HIGH);
    delayMicroseconds(1);
    count = (count << 1) | digitalRead(DT);
    digitalWrite(SCK, LOW);
    delayMicroseconds(1);
  }
  
  // Extra pulse to set HX711 gain to 128 (channel A)
  digitalWrite(SCK, HIGH);
  delayMicroseconds(1);
  digitalWrite(SCK, LOW);
  
  if (count & 0x800000) { // Check if the 24th bit is high (negative value)
    count |= 0xFF000000; // Sign extend to 32 bits
  }
  return count;
}

void loop() {
  start_time = millis();
  long sum = 0;
  int count = 0;

  while (millis() - start_time < duration) {
    long rawValue = readHX711();
    float weight = (rawValue / calibration_factor); // Apply calibration
    sum += weight;
    count++;
    Serial.print("Weight: ");
    Serial.println(weight);
    delay(500); // Adjust delay to control reading frequency
  }

  float averageWeight = sum / count;
  Serial.println("Measurement complete!");
  Serial.print("Average Weight: ");
  Serial.println(averageWeight);
  delay(10000); // Pause before the next run
}
