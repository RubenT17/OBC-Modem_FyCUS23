
// Estándar Recomendado CEPT Rec. 70-03

/* Private includes ----------------------------------------------------------*/
#include <SPI.h>
#include <LoRa.h>
#include <RTCZero.h>
#include <SD.h>


#include <Adafruit_SleepyDog.h>
// FlashStorage para EEPROM
#include "bus_packet.h"
#include "tf_packet.h"

/* Private define ------------------------------------------------------------*/
/* Debugger */
#define LORA_DEBUG            0
#define SERIAL_DEBUG_TX       0
#define SERIAL_DEBUG_RX       0
#define SERIAL_DEBUG_BUS      0
#define SERIAL_DEBUG_SAVING   0
#define SERIAL_DEBUG_ERROR    0
#define DATA_DEBUG            0
#define TIME_DOT              3000

#if !(SERIAL_DEBUG_TX | SERIAL_DEBUG_RX | SERIAL_DEBUG_ERROR | SERIAL_DEBUG_BUS | SERIAL_DEBUG_SAVING)
#define Error_Handle(x) ;
#endif


/* WDT TIME */
#define WDT_TIME          5000

#ifndef ARDUINO_SAMD_MKRWAN1300 || ARDUINO_SAMD_MKRWAN1310
//  #define SCK     5
//  #define MISO    19
//  #define MOSI    27
//  #define SS      18
//  #define RST     14
//  #define DIO0    26
#endif

#define BAND    433E6
                //433E6
                //866E6
                //915E6

#define CS_SD_PIN         4    // SDHD, FAT32
#define INT_PIN           1


/* Private typedef -----------------------------------------------------------*/
/* APIDs */
typedef enum
{
  APID_TM_PROGRAMMED_TELEMETRY  = 0x00,
  APID_TM_REPORT_OBC        = 0x01,
  APID_TM_REPORT_EPS        = 0x02,
  APID_TM_REPORT_MODEM      = 0x03,
  APID_TM_LAST_DATA_REQUIRED    = 0x04,
  APID_TM_REPORT_FAULT_TC     = 0x10,

  APID_TC_PROGRAMMED_TELECOMMAND  = 0x20,
  APID_TC_REQUIRED_REPORT_OBC   = 0x21,
  APID_TC_REQUIRED_REPORT_EPS   = 0x22,
  APID_TC_REQUIRED_REPORT_MODEM = 0x23,
  APID_TC_REQUIRED_DATA     = 0x24,
  APID_TC_SET_PROGRMMED_TELEMETRY = 0x30,
  APID_TC_ARE_YOU_ALIVE     = 0x31,

  APID_BUS_REPORT_OBC       = 0x41,
  APID_BUS_REPORT_EPS       = 0x42,
  APID_BUS_REPORT_MODEM     = 0x43,
  APID_BUS_DATA_SAVING      = 0x46,
  APID_BUS_FAULT_TM       = 0x50,
  APID_BUS_FAULT_TC       = 0x51,

  APID_BUS_GET_CURRENT_TIME   = 0x64,
  APID_BUS_SET_CURRENT_TIME   = 0x65,

} apid_t;

/* Private variables ---------------------------------------------------------*/
unsigned long int t;


uint8_t bus_buffer_in[BUS_PACKET_BUS_SIZE] = {0};
uint8_t sp_buffer_in[BUS_PACKET_BUS_SIZE] = {0};

uint8_t flag_bus_processing = 0;
uint8_t flag_telemetry = 0;
uint8_t flag_telecommand = 0;

uint8_t sd_buffer[BUS_PACKET_BUS_SIZE*4];
uint8_t sd_status = 0;
const char file_name[] = "logger.bin";

tfph_packet_t tx_tfph = {0};
tfdf_packet_t tx_tfdf = {0};
bus_packet_t rx_sp = {0};


RTCZero rtc;

// File object to represent file
File logger;



/* Private function prototypes -----------------------------------------------*/
void LoraTx ();
void LoraRx ();
void BusRx ();
void processing_bus();
void process_tc();
void data_saving();
void Error_Handler(String error);


void gnss_nmea_Time2bytes(float time, uint8_t *hours, uint8_t *min, uint8_t *sec)
{
  *hours = time*0.0001;
  *min = (time-*hours*10000)*0.01;
  *sec = time-((*hours)*10000)-((*min)*100);
}



