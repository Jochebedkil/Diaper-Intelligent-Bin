#define TRIG_PIN 11     // Trigger pin of the ultrasonic sensor
#define ECHO_PIN 10     // Echo pin of the ultrasonic sensor
#define LED_PIN 9   
#define BLUE_LED 8    // Pin for the LED strip
#define BUZZER_PIN 12   // Pin for the buzzer
#define MAX_DISTANCE 20 // Maximum valid distance in cm
#define THRESHOLD_DISTANCE 4 // Distance threshold for bin full (in cm)

// Timing variables
unsigned long alertStartTime = 0;     // Time when the alert starts
unsigned long pauseStartTime = 0;     // Time when the pause phase starts
const unsigned long alertDuration = 30000;   // 30 seconds in milliseconds
const unsigned long pauseDuration = 60000;   // 60 seconds in milliseconds

// State variables
enum SystemState { IDLE, ALERT, PAUSE };
SystemState currentState = IDLE;

void setup() {
  pinMode(BLUE_LED, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600); // For debugging
}

void loop() {
  digitalWrite (BLUE_LED,HIGH);
  int distance = measureDistance();

  // Debugging: Print the distance
  if (distance > 0 && distance <= MAX_DISTANCE) {
    Serial.print("Distance: ");
    Serial.println(distance);
  } else {
    Serial.println("Invalid distance.");
  }

  unsigned long currentTime = millis();

  switch (currentState) {
    case IDLE:
      if (distance > 0 && distance <= THRESHOLD_DISTANCE) {
        // Transition to ALERT state
        currentState = ALERT;
        alertStartTime = currentTime;
        activateAlerts();
        Serial.println("State: ALERT");
      }
      break;

    case ALERT:
      if (currentTime - alertStartTime >= alertDuration) {
        // Transition to PAUSE state
        deactivateAlerts();
        currentState = PAUSE;
        pauseStartTime = currentTime;
        Serial.println("State: PAUSE");
      }
      break;

    case PAUSE:
      if (distance > THRESHOLD_DISTANCE) {
        // Reset to IDLE if bin is emptied
        currentState = IDLE;
        Serial.println("State: IDLE (Bin emptied)");
      } else if (currentTime - pauseStartTime >= pauseDuration) {
        // Check bin state before reactivating alerts
        if (distance > 0 && distance <= THRESHOLD_DISTANCE) {
          currentState = ALERT;
          alertStartTime = currentTime;
          activateAlerts();
          Serial.println("State: ALERT (Bin still full)");
        } else {
          // Reset to IDLE if the bin is no longer full
          currentState = IDLE;
          Serial.println("State: IDLE (Bin no longer full)");
        }
      }
      break;
  }

  delay(500); // Stabilization delay
}

// Function to measure distance using the ultrasonic sensor
int measureDistance() {
  long duration;
  int distance;

  // Trigger the ultrasonic sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the echo pin and calculate the distance
  duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout after 30 ms
  if (duration == 0) {
    return -1; // Return -1 for invalid readings
  }

  distance = duration * 0.034 / 2; // Convert to cm
  return distance;
}

// Function to activate LED and buzzer
void activateAlerts() {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 1100); // Generate a 1 kHz tone
  Serial.println("Alerts activated!");
}

// Function to deactivate LED and buzzer
void deactivateAlerts() {
  digitalWrite(LED_PIN, LOW);
  noTone(BUZZER_PIN);
  Serial.println("Alerts deactivated.");
}
