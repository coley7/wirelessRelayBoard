#include <SPI.h>
#include "RF24.h"

#define DEBUG
#define BAUDRATE 115200

#define RADIO_LED 2
#define RECV_LED  3

#define RELAIS_1 22
#define RELAIS_2 24
#define RELAIS_3 26
#define RELAIS_4 28
#define RELAIS_5 30
#define RELAIS_6 32
#define RELAIS_7 34
#define RELAIS_8 36

#define TIME_RELAIS_ON 1000 //time in ms

RF24 radio(7, 8);  // Pin 7 and 8 for the connection
bool radioNumber = 0; // transceiver has the radioNumber = 1
const uint64_t addresses[] = { 0x65646f4e31, 0x65646f4e32}; //addresses for the radio channels
unsigned long triggered = 0; // time in ms, since the last triggered event

int relaisLUT[] =
{
  RELAIS_1,
  RELAIS_2,
  RELAIS_3,
  RELAIS_4,
  RELAIS_5,
  RELAIS_6,
  RELAIS_7,
  RELAIS_8
};

const int relaisChannelMax = sizeof(relaisLUT) / sizeof(relaisLUT[0]);

void setupLEDs()
{
  pinMode(RADIO_LED, OUTPUT);
  pinMode(RECV_LED, OUTPUT);

  digitalWrite(RADIO_LED, HIGH);
  digitalWrite(RECV_LED, HIGH);
}

void setupRelaisSafety()
{
  for(int i = 0; i < relaisChannelMax; i++)
  {
    pinMode(relaisLUT[i], OUTPUT);
    digitalWrite(relaisLUT[i], HIGH);
  }
}

void checkPayload(unsigned long *payload)
{
  if ((*payload > 0) && (*payload <= relaisChannelMax))
  {
    digitalWrite(RECV_LED, LOW);
    digitalWrite(relaisLUT[*payload - 1], LOW);
    triggered = millis();

#ifdef DEBUG
    Serial.print(F("[+] Got event "));
    Serial.println(*payload);
#endif
  }
}

void setup()
{
  setupRelaisSafety();
  setupLEDs();
#ifdef DEBUG
  Serial.begin(BAUDRATE);
#endif

  radio.begin();
  radio.setAutoAck(true);
#ifdef DEBUG
  radio.printDetails();
#endif
  radio.setPALevel(RF24_PA_MAX);

  if (radioNumber) 
  {
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);
  }
  else 
  {
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
  }
  radio.startListening();
}

void loop()
{
  unsigned long payload;
  bool wasSent;

  if ((0 != triggered) && (millis() - triggered > TIME_RELAIS_ON))
      setupRelaisSafety();

  if ( radio.available())
  {
    while (radio.available())
    {
      radio.read( &payload, sizeof(unsigned long) );
    }
    radio.stopListening();
    digitalWrite(RADIO_LED, LOW);
    checkPayload(&payload);


    wasSent = radio.write( &payload, sizeof(unsigned long) );
#ifdef DEBUG
    if (!wasSent)
    {
      Serial.print(F("[-] Sent not possible... \n"));
    }
    else
    {
      Serial.print(F("[+] Sent response back"));
      Serial.println(payload);
    }
#endif
    radio.startListening();

    digitalWrite(RADIO_LED, HIGH);
    digitalWrite(RECV_LED, HIGH);
  }
}
