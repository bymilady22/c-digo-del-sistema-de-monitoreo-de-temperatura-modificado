#define BLYNK_TEMPLATE_ID "TMPL2AEA3O4cU"
#define BLYNK_TEMPLATE_NAME "Control de Temperatura"
#define BLYNK_AUTH_TOKEN "7HSHTt5vyuKbasbUG7_WVCfGzfLXDhtO"

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>

// =================================================
// SENSOR MLX90614
// =================================================

#define MLX90614_ADDR 0x5A

// =================================================
// LCD 16x2 I2C
// =================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =================================================
// PINES NODEMCU V3
// =================================================

#define BUZZER_PIN 14

// =================================================
// WIFI
// =================================================

char ssid[] = "PRIMER_PISO";
char pass[] = "98385317";

// =================================================
// VARIABLES DE TIEMPO
// =================================================

unsigned long inicio = 0;
unsigned long ultimaMedicion = 0;

// =================================================
// RANGO DE TEMPERATURA
// =================================================

float temperaturaMinima = 30.0;
float temperaturaMaxima = 40.0;

bool alarma = false;

// =================================================
// LEER REGISTRO DEL MLX90614
// =================================================

uint16_t leerRegistro(uint8_t registro) {

  Wire.beginTransmission(MLX90614_ADDR);
  Wire.write(registro);

  if (Wire.endTransmission(false) != 0) {
    return 0xFFFF;
  }

  if (Wire.requestFrom((uint8_t)MLX90614_ADDR, (uint8_t)3) != 3) {
    return 0xFFFF;
  }

  uint8_t lowByte = Wire.read();
  uint8_t highByte = Wire.read();

  Wire.read();

  return ((uint16_t)highByte << 8) | lowByte;
}

// =================================================
// CONVERTIR RAW A GRADOS CELSIUS
// =================================================

float convertirTemperatura(uint16_t raw) {

  return (raw * 0.02) - 273.15;
}

// =================================================
// BLYNK - TEMPERATURA MINIMA V3
// =================================================

BLYNK_WRITE(V3) {

  temperaturaMinima = param.asFloat();

  Serial.println();
  Serial.println(">>> CAMBIO DESDE BLYNK <<<");

  Serial.print("Temperatura minima: ");
  Serial.print(temperaturaMinima, 2);
  Serial.println(" °C");
}

// =================================================
// BLYNK - TEMPERATURA MAXIMA V4
// =================================================

BLYNK_WRITE(V4) {

  temperaturaMaxima = param.asFloat();

  Serial.println();
  Serial.println(">>> CAMBIO DESDE BLYNK <<<");

  Serial.print("Temperatura maxima: ");
  Serial.print(temperaturaMaxima, 2);
  Serial.println(" °C");
}

// =================================================
// SETUP
// =================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.println(" SISTEMA DE CONTROL DE TEMPERATURA");
  Serial.println("========================================");

  // CONFIGURAR BUZZER

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("Buzzer configurado en D5");

  // CONFIGURAR I2C

  Wire.begin(4, 5);
  Wire.setClock(100000);

  Serial.println("Sensor MLX90614 configurado");

  // CONFIGURAR LCD

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Control Temp.");

  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  delay(2000);

  lcd.clear();

  Serial.println("LCD 16x2 configurada");

  // CONECTAR A BLYNK

  Serial.println();
  Serial.println("Conectando a Blynk...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Blynk conectado");

  // CONFIGURAR TIEMPO

  inicio = millis();

  // MOSTRAR RANGO INICIAL

  Serial.println();
  Serial.println("========================================");
  Serial.println("RANGO INICIAL");
  Serial.println("========================================");

  Serial.print("Temperatura minima: ");
  Serial.print(temperaturaMinima, 2);
  Serial.println(" °C");

  Serial.print("Temperatura maxima: ");
  Serial.print(temperaturaMaxima, 2);
  Serial.println(" °C");

  Serial.println();
  Serial.println("COMANDOS DISPONIBLES:");
  Serial.println("MIN 30");
  Serial.println("MAX 40");
  Serial.println("RANGO");
  Serial.println();
}

// =================================================
// LOOP
// =================================================

