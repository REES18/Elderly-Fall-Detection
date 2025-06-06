#include <WiFi.h>
#include <Wire.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// Wi-Fi credentials
const char *ssid = "moto g54 5G";          // Enter your Wi-Fi Name
const char *pass = "123456789";       // Enter your Wi-Fi Password

// Telegram Bot details
const char* BOT_TOKEN = "7347781312:AAErHYLjM45xhKMTmaeX0QEmeT9COA0hm1Y";   // Replace with your Telegram Bot Token
const char* CHAT_ID = "5588436358";               // Replace with your Chat ID

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// MPU-6050 settings
const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
float axOffset = 2050, ayOffset = 77, azOffset = 1947;

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22); // Default I2C pins on ESP32

    // Connect to Wi-Fi
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected");

    // Initialize Telegram Bot
    client.setInsecure(); // Disable SSL certificate validation
    Serial.println("Telegram Bot initialized");

    // Initialize MPU-6050
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0);   // Set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    Serial.println("MPU-6050 Initialized");
}

void loop() {
    mpu_read();

    // Adjust for calibration offsets
    ax = (AcX - axOffset) / 16384.00;
    ay = (AcY - ayOffset) / 16384.00;
    az = (AcZ - azOffset) / 16384.00;

    // Calculate amplitude vector
    float Raw_Amp = sqrt(ax * ax + ay * ay + az * az);
    int Amp = Raw_Amp * 5; // Scaling factor
    Serial.print("Amplitude: ");
    Serial.println(Amp);

    // Print raw sensor values for debugging
    Serial.print("Ax: "); Serial.print(ax, 4);
    Serial.print(" Ay: "); Serial.print(ay, 4);
    Serial.print(" Az: "); Serial.println(az, 4);

    // Detect fall based on amplitude threshold
    if (Amp >= 5) { // Adjusted threshold for fall detection
        Serial.println("FALL DETECTED");
        sendTelegramNotification("Fall Detected! Take action immediately.");
    }

    delay(1000); // Delay for loop
}

void mpu_read() {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true);

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    Tmp = Wire.read() << 8 | Wire.read();
    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();
}

void sendTelegramNotification(String message) {
    if (bot.sendMessage(CHAT_ID, message, "")) {
        Serial.println("Notification sent to Telegram");
    } else {
        Serial.println("Failed to send Telegram notification");
    }
}
