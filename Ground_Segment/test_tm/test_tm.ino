#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_SleepyDog.h>

#include "bus_packet.h"
#include "tf_packet.h"

#include <WiFi.h>
#include <WiFiClient.h>

const char *ssid = "FyCUS23";
const char *password = "Parihuela1notfound";
const int serverPort = 2023;

WiFiServer server(serverPort);
WiFiClient client;


#define LORA_DEBUG  0
#define TIME_DOT    500


unsigned long int t;

#ifndef ARDUINO_SAMD_MKRWAN1300 || ARDUINO_SAMD_MKRWAN1310
  #define SCK     5
  #define MISO    19
  #define MOSI    27
  #define SS      18
  #define RST     14
  #define DIO0    26
#endif

#define BAND    433E6
                //433E6
                //866E6
                //915E6


/* WDT TIME */
#define WDT_TIME          10000



// Set your Static IP address
IPAddress local_IP(192, 168, 1, 1);
 IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);



void LoraRx ();
void LoraTx ();


void setup() { 
  Serial.begin(115200);

  Serial.println("LoRa Ground Station");
  
#ifndef ARDUINO_SAMD_MKRWAN1300 || ARDUINO_SAMD_MKRWAN1310
  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
#endif
  if (!LoRa.begin(BAND)) 
  {
    while (1)
    {
      Serial.println(F("*** ERROR LoRa ***"));
      delay(1000);
    }
  }
  
  else
  {                                  
    Serial.println(F("LoRa INITIALIZATION SUCCESSFUL"));
    
#if LORA_DEBUG
    LoRa.setSignalBandwidth(500E3);
    LoRa.setSpreadingFactor(7);
    LoRa.setCodingRate4(5);
    LoRa.setTxPower(20);
    LoRa.setGain(0);
    LoRa.setSyncWord(0x12);

#else
    LoRa.setSignalBandwidth(500E3);
    LoRa.setSpreadingFactor(12);
    LoRa.setCodingRate4(8);
    LoRa.setTxPower(20);
    LoRa.setGain(0);
    LoRa.setSyncWord(0x12);
#endif
  }


  // Configures static IP address
  if (!WiFi.softAPConfig(local_IP, local_IP, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  WiFi.softAP(ssid, password);

  Serial.println("Conectado a la red WiFi");
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("IP gateway: ");
  Serial.println(WiFi.gatewayIP());


  server.begin();
  Serial.print("Servidor iniciado en el puerto: ");
  Serial.println(serverPort);

  
  t=millis();
  Serial.print("Esperando paquete");
  Watchdog.enable(WDT_TIME);

}



void loop() 
{
  client = server.available();
  uint8_t data[100] = {0};
  
  if (client) {
    Serial.println("Cliente conectado");
    while (client.connected()) {
      LoraRx();
      if (client.available()) 
      {
        LoraTx();
      }
      Watchdog.reset();
    }
    Watchdog.reset();
    
  }
  else
  {
    Serial.println("NO HAY CLIENTES CONECTADOS");
    delay(3000);
  }

    Watchdog.reset();
}

/*
 * 10110001, 10000100, 11111100, 110000, APID_TC_ARE_YOU_ALIVE
 * 10110000, 10000111, 110000, 11111, 101, 100100, 1100110, APID_TC_SET_PROGRMMED_TELEMETRY {0b00110000,0b00011111,5}
 * 10100001, 10000100, 11111111, 1000011 APID_TC_REQUIRED_REPORT_OBC
 */
void LoraTx()
{
  Serial.print("ENVIANDO PAQUETE");
  uint8_t tf_buffer_out[TF_PACKET_MAX_SIZE] = {0};
  uint8_t sp_data[BUS_PACKET_DATA_SIZE] = {0};
  uint8_t sp_buffer_out[BUS_PACKET_DATA_SIZE] = {0};
  static uint8_t packet_count[2] = {0x00, 0x01};

  uint16_t i = 0;
  for(; client.available(); i++)
  {
    sp_buffer_out[i] = client.read();
  }

  static tfph_packet_t tx_tfph = {0};
  static tfdf_packet_t tx_tfdf = {0};

  tx_tfph.end_flag = TF_PACKET_NOT_TRUNCATED;
  tx_tfph.tfvn = TF_PACKET_TFVN;
  tx_tfph.mapid = TF_PACKET_DEFAULT_MAPID;
  tx_tfph.vcid = TF_PACKET_DEFAULT_VCID;
  tx_tfph.scid = TF_PACKET_DEFAULT_SCID;
  tx_tfph.source_dest_id = TF_PACKET_DESTINATION;

  if(packet_count[1] == 0xFF)
  {
    packet_count[0]++;
    packet_count[1] = 0x00;
  }
  else packet_count[1]++;
  
  tf_packet_SetData(sp_buffer_out, (sp_buffer_out[1] & 0b01111111), packet_count, 2, &tx_tfph, &tx_tfdf);
  tf_packet_Packetize(0, &tx_tfph, &tx_tfdf, tf_buffer_out);


  Serial.print("\nDatos enviados:\n");
  for(int i=0; i<tx_tfph.length; i++)
  {
    Serial.print(tf_buffer_out[i], DEC);
    Serial.print(", ");
  }
  Serial.print("\n");

}



void LoraRx ()
{
  static uint32_t t = millis();
  static uint8_t data1[42] = {0,1*255+42,39,8,108,203,104,69,78,245,0,22,68,87,154,153,136,66,205,204,60,66,62,6,107,247,204,225,0,0,0,0,0,0,19,255,187,65,0,0,0xe7,0x5b};
  static uint8_t data2[42] = {0,1*255+42,39,8,85,203,104,69,78,12,1,22,68,87,102,102,90,66,205,204,60,66,120,6,179,246,13,226,0,0,0,0,0,0,2,232,187,65,0,0,0x38,0x9a};
  static uint8_t data3[42] = {0,1*255+42,39,8,80,203,104,69,78,10,1,22,68,87,154,153,79,66,205,204,60,66,175,6,237,246,2,226,0,0,0,0,0,0,232,208,189,65,0,0,0xc3,0xa2};
  static uint8_t data4[42] = {0,1*255+42,39,8,80,203,104,69,78,10,1,22,68,87,154,153,79,66,205,204,60,66,175,6,237,246,2,226,0,0,0,0,0,0,232,208,189,65,0,0,0xc3,0xa2};
  static uint8_t data5[42] = {0,1*255+42,39,8,87,203,104,69,78,4,1,22,68,87,0,0,88,66,205,204,60,66,117,6,142,247,233,225,0,0,0,0,0,0,109,31,186,65,0,0,0xe2,0xbc};
  static uint8_t data6[42] = {0,1*255+42,39,8,64,203,104,69,78,212,0,22,68,87,205,204,80,66,205,204,60,66,167,7,105,253,227,220,0,0,0,0,0,0,207,21,184,65,0,0,0x27,0xf7};
  static uint8_t data7[42] = {0,1*255+42,39,8,57,203,104,69,78,216,0,22,68,87,0,0,0,0,0,0,0,0,65,0,70,252,48,224,0,0,0,0,0,0,168,240,212,65,0,0,0x69,0xad};
  static uint8_t data8[42] = {0,1*255+42,39,8,189,202,104,69,78,60,253,11,68,87,0,0,183,66,205,204,60,66,8,16,8,251,100,228,0,0,0,0,0,0,197,24,4,66,0,0,0xb9,0x30};
  static uint8_t data9[42] = {0,1*255+42,39,8,2,203,104,69,78,158,1,22,68,87,0,0,0,0,0,0,0,0,56,0,87,252,45,224,0,0,0,0,0,0,1,54,219,65,0,0,0x11,0x9d};
  static uint8_t data[42] = {0};

  uint8_t ran = random(0,9);
  if(t > millis()) return;
  else t = millis()+3000;

  switch(ran)
  {
    case 1:
        memcpy(data,data1,42);
        break;
    case 2:
        memcpy(data,data2,42);
        break;
    case 3:
        memcpy(data,data3,42);
        break;
    case 4:
        memcpy(data,data4,42);
        break;
    case 5:
        memcpy(data,data5,42);
        break;
    case 6:
        memcpy(data,data6,42);
        break;
    case 7:
        memcpy(data,data7,42);
        break;
    case 8:
        memcpy(data,data8,42);
        break;
    case 9:
        memcpy(data,data9,42);
        break;
  
  }

  client.write(data, 42);
  
}