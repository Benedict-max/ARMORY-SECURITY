#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Define the I2C LCD address and dimensions
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Initialize I2C LCD
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Initialize software serial for GSM module
SoftwareSerial gsmSerial(2, 3); // RX, TX

// Pin Definitions
#define RELAY_PIN   8
#define ALARM_PIN   9

// Authorized RFID tags
String authorizedUIDs[] = {
  "1234", // Example UID, replace with actual authorized tags
  "ABCD"
};

// Function prototypes
void sendSMS(const String &message);
void unlockDoor();
void triggerAlarm();
String readRFID();

void setup() {
  // Initialize serial communication for Virtual Terminal (RFID)
  Serial.begin(9600);

  // Initialize software serial for GSM
  gsmSerial.begin(9600);

  // Initialize I2C LCD
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.init();
  lcd.backlight();
  lcd.print("System Ready");

  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW); // Ensure door is locked initially
  digitalWrite(ALARM_PIN, LOW); // Ensure alarm is off initially
}

void loop() {
  // Check for RFID data from Virtual Terminal
  String uid = readRFID();
  if (uid != "") {
    // Check if UID is authorized
    bool authorized = false;
    for (String authorizedUID : authorizedUIDs) {
      if (uid == authorizedUID) {
        authorized = true;
        break;
      }
    }

    // Display and process authentication result
    lcd.clear();
    if (authorized) {
      lcd.print("Access Granted");
      unlockDoor();
    } else {
      lcd.print("Access Denied");
      triggerAlarm();
    }

    // Delay to allow the user to see the message
    delay(5000);
    lcd.clear();
    lcd.print("System Ready");
  }

  // Short delay before next loop iteration
  delay(1000);
}

// Function to send SMS via GSM module
void sendSMS(const String &message) {
  gsmSerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(100);
  gsmSerial.println("AT+CMGS=\"+1234567890\""); // Replace with target phone number
  delay(100);
  gsmSerial.println(message);
  delay(100);
  gsmSerial.write(26); // Send Ctrl+Z to indicate end of message
  delay(100);
}

// Function to unlock the door
void unlockDoor() {
  digitalWrite(RELAY_PIN, HIGH); // Activate relay
  sendSMS("Access Granted: Armory Unlocked");
  delay(5000); // Keep door unlocked for 5 seconds
  digitalWrite(RELAY_PIN, LOW); // Deactivate relay (lock door)
}

// Function to trigger the alarm
void triggerAlarm() {
  digitalWrite(ALARM_PIN, HIGH); // Activate alarm
  sendSMS("Access Denied: Unauthorized Attempt");
  delay(5000); // Sound alarm for 5 seconds
  digitalWrite(ALARM_PIN, LOW); // Deactivate alarm
}

// Function to read RFID data from Virtual Terminal
String readRFID() {
  String rfidData = "";
  if (Serial.available() > 0) {
    // Read data from the serial port
    rfidData = Serial.readStringUntil('\n');
    rfidData.trim(); // Remove any leading/trailing whitespace
  }
  return rfidData;
}
