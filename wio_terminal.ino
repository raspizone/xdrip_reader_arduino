#include <rpcWiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h> // Librería para controlar la pantalla del Wio Terminal
#include <ArduinoJson.h> // Incluye la librería ArduinoJson para manejar la respuesta JSON

const char* ssid = "Pixel";      // Cambia "yourNetwork" por tu SSID
const char* password = "wifipixel"; // Cambia "yourPassword" por tu contraseña

TFT_eSPI tft = TFT_eSPI();  // Inicializa el objeto para la pantalla TFT

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1); // Configura la orientación de la pantalla
  tft.fillScreen(TFT_BLACK); // Limpia la pantalla con color negro
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Configura el color del texto

  WiFi.begin(ssid, password);

  // Conexión al Wi-Fi
  tft.println("Conectando al Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
    WiFi.begin(ssid, password);
  }
  
  tft.fillScreen(TFT_BLACK); // Limpia la pantalla una vez conectado
  tft.println("Wi-Fi Conectado!");
  tft.print("IP: ");
  tft.println(WiFi.localIP());
}

void loop() {
  
  // Verifica que el Wio Terminal esté conectado al Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Configura la URL de destino
    http.begin("http://192.168.173.175:17580/pebble");

    // Envía la solicitud GET
    int httpCode = http.GET();

    if (httpCode > 0) {
      // Verifica si la solicitud tuvo éxito
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Analiza el JSON recibido
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          int sgv = doc["bgs"][0]["sgv"].as<int>(); // Extrae el valor del campo "sgv"
          String direction = doc["bgs"][0]["direction"].as<String>(); // Extrae el valor de "direction"

          // Muestra el valor en grande
          tft.fillScreen(TFT_BLACK); // Limpia la pantalla antes de mostrar el resultado
          tft.setTextSize(5); // Establece un tamaño de fuente grande
          tft.setCursor(30, 60); // Posiciona el texto
          tft.printf("%d mg/dL", sgv);

          // Muestra la flecha según la dirección
          drawTrendArrow(direction);
          
          delay(30000);  // Espera 30 segundos antes de la próxima solicitud
          
        } else {
          tft.fillScreen(TFT_BLACK); 
          tft.println("Error de parseo JSON");
        }
      }
    } else {
      tft.fillScreen(TFT_BLACK);
      tft.print("Error: ");
      tft.println(http.errorToString(httpCode).c_str()); // Muestra el error en la pantalla
    }

    // Finaliza la conexión HTTP
    http.end();
  } else {
    // Si se pierde la conexión Wi-Fi, intenta reconectar
    tft.fillScreen(TFT_BLACK);
    tft.println("Reconectando al Wi-Fi...");
    WiFi.begin(ssid, password);
  }
}

// Función para dibujar la flecha dependiendo de la dirección
void drawTrendArrow(String direction) {
  tft.fillRect(0, 150, 320, 90, TFT_BLACK); // Limpia el área donde se dibujará la flecha
  tft.setTextSize(3);
  
  // Configura los puntos del triángulo (flecha)
  int x0 = 160; // Centro de la pantalla
  int y0 = 190;
  
  if (direction == "Flat") {
    // Flecha hacia la derecha para tendencia plana
    tft.fillTriangle(x0 + 40, y0, x0 - 30, y0 - 30, x0 - 30, y0 + 30, TFT_WHITE);
  } else if (direction == "SingleUp") {
    // Flecha hacia arriba
    tft.fillTriangle(x0, y0 - 40, x0 - 30, y0 + 30, x0 + 30, y0 + 30, TFT_WHITE);
  } else if (direction == "FortyFiveDown") {
    // Flecha inclinada hacia abajo
    tft.fillTriangle(x0, y0 + 40, x0 - 30, y0 - 30, x0 + 30, y0 - 30, TFT_WHITE);
  } else if (direction == "FortyFiveUp") {
    // Flecha inclinada hacia arriba
    tft.fillTriangle(x0, y0 - 40, x0 - 30, y0 + 30, x0 + 30, y0 + 30, TFT_WHITE);
  } else if (direction == "DoubleUp") {
    // Flecha doble hacia arriba
    tft.fillTriangle(x0, y0 - 50, x0 - 30, y0, x0 + 30, y0, TFT_WHITE);   // Primera flecha
    tft.fillTriangle(x0, y0 - 100, x0 - 30, y0 - 50, x0 + 30, y0 - 50, TFT_WHITE); // Segunda flecha
  } else if (direction == "DoubleDown") {
    // Flecha doble hacia abajo
    tft.fillTriangle(x0, y0 + 50, x0 - 30, y0, x0 + 30, y0, TFT_WHITE);   // Primera flecha
    tft.fillTriangle(x0, y0 + 100, x0 - 30, y0 + 50, x0 + 30, y0 + 50, TFT_WHITE); // Segunda flecha
  } else {
    // Mensaje en caso de dirección desconocida
    tft.setCursor(100, 200);
    tft.setTextColor(TFT_RED);
    tft.print("Dirección desconocida");
  }
}