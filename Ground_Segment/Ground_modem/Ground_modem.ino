// #include <SPI.h>
// #include <LoRa.h>
// #include <Adafruit_SleepyDog.h>

// #include "bus_packet.h"
// #include "tf_packet.h"

// #include <WiFi.h>
// #include <WiFiClient.h>

// const char *ssid = "NAVE-RUBEN";
// const char *password = "G/912r54";
// const int serverPort = 2023;

// WiFiServer server(serverPort);
// WiFiClient client;


// #define LORA_DEBUG  0
// #define TIME_DOT    500


// unsigned long int t;

// #ifndef ARDUINO_SAMD_MKRWAN1300 || ARDUINO_SAMD_MKRWAN1310
//   #define SCK     5
//   #define MISO    19
//   #define MOSI    27
//   #define SS      18
//   #define RST     14
//   #define DIO0    26
// #endif

// #define BAND    433E6
//                 //433E6
//                 //866E6
//                 //915E6


// /* WDT TIME */
// #define WDT_TIME          10000



// // Set your Static IP address
// IPAddress local_IP(192, 168, 137, 17);
// // Set your Gateway IP address
// IPAddress gateway(192, 168, 137, 1);

// IPAddress subnet(255, 255, 255, 255);



// void LoraRx ();
// void LoraTx ();


// void setup() { 
//   Serial.begin(115200);

//   Serial.println("LoRa Ground Station");
  
// #ifndef ARDUINO_SAMD_MKRWAN1300 || ARDUINO_SAMD_MKRWAN1310
//   //SPI LoRa pins
//   SPI.begin(SCK, MISO, MOSI, SS);
//   //setup LoRa transceiver module
//   LoRa.setPins(SS, RST, DIO0);
// #endif
//   if (!LoRa.begin(BAND)) 
//   {
//     while (1)
//     {
//       Serial.println(F("*** ERROR LoRa ***"));
//       delay(1000);
//     }
//   }
  
//   else
//   {                                  
//     Serial.println(F("LoRa INITIALIZATION SUCCESSFUL"));
    
// #if LORA_DEBUG
//     LoRa.setSignalBandwidth(500E3);
//     LoRa.setSpreadingFactor(7);
//     LoRa.setCodingRate4(5);
//     LoRa.setTxPower(20);
//     LoRa.setGain(0);
//     LoRa.setSyncWord(0x12);

// #else
//     LoRa.setSignalBandwidth(500E3);
//     LoRa.setSpreadingFactor(12);
//     LoRa.setCodingRate4(8);
//     LoRa.setTxPower(20);
//     LoRa.setGain(0);
//     LoRa.setSyncWord(0x12);
// #endif
//   }


//   // Configures static IP address
//   if (!WiFi.config(local_IP, gateway, subnet)) {
//     Serial.println("STA Failed to configure");
//   }
//     WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Conectando a la red WiFi...");
//   }


//   Serial.println("Conectado a la red WiFi");
//   Serial.println("");
//   Serial.print("Connected to ");
//   Serial.println(ssid);
//   Serial.print("IP address: ");
//   Serial.println(WiFi.localIP());
//   Serial.print("IP gateway: ");
//   Serial.println(WiFi.gatewayIP());


//   server.begin();
//   Serial.print("Servidor iniciado en el puerto: ");
//   Serial.println(serverPort);

  
//   t=millis();
//   Serial.print("Esperando paquete");
//   Watchdog.enable(WDT_TIME);

// }



// void loop() 
// {
//   client = server.available();
//   uint8_t data[100] = {0};
  
//   if (client) {
//     Serial.println("Cliente conectado");
//     while (client.connected()) {
//       LoraRx();
//       if (client.available()) 
//       {
//         LoraTx();
//       }
//       Watchdog.reset();
//     }
//     Watchdog.reset();
    
//   }
//   else
//   {
//     Serial.println("NO HAY CLIENTES CONECTADOS");
//     delay(3000);
//   }

//     Watchdog.reset();
// }

