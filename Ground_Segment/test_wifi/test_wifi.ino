#include <WiFi.h>
#include <WiFiClient.h>

const char *ssid = "IPhone de Rubén";
const char *password = "rtb123456";
const int serverPort = 2023;

WiFiServer server(serverPort);
WiFiClient client;


void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }


  Serial.println("Conectado a la red WiFi");
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.print("Servidor iniciado en el puerto: ");
  Serial.println(serverPort);
}

void loop() {
  // Esperar una conexión entrante
  client = server.available();
  uint8_t data[100] = {0};
  
  if (client) {
    Serial.println("Cliente conectado");

    while (client.connected()) {
      if (client.available()) 
      {
        uint8_t i=0;
        for(i=0; client.available(); i++)
        {
          data[i] = client.read();
          Serial.print(data[i]);
          Serial.print(",");
        }
        Serial.print(" i = ");
        Serial.print(i);
        Serial.print("\n");

        client.write(data, i);
      }
    }
    client.stop();
    Serial.println("Cliente desconectado");
  }
}