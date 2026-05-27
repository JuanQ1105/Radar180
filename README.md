# Radar Ultrasónico de Proximidad con Interfaz OLED

Este repositorio contiene el código fuente y la documentación de un proyecto de radar ultrasónico. El sistema coordina un servomotor y un sensor ultrasónico mediante un Arduino, visualizando la detección de obstáculos en tiempo real a través de una pantalla gráfica.

---

## 🛠️ Materiales del Sistema

* Microcontrolador Arduino.
* Servomotor de 180 grados.
* Sensor Ultrasónico (Pines Trig 9 y Echo 10).
* Pantalla OLED I2C (128x64 píxeles).
* Baterías para alimentación del circuito.

---

## ⚙️ Diseño y Selección de Motores

Para el desarrollo del mecanismo de rotación del radar, se evaluaron tres alternativas distintas de motores. El objetivo principal fue lograr un emparejamiento exacto entre el ángulo físico de barrido y la representación gráfica en la pantalla:

* **Servomotor de 180° (Seleccionado):** Al permitir control directo por ángulos, facilitó el emparejamiento posicional con la pantalla, eliminando los errores de desfase.
* **Motor Paso a Paso (Descartado):** Exigía el uso de dos fuentes de alimentación independientes (una directa al motor y otra para el Arduino), aumentando la complejidad del hardware.
* **Servomotor 360° de Rotación Continua (Descartado):** Dado que funciona mediante control de aceleración, resultó sumamente difícil sincronizar la posición física exacta con la pantalla OLED para completar el radar de 360 grados.

---

## 🔧 Calibración y Ajustes del Sensor

Durante la fase de pruebas, se detectó que el sensor captaba múltiples elementos muy cercanos, generando ruido en la lectura. Para solucionar esto, se implementaron los siguientes ajustes paramétricos:

1. **Acortamiento de rango:** La distancia de detección original, que podía alcanzar los 2 metros, fue limitada a un máximo de 30 centímetros mediante software.
2. **Umbral de tolerancia:** Se estableció un límite lógico para ignorar cambios milimétricos en el entorno inmediato, evitando que la pantalla gráfica registre falsos positivos constantes. Adicionalmente, el código implementa un filtro de ordenamiento por burbuja para extraer la mediana de un set de 7 muestras en cada lectura.

---

## 💻 Análisis del Funcionamiento y Lógica (Firmware)

El comportamiento del radar está definido por el código en Arduino IDE, el cual utiliza las librerías `Wire.h` y `Adafruit` para el control del display OLED, junto con `Servo.h` para el movimiento. El flujo se divide en las siguientes etapas principales:

* **Inicialización (Setup):** Inicializa la comunicación I2C con la dirección 0x3C de la pantalla OLED. Muestra un mensaje de bienvenida y configura los pines del servo (Pin 3) y el sensor ultrasónico (Trig en salida, Echo en entrada).
* **Medición de Distancia (getDistance):** Genera un pulso corto de 10 microsegundos por el pin Trig. Luego, cuenta el tiempo de respuesta del Echo limitando el tiempo de espera a 20ms para evitar interrupciones o bloqueos del flujo del programa. Si la medición es errónea o sobrepasa los 30 cm, la lectura se restringe automáticamente a la distancia máxima.
* **Barrido (Loop):** El servomotor hace un recorrido incremental de 0 a 180 grados con saltos de 2 en 2 grados, para posteriormente regresar. En cada paso angular, captura la distancia y llama a la función de renderizado.
* **Interfaz de Usuario (drawRadar):** Limpia el buffer gráfico y utiliza funciones trigonométricas (seno y coseno) para traducir el grado del servo a coordenadas en el eje X y Y del display. Dibuja los semicírculos del radar (radios de 15, 35 y 53 pixeles), la línea central, y la línea de escaneo del ángulo actual; cuando detecta una intrusión real menor a 30 cm, renderiza un círculo blanco relleno. Finalmente, proyecta la información del ángulo y la distancia en formato numérico en las esquinas.

---

## 🚀 Cómo usar este repositorio

1. Abre el archivo `src/radar_ultrasonico/radar_ultrasonico.ino` en el Arduino IDE.
2. Asegúrate de tener instaladas las librerías `Adafruit_GFX`, `Adafruit_SSD1306` y `Servo`.
3. Compila y sube el código a tu placa Arduino.