// /*
//  * 10110001, 10000100, 11111100, 110000, APID_TC_ARE_YOU_ALIVE
//  * 10110000, 10000111, 110000, 11111, 101, 100100, 1100110, APID_TC_SET_PROGRMMED_TELEMETRY {0b00110000,0b00011111,5}
//  * 10100001, 10000100, 11111111, 1000011 APID_TC_REQUIRED_REPORT_OBC
//  */
// void LoraTx()
// {
//   Serial.print("ENVIANDO PAQUETE");
//   uint8_t tf_buffer_out[TF_PACKET_MAX_SIZE] = {0};
//   uint8_t sp_data[BUS_PACKET_DATA_SIZE] = {0};
//   uint8_t sp_buffer_out[BUS_PACKET_DATA_SIZE] = {0};
//   static uint8_t packet_count[2] = {0x00, 0x01};

//   uint16_t i = 0;
//   for(; client.available(); i++)
//   {
//     sp_buffer_out[i] = client.read();
//   }

//   static tfph_packet_t tx_tfph = {0};
//   static tfdf_packet_t tx_tfdf = {0};

//   tx_tfph.end_flag = TF_PACKET_NOT_TRUNCATED;
//   tx_tfph.tfvn = TF_PACKET_TFVN;
//   tx_tfph.mapid = TF_PACKET_DEFAULT_MAPID;
//   tx_tfph.vcid = TF_PACKET_DEFAULT_VCID;
//   tx_tfph.scid = TF_PACKET_DEFAULT_SCID;
//   tx_tfph.source_dest_id = TF_PACKET_DESTINATION;

//   if(packet_count[1] == 0xFF)
//   {
//     packet_count[0]++;
//     packet_count[1] = 0x00;
//   }
//   else packet_count[1]++;
  
//   tf_packet_SetData(sp_buffer_out, (sp_buffer_out[1] & 0b01111111), packet_count, 2, &tx_tfph, &tx_tfdf);
//   tf_packet_Packetize(0, &tx_tfph, &tx_tfdf, tf_buffer_out);

//   LoRa.beginPacket();
//   LoRa.write(tf_buffer_out, tx_tfph.length);
//   LoRa.endPacket();

//   Serial.print("\nDatos enviados:\n");
//   for(int i=0; i<tx_tfph.length; i++)
//   {
//     Serial.print(tf_buffer_out[i], DEC);
//     Serial.print(", ");
//   }
//   Serial.print("\n");

// }



// void LoraRx ()
// {
//   static uint8_t tf_buffer_in[TF_PACKET_MAX_SIZE] = {0};
//   static uint8_t sp_data[BUS_PACKET_DATA_SIZE] = {0};
//   static uint8_t sp_buffer_in[BUS_PACKET_DATA_SIZE] = {0};
//   static bus_packet_t rx_packet = {0};
//   static tfph_packet_t rx_tfph = {0};
//   static tfdf_packet_t rx_tfdf = {0};


//   int packet_length = LoRa.parsePacket();  
//   if (packet_length) 
//   {
//     Serial.print("\nReceived packet:  ");

//       LoRa.readBytes(tf_buffer_in, packet_length);
//       for(int i=0; i<packet_length; i++)
//       {
//         Serial.print(tf_buffer_in[i], DEC);
//         Serial.print(", ");
//       }
//       Serial.print("\n");
      
//     Serial.print("\t - Length:  ");
//       Serial.print(packet_length);
//       Serial.print(" bytes\r\n");
      
//     Serial.print("\t - RSSI: ");
//       Serial.print(LoRa.packetRssi());
//       Serial.print(" dBm\r\n");
      
//     Serial.print("\t - SNR:  ");
//       Serial.print(LoRa.packetSnr());
//       Serial.print(" dB\r\n");
      
//     Serial.print("\t - Freq. Error:  ");
//       Serial.print(LoRa.packetFrequencyError());
//       Serial.print(" Hz\r\n\n");


