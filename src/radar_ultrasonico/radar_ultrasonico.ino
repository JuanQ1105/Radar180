#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


Servo myServo;

#define SERVO_PIN 3

#define TRIG_PIN 9
#define ECHO_PIN 10

const int DISTANCIA_MAXIMA = 30;

long duration;

bool objetoDetectado = false;

int ultimaDistancia = 0;

int medirDistanciaFiltrada() {

  const int muestras = 7;

  int valores[muestras];
  int validas = 0;

  for (int i = 0; i < muestras; i++) {

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 12000);

    int dist = duration * 0.034 / 2;

    if (dist >= 3 && dist < DISTANCIA_MAXIMA) {

      valores[validas] = dist;
      validas++;
    }

    delay(2);
  }

  if (validas == 0) {

    objetoDetectado = false;
    ultimaDistancia = 0;

    return 0;
  }

  for (int i = 0; i < validas - 1; i++) {

    for (int j = i + 1; j < validas; j++) {

      if (valores[j] < valores[i]) {

        int temp = valores[i];
        valores[i] = valores[j];
        valores[j] = temp;
      }
    }
  }

  int mediana = valores[validas / 2];

  objetoDetectado = true;

  ultimaDistancia = mediana;

  return mediana;
}

void setup() {

  Serial.begin(115200);

  Wire.begin();

  // OLED más rápida
  Wire.setClock(400000);

  // Inicializar OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {

    Serial.println(F("Error OLED"));

    while (1);
  }

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(20, 25);
  display.print("RADAR INICIANDO");

  display.display();

  delay(1500);

  // Servo
  myServo.attach(SERVO_PIN);

  // Pines sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {

  for (int pos = 0; pos <= 180; pos += 2) {

    myServo.write(pos);

    delay(15);

    medirDistanciaFiltrada();

    drawRadar(pos);
  }

  for (int pos = 180; pos >= 0; pos -= 2) {

    myServo.write(pos);

    delay(15);

    medirDistanciaFiltrada();

    drawRadar(pos);
  }
}

void drawRadar(int angle) {

  display.clearDisplay();

  int cx = 64;
  int cy = 63;

  int maxRadio = 53;

  display.drawCircle(cx, cy, 15, SSD1306_WHITE);
  display.drawCircle(cx, cy, 35, SSD1306_WHITE);
  display.drawCircle(cx, cy, maxRadio, SSD1306_WHITE);

  display.drawLine(cx, cy, cx, cy - maxRadio, SSD1306_WHITE);

  float rad = radians(angle);

  int sx = cx + (maxRadio * cos(rad));
  int sy = cy - (maxRadio * sin(rad));

  display.drawLine(cx, cy, sx, sy, SSD1306_WHITE);

  if (objetoDetectado && ultimaDistancia > 0) {

    float escala = (float)maxRadio / DISTANCIA_MAXIMA;

    int xObj = cx + (ultimaDistancia * cos(rad) * escala);
    int yObj = cy - (ultimaDistancia * sin(rad) * escala);

    display.fillCircle(xObj, yObj, 3, SSD1306_WHITE);
  }

  display.setTextSize(1);

  display.setCursor(0, 0);

  display.print("D:");

  if (objetoDetectado) {

    display.print(ultimaDistancia);
    display.print("cm");
  }
  else {

    display.print("---");
  }

  display.setCursor(92, 0);

  display.print("A:");
  display.print(angle);

  display.display();
}
