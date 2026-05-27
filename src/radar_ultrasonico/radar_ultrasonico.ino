// --- LIBRERÍAS ---
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// --- CONFIGURACIÓN DE LA PANTALLA OLED ---
#define SCREEN_WIDTH 128     // Ancho de la pantalla OLED en píxeles
#define SCREEN_HEIGHT 64     // Alto de la pantalla OLED en píxeles
#define OLED_RESET -1        // Pin de reset (usamos -1 si comparte el reset del Arduino)
#define SCREEN_ADDRESS 0x3C  // Dirección I2C estándar para pantallas de 128x64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- CONFIGURACIÓN DEL SERVOMOTOR ---
Servo myServo;
#define SERVO_PIN 3          // Pin PWM donde está conectado el servo de 180°

// --- CONFIGURACIÓN DEL SENSOR ULTRASÓNICO ---
#define TRIG_PIN 9           // Pin de disparo (Trigger)
#define ECHO_PIN 10          // Pin de eco (Echo)
const int DISTANCIA_MAXIMA = 30; // Límite de detección en centímetros (ajuste de rango)

// --- VARIABLES GLOBALES ---
long duration;               // Tiempo de viaje del pulso ultrasónico
bool objetoDetectado = false;// Bandera para saber si hay intrusión en el rango
int ultimaDistancia = 0;     // Almacena la última distancia válida calculada

/*
 * Función: medirDistanciaFiltrada
 * --------------------------------
 * Realiza múltiples lecturas del sensor y aplica un filtro de mediana 
 * (ordenamiento de burbuja) para eliminar el ruido y evitar falsos positivos.
 */
int medirDistanciaFiltrada() {
  const int muestras = 7;    // Número de lecturas por cada paso del servo
  int valores[muestras];
  int validas = 0;

  // Toma de muestras
  for (int i = 0; i < muestras; i++) {
    // Generar pulso de 10us en el Trig
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Leer el pulso de retorno con un timeout de 12ms
    duration = pulseIn(ECHO_PIN, HIGH, 12000);
    
    // Calcular distancia en cm (Velocidad del sonido: 0.034 cm/us)
    int dist = duration * 0.034 / 2;

    // Filtrar rangos no deseados o errores del sensor
    if (dist >= 3 && dist < DISTANCIA_MAXIMA) {
      valores[validas] = dist;
      validas++;
    }
    delay(2); // Pequeña pausa entre pulsos
  }

  // Si no hubo lecturas válidas, no hay objeto en el rango
  if (validas == 0) {
    objetoDetectado = false;
    ultimaDistancia = 0;
    return 0;
  }

  // Ordenamiento de burbuja (Bubble Sort) para encontrar la mediana
  for (int i = 0; i < validas - 1; i++) {
    for (int j = i + 1; j < validas; j++) {
      if (valores[j] < valores[i]) {
        int temp = valores[i];
        valores[i] = valores[j];
        valores[j] = temp;
      }
    }
  }

  // Extraer el valor central (mediana) para descartar picos de ruido
  int mediana = valores[validas / 2];

  objetoDetectado = true;
  ultimaDistancia = mediana;

  return mediana;
}

/*
 * Función: setup
 * ---------------
 * Configuración inicial de periféricos, pines y comunicación.
 */
void setup() {
  Serial.begin(115200);

  // Inicializar bus I2C a alta velocidad para gráficos fluidos
  Wire.begin();
  Wire.setClock(400000);

  // Inicializar pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Error: No se encontró la pantalla OLED"));
    while (1); // Detener el sistema si falla la pantalla
  }

  // Mostrar mensaje de bienvenida
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.print("RADAR INICIANDO");
  display.display();
  delay(1500);

  // Configurar pines
  myServo.attach(SERVO_PIN);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

/*
 * Función: loop
 * --------------
 * Ciclo principal. Realiza el barrido de ida y vuelta con el servo,
 * tomando mediciones y actualizando la interfaz en cada paso.
 */
void loop() {
  // Barrido de 0° a 180° (Ida)
  for (int pos = 0; pos <= 180; pos += 2) {
    myServo.write(pos);
    delay(15);                 // Dar tiempo al servo para alcanzar la posición
    medirDistanciaFiltrada();  // Actualizar lectura del sensor
    drawRadar(pos);            // Renderizar gráficos en la OLED
  }

  // Barrido de 180° a 0° (Vuelta)
  for (int pos = 180; pos >= 0; pos -= 2) {
    myServo.write(pos);
    delay(15);
    medirDistanciaFiltrada();
    drawRadar(pos);
  }
}

/*
 * Función: drawRadar
 * -------------------
 * Renderiza la interfaz gráfica en la OLED: arcos, líneas de barrido,
 * puntos de detección e información numérica (Distancia y Ángulo).
 */
void drawRadar(int angle) {
  display.clearDisplay();

  // Coordenadas del origen (centro inferior de la pantalla)
  int cx = 64;
  int cy = 63;
  int maxRadio = 53; // Radio máximo del radar en píxeles

  // Dibujar semicírculos estáticos del radar
  display.drawCircle(cx, cy, 15, SSD1306_WHITE);
  display.drawCircle(cx, cy, 35, SSD1306_WHITE);
  display.drawCircle(cx, cy, maxRadio, SSD1306_WHITE);

  // Línea horizontal base
  display.drawLine(cx, cy, cx, cy - maxRadio, SSD1306_WHITE);

  // Convertir el ángulo actual a radianes para cálculos trigonométricos
  float rad = radians(angle);

  // Calcular el punto final de la línea de barrido
  int sx = cx + (maxRadio * cos(rad));
  int sy = cy - (maxRadio * sin(rad));

  // Dibujar la línea de escaneo actual
  display.drawLine(cx, cy, sx, sy, SSD1306_WHITE);

  // Si se detectó un obstáculo en el rango válido, dibujar un punto
  if (objetoDetectado && ultimaDistancia > 0) {
    // Mapear la distancia física (cm) a la escala de la pantalla (píxeles)
    float escala = (float)maxRadio / DISTANCIA_MAXIMA;
    int xObj = cx + (ultimaDistancia * cos(rad) * escala);
    int yObj = cy - (ultimaDistancia * sin(rad) * escala);

    // Renderizar la intrusión
    display.fillCircle(xObj, yObj, 3, SSD1306_WHITE);
  }

  // --- IMPRESIÓN DE TEXTOS EN PANTALLA ---
  display.setTextSize(1);

  // Esquina superior izquierda: Distancia
  display.setCursor(0, 0);
  display.print("D:");
  if (objetoDetectado) {
    display.print(ultimaDistancia);
    display.print("cm");
  } else {
    display.print("---");
  }

  // Esquina superior derecha: Ángulo
  display.setCursor(92, 0);
  display.print("A:");
  display.print(angle);

  // Enviar el buffer a la pantalla
  display.display();
}
