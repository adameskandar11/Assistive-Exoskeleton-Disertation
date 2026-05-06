#include <MyoWare.h>
MyoWare sensorAbduction;  
MyoWare sensorFlexion;    
const int ENV_ABDUCTION = A0;
const int ENV_FLEXION   = A1;
MyoWare::OutputType outputType = MyoWare::ENVELOPE;
int thresholdAbd = 400;
int thresholdFlx = 400;
const unsigned long LOCKOUT_MS = 600;
unsigned long lastTriggerAbd = 0;
unsigned long lastTriggerFlx = 0;
bool wasFiring = false;
unsigned long lastStatusPrint = 0;
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(10);
  delay(500);

  Serial.println(F("============================================="));
  Serial.println(F(" EMG Intent Detection Test"));
  Serial.println(F(" using MyoWare 2.0 Library"));
  Serial.println(F("============================================="));
  Serial.println(F(" Sensor 1 (A0): Abduction — medial deltoid"));
  Serial.println(F(" Sensor 2 (A1): Flexion — anterior deltoid"));
  Serial.println();
  Serial.println(F(" Commands:"));
  Serial.println(F("   cal       - run calibration routine"));
  Serial.println(F("   raw       - show raw EMG values"));
  Serial.println(F("   stats     - show current thresholds"));
  Serial.println(F("   thr1=400  - set abduction threshold"));
  Serial.println(F("   thr2=400  - set flexion threshold"));
  Serial.println(F("============================================="));
  sensorAbduction.setENVPin(ENV_ABDUCTION);
  sensorFlexion.setENVPin(ENV_FLEXION);
  Serial.println(F(" Sensors initialised."));
  Serial.println(F(" Default threshold: 400 (run 'cal' to set properly)"));
  Serial.println();
  Serial.println(F(" Flex your shoulder muscles to see intent detection..."));
  Serial.println(F("============================================="));
  delay(1500);
}

void loop() {
  handleSerial();
  unsigned long now = millis();
  int emgAbd = sensorAbduction.readSensorOutput(outputType);
  int emgFlx = sensorFlexion.readSensorOutput(outputType);
  bool abdActive = emgAbd > thresholdAbd;
  bool flxActive = emgFlx > thresholdFlx;
  if (abdActive && (now - lastTriggerAbd) > LOCKOUT_MS) {
    lastTriggerAbd = now;
    Serial.println();
    Serial.println(F(">>> ABDUCTION INTENT DETECTED <<<"));
    Serial.print(F("    Medial deltoid signal: "));
    Serial.print(emgAbd);
    Serial.print(F("  (threshold "));
    Serial.print(thresholdAbd);
    Serial.println(F(")"));
    Serial.println();
    wasFiring = true;
  }

  if (flxActive && (now - lastTriggerFlx) > LOCKOUT_MS) {
    lastTriggerFlx = now;
    Serial.println();
    Serial.println(F(">>> FLEXION INTENT DETECTED <<<"));
    Serial.print(F("    Anterior deltoid signal: "));
    Serial.print(emgFlx);
    Serial.print(F("  (threshold "));
    Serial.print(thresholdFlx);
    Serial.println(F(")"));
    Serial.println();
    wasFiring = true;
  }
  if (!abdActive && !flxActive && (now - lastStatusPrint) > 1500) {
    lastStatusPrint = now;
    Serial.print(F("-- resting --   ABD: "));
    Serial.print(emgAbd);
    Serial.print(F("   FLX: "));
    Serial.println(emgFlx);
  }

  delay(20);  // ~50Hz sampling
}

void calibrate() {
  Serial.println();
  Serial.println(F("============================================="));
  Serial.println(F(" CALIBRATION"));
  Serial.println(F("============================================="));
  Serial.println(F(" Step 1: Relax both muscles (3 sec)"));
  delay(3000);
  Serial.println(F(" Recording baseline..."));

  int baselineAbd = 0, baselineFlx = 0;
  for (int i = 0; i < 150; i++) {
    int a = sensorAbduction.readSensorOutput(outputType);
    int f = sensorFlexion.readSensorOutput(outputType);
    if (a > baselineAbd) baselineAbd = a;
    if (f > baselineFlx) baselineFlx = f;
    delay(20);
  }
  Serial.print(F(" Baseline ABD: ")); Serial.println(baselineAbd);
  Serial.print(F(" Baseline FLX: ")); Serial.println(baselineFlx);

  Serial.println();
  Serial.println(F(" Step 2: FLEX abduction muscle (raise arm sideways)"));
  Serial.println(F(" Hold for 3 seconds..."));
  delay(2000);
  Serial.println(F(" Recording..."));

  int peakAbd = 0;
  for (int i = 0; i < 150; i++) {
    int a = sensorAbduction.readSensorOutput(outputType);
    if (a > peakAbd) peakAbd = a;
    delay(20);
  }
  Serial.print(F(" Peak ABD: ")); Serial.println(peakAbd);

  delay(2000);

  Serial.println();
  Serial.println(F(" Step 3: FLEX flexion muscle (raise arm forward)"));
  Serial.println(F(" Hold for 3 seconds..."));
  delay(2000);
  Serial.println(F(" Recording..."));

  int peakFlx = 0;
  for (int i = 0; i < 150; i++) {
    int f = sensorFlexion.readSensorOutput(outputType);
    if (f > peakFlx) peakFlx = f;
    delay(20);
  }
  Serial.print(F(" Peak FLX: ")); Serial.println(peakFlx);
  thresholdAbd = baselineAbd + (int)((peakAbd - baselineAbd) * 0.6);
  thresholdFlx = baselineFlx + (int)((peakFlx - baselineFlx) * 0.6);

  Serial.println();
  Serial.println(F(" CALIBRATION COMPLETE"));
  Serial.print(F(" Abduction threshold: ")); Serial.println(thresholdAbd);
  Serial.print(F(" Flexion threshold:   ")); Serial.println(thresholdFlx);
  Serial.println(F("============================================="));
  Serial.println();
}

void showRaw() {
  Serial.println(F(" Showing raw values for 10 seconds..."));
  unsigned long start = millis();
  while (millis() - start < 10000) {
    int a = sensorAbduction.readSensorOutput(outputType);
    int f = sensorFlexion.readSensorOutput(outputType);
    Serial.print(F(" ABD: ")); Serial.print(a);
    Serial.print(F("   FLX: ")); Serial.println(f);
    delay(100);
  }
  Serial.println(F(" Returning to intent detection..."));
}


void handleSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "cal") {
    calibrate();
  } else if (cmd == "raw") {
    showRaw();
  } else if (cmd == "stats") {
    Serial.print(F(" Threshold ABD: ")); Serial.println(thresholdAbd);
    Serial.print(F(" Threshold FLX: ")); Serial.println(thresholdFlx);
  } else if (cmd.startsWith("thr1=")) {
    thresholdAbd = cmd.substring(5).toInt();
    Serial.print(F(" Threshold ABD set to ")); Serial.println(thresholdAbd);
  } else if (cmd.startsWith("thr2=")) {
    thresholdFlx = cmd.substring(5).toInt();
    Serial.print(F(" Threshold FLX set to ")); Serial.println(thresholdFlx);
  }
}
