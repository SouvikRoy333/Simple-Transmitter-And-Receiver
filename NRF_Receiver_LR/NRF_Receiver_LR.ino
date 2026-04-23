#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// Must match the transmitter address exactly
const uint64_t pipeIn = 0xF9E8F0F0E1LL; 

RF24 radio(8, 9); // CE, CSN pins

// Struct must match the transmitter's PacketData exactly
struct PacketData 
{
  byte lxAxisValue; // Used for Servo (0-254)
  byte lyAxisValue; // Used for ESC (0-254)
};
PacketData data;

Servo steeringServo;  // Servo object for X-axis
Servo brushedESC;     // ESC object for Y-axis (treated like a servo)

void setup()
{
  // Attach components to specified pins
  steeringServo.attach(5);
  brushedESC.attach(6);

  // Initialize NRF24L01
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipeIn);
  radio.startListening(); // Set as Receiver

  // Set initial neutral positions (127 maps to roughly 90 degrees)
  steeringServo.write(90);
  brushedESC.write(90);
}

void loop()
{
  if (radio.available())
  {
    radio.read(&data, sizeof(PacketData));

    // 1. Map X-Axis (lxAxisValue) to Servo (0 to 180 degrees)
    int servoPos = map(data.lxAxisValue, 0, 254, 0, 180);
    steeringServo.write(servoPos);

    // 2. Map Y-Axis (lyAxisValue) to ESC (0 to 180 for Servo library)
    // 90 is neutral/stop, 180 is full forward, 0 is full reverse
    int escSpeed = map(data.lyAxisValue, 0, 254, 0, 180);
    brushedESC.write(escSpeed);
  }
}