/* Private user code ---------------------------------------------------------*/
void setup() { 

  pinMode(INT_PIN, INPUT);
  Watchdog.enable(WDT_TIME);


   
#if (SERIAL_DEBUG_TX | SERIAL_DEBUG_RX | SERIAL_DEBUG_ERROR | SERIAL_DEBUG_BUS | SERIAL_DEBUG_SAVING)
  Serial.begin(115200);
  while(!Serial);
  Serial.print(F(" ________ ___    ___ ________  ___  ___  ________    \r\n")); 
  Serial.print(F("|\\  _____\\\\  \\  /  /|\\   ____\\|\\  \\|\\  \\|\\   ____\\     \r\n"));
  Serial.print(F("\\ \\  \\__/\\ \\  \\/  / | \\  \\___|\\ \\  \\\\\\  \\ \\  \\___|_    \r\n"));
  Serial.print(F(" \\ \\   __\\\\ \\    / / \\ \\  \\    \\ \\  \\\\\\  \\ \\_____  \\   \r\n"));
  Serial.print(F("  \\ \\  \\_| \\/  /  /   \\ \\  \\____\\ \\  \\\\\\  \\|____|\\  \\  \r\n"));
  Serial.print(F("   \\ \\__\\__/  / /      \\ \\_______\\ \\_______\\____\\_\\  \\ \r\n"));
  Serial.print(F("    \\|__|\\___/ /        \\|_______|\\|_______|\\_________\\\r\n"));
  Serial.print(F("        \\|___|/                            \\|_________|\r\n\n\n"));
#endif

  if (!LoRa.begin(BAND))  Error_Handler("Fallo en Inicialización LoRa");  
  else
  {    
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



#if SERIAL_DEBUG_ERROR
  uint8_t reset_cause = Watchdog.resetCause();
  Serial.print("Last reset cause: ");
  Serial.print(reset_cause, BIN);
  Serial.println("\n\n\n");
#endif

#if SERIAL_DEBUG_RX
  t=millis();
  Serial.print("Esperando paquete");
#endif


  rtc.begin();                          
  rtc.setHours(0);
  rtc.setMinutes(0);
  rtc.setSeconds(0);
  rtc.setDay(2);
  rtc.setMonth(10);
  rtc.setYear(2023);

  Serial1.begin(500000);


  tx_tfph.tfvn = TF_PACKET_TFVN;
  tx_tfph.scid = TF_PACKET_DEFAULT_SCID;
  tx_tfph.source_dest_id = TF_PACKET_SOURCE;
  tx_tfph.vcid = TF_PACKET_DEFAULT_VCID;
  tx_tfph.mapid = TF_PACKET_DEFAULT_MAPID;
  tx_tfph.end_flag = TF_PACKET_NOT_TRUNCATED;
  tx_tfph.bypass_flag = TF_PACKET_EXPEDITED;
  tx_tfph.command_flag = TF_PACKET_INFO;
  tx_tfph.ocf_flag = TF_PACKET_OCF_EXIST;
  tx_tfdf.constr_rule = TF_PACKET_DEFAULT_CONSTR_RULE;
  tx_tfdf.protocol_id = TF_PACKET_DEFAULT_PROTOCOL_ID;



  Watchdog.reset();

//  if (!SD.begin())
//  {
//    Error_Handler("*** ERROR SD ***");
//    sd_status = 0;
//  }
//  else
//  {         
//    logger = SD.open(file_name, FILE_WRITE);
//    if(!logger)
//    {
//      Error_Handler("*** DATA LOGGER OPENING ERROR ***");
//      sd_status = 0;
//    }
//    else
//    {
//      sd_status = 1;
//      logger.close();
//    }
//  }
  

  Watchdog.reset();
}



void loop() 
{
  LoraRx();
  Watchdog.reset();
  BusRx();
  Watchdog.reset();
  if(flag_bus_processing)   processing_bus();
  if(flag_telemetry)        LoraTx();
  if(flag_telecommand)      process_tc();
    
  Watchdog.reset();
}




 /**********************************************************************/


void processing_bus()
{
  flag_bus_processing = 0;
  memmove(sp_buffer_in, bus_buffer_in, (bus_buffer_in[1] & 0b01111111));
    if (bus_packet_Decode(sp_buffer_in, &rx_sp) != HAL_OK)                Error_Handler("Fallo en Decode BUS Space Packet");
    else
    {
      if(rx_sp.packet_type == BUS_PACKET_TYPE_TC)   flag_telecommand = 1;
      else                                          flag_telemetry = 1;

#if SERIAL_DEBUG_BUS
      static char header[50];
      sprintf(header, "Packet Type: %d, APID: %d, Length: %d", rx_sp.packet_type, rx_sp.apid, rx_sp.length);
      Serial.println(header);
      for(int i=0; i<(rx_sp.length-BUS_PACKET_HEADER_SIZE-BUS_PACKET_ECF_SIZE); i++)
      {
        Serial.print(rx_sp.data[i], DEC);
        Serial.print(", ");
      }
      Serial.print("\n");
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
#endif
    }
}


 /**********************************************************************/
void process_tc()
{
  switch((apid_t) rx_sp.apid)
  {
    case APID_BUS_DATA_SAVING:
      data_saving();
      break;

      default:
        data_saving();
  }
  data_saving();

  flag_telecommand = 0;
}





 /**********************************************************************/


void LoraTx()
{
  static uint8_t packet_count[2] = {0x00, 0x01};
  static uint8_t tf_buffer_out[TF_PACKET_MAX_SIZE] = {0};
  static uint8_t sp_buffer_out[BUS_PACKET_BUS_SIZE] = {0};
  

  memmove(sp_buffer_out, bus_buffer_in, (bus_buffer_in[1] & 0b01111111));
  
LoraRx();
  
//    for(int i=0; i<6; i++)
//    {
//      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//      delay(50);
//    }

    
    if(packet_count[1] == 0xFF)
    {
      packet_count[0]++;
      packet_count[1] = 0x00;
    }
    else packet_count[1]++;
    
    tf_packet_SetData(sp_buffer_out, (sp_buffer_out[1] & 0b01111111), packet_count, 2, &tx_tfph, &tx_tfdf);
LoraRx();
    if(tf_packet_Packetize(NULL, &tx_tfph, &tx_tfdf, tf_buffer_out) != HAL_OK)
    {
      String err = "";
#if SERIAL_DEBUG_ERROR
      for(int i=0; i<(sp_buffer_out[1] & 0b01111111); i++)
      {
        err = err + String(sp_buffer_out[i]) + ",";
      }
#endif
      Error_Handler("Fallo en paquetizar TF:\n" + err);
    }
  
LoraRx();

    LoRa.beginPacket();
    LoRa.write(tf_buffer_out, tx_tfph.length);
    LoRa.endPacket(true);
    
LoraRx();

#if SERIAL_DEBUG_TX
    Serial.print("\nDatos enviados:\n");
    for(int i=0; i<tx_tfph.length; i++)
    {
      Serial.print(tf_buffer_out[i], DEC);
      Serial.print(", ");
    }
    Serial.print("\n");
#endif

  flag_telemetry = 0;

}




 /**********************************************************************/



void LoraRx ()
{
  static uint8_t tf_buffer_in[TF_PACKET_MAX_SIZE] = {0};
  static uint8_t sp_data[BUS_PACKET_DATA_SIZE] = {0};
  static uint8_t sp_buffer_in[BUS_PACKET_BUS_SIZE] = {0};
  static bus_packet_t rx_packet = {0};
  static tfph_packet_t rx_tfph = {0};
  static tfdf_packet_t rx_tfdf = {0};


  int packet_length = LoRa.parsePacket(); 
  if (packet_length) 
  {
      LoRa.readBytes(tf_buffer_in, packet_length);


#if SERIAL_DEBUG_RX
      Serial.print("\nReceived packet:  ");
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
      Serial.print(" dBm\r\n");     // mW = 10^(rssi_dbm/10)
      
    Serial.print("\t - SNR:  ");
      Serial.print(LoRa.packetSnr());
      Serial.print(" dB\r\n");
      
    Serial.print("\t - Freq. Error:  ");
      Serial.print(LoRa.packetFrequencyError());
      Serial.print(" Hz\r\n\n");
#endif


    if (tf_packet_Decode(tf_buffer_in, packet_length, &rx_tfph, &rx_tfdf) != HAL_OK)      Error_Handler("Fallo en Decode TF USLP");
    else if (bus_packet_Decode(rx_tfdf.data, &rx_packet) != HAL_OK)                       Error_Handler("Fallo en Decode Space Packet");
    else
    {
      Serial1.write(BUS_PACKET_FRAME_SYNC, 4);

      int sp_length = packet_length;
      if(!rx_tfph.end_flag)   sp_length = sp_length - TF_PACKET_PRIMARY_BASE_HEADER_SIZE - rx_tfph.vc_length - TF_PACKET_DATA_HEADER_SIZE - TF_PACKET_ECF_SIZE;
      else                    sp_length = sp_length - TF_PACKET_PRIMARY_TRUNCATED_HEADER_SIZE - TF_PACKET_DATA_HEADER_SIZE - TF_PACKET_ECF_SIZE;

      Serial1.write(rx_tfdf.data, sp_length);


#if SERIAL_DEBUG_RX
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      Serial.print("Datos recibidos:\n");
      Serial.write(rx_tfdf.data, sp_length);
      Serial.print("\n");
      static char header[50];
      sprintf(header, "Packet Type: %d, APID: %d, Length: %d", rx_packet.packet_type, rx_packet.apid, rx_packet.length);
      Serial.println(header);
      for(int i=0; i<(sp_length-BUS_PACKET_HEADER_SIZE-BUS_PACKET_ECF_SIZE); i++)
      {
        Serial.print(rx_packet.data[i], DEC);
        Serial.print(", ");
      }
      Serial.print("\n");
#endif
      
    }

#if SERIAL_DEBUG_RX
    Serial.print("\nEsperando paquete");
#endif
  }

#if SERIAL_DEBUG_RX
  else
  {
    if(millis()-t > TIME_DOT)
    {
      Serial.print(".");
      t=millis();
    }
  }
#endif
  

}




 /**********************************************************************/



void data_saving()
{
//  static uint8_t buffer[BUS_PACKET_BUS_SIZE];
//  static uint8_t ones[3] = {255,255,255};
//  if(!sd_status)
//  {
//    if (!SD.begin()) return;
//  }
//  
//  logger = SD.open(file_name, FILE_WRITE);
//  if(logger)
//  {
//    bus_packet_Packetize(buffer, &rx_sp);
//    logger.write(buffer, rx_sp.length);
//    logger.write(ones, 3);
//    logger.close();
//    sd_status = 1;
//  }
//  else
//  {
//    sd_status = 0;
//    return;
//  }
//  
//  
//#if SERIAL_DEBUG_SAVING
//  Serial.print("Paquete para guardar: ");
//  Serial.write(buffer, rx_sp.length);
//  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//  delay(200);
//  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//  delay(100);
//#endif
}


 /**********************************************************************/


void BusRx()
{
  static uint8_t bus_buffer_pos = 0;
  static bus_sync_flag_t  sync_flag = BUS_PACKET_SYNC_FIND;
  static int packet_length = 0;

  while(Serial1.available())
  {
    bus_buffer_in[bus_buffer_pos] = Serial1.read();
    
    if (sync_flag != BUS_PACKET_SYNC_COMPLETED)
    {
      sync_flag = bus_packet_SyncFrameDetect(sync_flag, bus_buffer_in[bus_buffer_pos]);

    
      if(sync_flag == BUS_PACKET_SYNC_COMPLETED)
      {
        bus_buffer_pos=0;
      }
      else
      {
        bus_buffer_pos++;
        if(bus_buffer_pos >= BUS_PACKET_BUS_SIZE)
          bus_buffer_pos=0;
    
      }
    }
    
    else
    {
      if(bus_buffer_pos > 0)
      {
        packet_length = bus_buffer_in[1] & 0b01111111;
    
        if (bus_buffer_pos >= (packet_length-1))
        {
          bus_buffer_pos = 0;
          sync_flag = BUS_PACKET_SYNC_FIND;
          flag_bus_processing = 1;
        }
        else bus_buffer_pos++;
      }
      else  bus_buffer_pos++;
    
      if(bus_buffer_pos < BUS_PACKET_BUS_SIZE);
      else Error_Handler("Fallo leyendo paquete");
    }
  }
}






 /**********************************************************************/




void Error_Handler(String error)
{
#if SERIAL_DEBUG_ERROR
  Serial.print("\r\n\nError:\r\n");
  Serial.print(error);
  Serial.flush();

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(200);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(100);
#endif

}