void loop() {

  // MANTENER CONEXION BLYNK

  Blynk.run();

  // =================================================
  // LEER MONITOR SERIAL
  // =================================================

  if (Serial.available()) {

    String comando = Serial.readStringUntil('\n');

    comando.trim();
    comando.toUpperCase();

    // CAMBIAR TEMPERATURA MINIMA

    if (comando.startsWith("MIN ")) {

      float nuevoMinimo = comando.substring(4).toFloat();

      temperaturaMinima = nuevoMinimo;

      Serial.println();
      Serial.println(">>> NUEVO LIMITE MINIMO <<<");

      Serial.print("Temperatura minima: ");
      Serial.print(temperaturaMinima, 2);
      Serial.println(" °C");

      Blynk.virtualWrite(V3, temperaturaMinima);
    }

    // CAMBIAR TEMPERATURA MAXIMA

    else if (comando.startsWith("MAX ")) {

      float nuevoMaximo = comando.substring(4).toFloat();

      temperaturaMaxima = nuevoMaximo;

      Serial.println();
      Serial.println(">>> NUEVO LIMITE MAXIMO <<<");

      Serial.print("Temperatura maxima: ");
      Serial.print(temperaturaMaxima, 2);
      Serial.println(" °C");

      Blynk.virtualWrite(V4, temperaturaMaxima);
    }

    // MOSTRAR RANGO ACTUAL

    else if (comando == "RANGO") {

      Serial.println();
      Serial.println("========================================");
      Serial.println("RANGO ACTUAL");
      Serial.println("========================================");

      Serial.print("Minima: ");
      Serial.print(temperaturaMinima, 2);
      Serial.println(" °C");

      Serial.print("Maxima: ");
      Serial.print(temperaturaMaxima, 2);
      Serial.println(" °C");

      Serial.println("========================================");
    }
  }

  // =================================================
  // MEDICION CADA 1 SEGUNDO
  // =================================================

  if (millis() - ultimaMedicion >= 1000) {

    ultimaMedicion = millis();

    // LEER SENSOR MLX90614

    uint16_t rawAmbiente = leerRegistro(0x06);
    uint16_t rawObjeto = leerRegistro(0x07);

    // CONVERTIR TEMPERATURAS

    float temperaturaAmbiente =
      convertirTemperatura(rawAmbiente);

    float temperaturaObjeto =
      convertirTemperatura(rawObjeto);

    // CALCULAR TIEMPO

    float tiempo =
      (millis() - inicio) / 1000.0;

    // =================================================
    // MONITOR SERIAL
    // =================================================

    Serial.println();
    Serial.println("========================================");

    Serial.print("Temperatura ambiente: ");
    Serial.print(temperaturaAmbiente, 2);
    Serial.println(" °C");

    Serial.print("Temperatura objeto: ");
    Serial.print(temperaturaObjeto, 2);
    Serial.println(" °C");

    Serial.print("Temperatura minima: ");
    Serial.print(temperaturaMinima, 2);
    Serial.println(" °C");

    Serial.print("Temperatura maxima: ");
    Serial.print(temperaturaMaxima, 2);
    Serial.println(" °C");

    Serial.print("Tiempo: ");
    Serial.print(tiempo, 1);
    Serial.println(" s");

    // =================================================
    // CONTROL DEL BUZZER
    // =================================================

    if (temperaturaObjeto >= temperaturaMinima &&
        temperaturaObjeto <= temperaturaMaxima) {

      alarma = true;

      digitalWrite(BUZZER_PIN, HIGH);

      Serial.println();
      Serial.println("***** BUZZER ACTIVADO *****");
      Serial.println("Temperatura DENTRO del rango");

    } else {

      alarma = false;

      digitalWrite(BUZZER_PIN, LOW);

      Serial.println();
      Serial.println("Buzzer: APAGADO");

      if (temperaturaObjeto < temperaturaMinima) {
        Serial.println("Temperatura POR DEBAJO del rango");
      }

      if (temperaturaObjeto > temperaturaMaxima) {
        Serial.println("Temperatura POR ENCIMA del rango");
      }
    }

    Serial.println("========================================");

    // =================================================
    // ENVIAR DATOS A BLYNK
    // =================================================

    Blynk.virtualWrite(V0, temperaturaObjeto);
    Blynk.virtualWrite(V1, temperaturaAmbiente);
    Blynk.virtualWrite(V2, tiempo);
    Blynk.virtualWrite(V3, temperaturaMinima);
    Blynk.virtualWrite(V4, temperaturaMaxima);
    Blynk.virtualWrite(V5, alarma ? 1 : 0);

    // =================================================
    // LCD 16x2
    // MOSTRAR SOLAMENTE LAS DOS TEMPERATURAS
    // =================================================

    lcd.setCursor(0, 0);
    lcd.print("Obj:");
    lcd.print(temperaturaObjeto, 2);
    lcd.print(" C   ");

    lcd.setCursor(0, 1);
    lcd.print("Amb:");
    lcd.print(temperaturaAmbiente, 2);
    lcd.print(" C   ");
  }
}