//     if (tf_packet_Decode(tf_buffer_in, packet_length, &rx_tfph, &rx_tfdf) != HAL_OK)      
//     {
//       Serial.print("Fallo en Decode TF USLP\r\n");
//       return;
//     }
    
//     client.write(rx_tfdf.data, rx_tfph.length-rx_tfph.vc_length-TF_PACKET_ECF_SIZE-TF_PACKET_DATA_HEADER_SIZE-TF_PACKET_PRIMARY_TRUNCATED_HEADER_SIZE+TF_PACKET_PRIMARY_BASE_HEADER_SIZE);





//     if (bus_packet_Decode(rx_tfdf.data, &rx_packet) != HAL_OK)                       Serial.print("Fallo en Decode Space Packet\r\n");
//     else
//     {
//       char header[50];
//       sprintf(header, "Packet Type: %d, APID: %d, Length: %d", rx_packet.packet_type, rx_packet.apid, rx_packet.length);
//       Serial.println(header);
//       for(int i=0; i<(rx_packet.length-BUS_PACKET_HEADER_SIZE-BUS_PACKET_ECF_SIZE); i++)
//       {
//         Serial.print(rx_packet.data[i], DEC);
//         Serial.print(", ");
//       }
//       Serial.print("\n");
//       digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//     }
//   }
  
// }




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

  LoRa.beginPacket();
  LoRa.write(tf_buffer_out, tx_tfph.length);
  LoRa.endPacket();

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
  static uint8_t tf_buffer_in[TF_PACKET_MAX_SIZE] = {0};
  static uint8_t sp_data[BUS_PACKET_DATA_SIZE] = {0};
  static uint8_t sp_buffer_in[BUS_PACKET_DATA_SIZE] = {0};
  static bus_packet_t rx_packet = {0};
  static tfph_packet_t rx_tfph = {0};
  static tfdf_packet_t rx_tfdf = {0};


  int packet_length = LoRa.parsePacket();  
  if (packet_length) 
  {
    Serial.print("\nReceived packet:  ");

      LoRa.readBytes(tf_buffer_in, packet_length);
      for(int i=0; i<packet_length; i++)
      {
        Serial.print(tf_buffer_in[i], DEC);
        Serial.print(", ");
      }
      Serial.print("\n");
      
    Serial.print("\t - Length:  ");
      Serial.print(packet_length);
      Serial.print(" bytes\r\n");
      
    Serial.print("\t - RSSI: ");
      Serial.print(LoRa.packetRssi());
      Serial.print(" dBm\r\n");
      
    Serial.print("\t - SNR:  ");
      Serial.print(LoRa.packetSnr());
      Serial.print(" dB\r\n");
      
    Serial.print("\t - Freq. Error:  ");
      Serial.print(LoRa.packetFrequencyError());
      Serial.print(" Hz\r\n\n");


    if (tf_packet_Decode(tf_buffer_in, packet_length, &rx_tfph, &rx_tfdf) != HAL_OK)      
    {
      Serial.print("Fallo en Decode TF USLP\r\n");
      return;
    }
    
    client.write(rx_tfdf.data, rx_tfph.length-rx_tfph.vc_length-TF_PACKET_ECF_SIZE-TF_PACKET_DATA_HEADER_SIZE-TF_PACKET_PRIMARY_TRUNCATED_HEADER_SIZE+TF_PACKET_PRIMARY_BASE_HEADER_SIZE);





    if (bus_packet_Decode(rx_tfdf.data, &rx_packet) != HAL_OK)                       Serial.print("Fallo en Decode Space Packet\r\n");
    else
    {
      char header[50];
      sprintf(header, "Packet Type: %d, APID: %d, Length: %d", rx_packet.packet_type, rx_packet.apid, rx_packet.length);
      Serial.println(header);
      for(int i=0; i<(rx_packet.length-BUS_PACKET_HEADER_SIZE-BUS_PACKET_ECF_SIZE); i++)
      {
        Serial.print(rx_packet.data[i], DEC);
        Serial.print(", ");
      }
      Serial.print("\n");
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
  
}