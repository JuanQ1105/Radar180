# Radar Ultrasónico de Proximidad con Interfaz OLED

Este repositorio contiene el código fuente y la documentación de un proyecto de radar ultrasónico desarrollado en la Universidad Industrial de Santander (UIS). El sistema coordina un servomotor y un sensor ultrasónico mediante un Arduino, visualizando la detección de obstáculos en tiempo real a través de una pantalla gráfica.

---

## 🛠️ Materiales del Sistema

* [cite_start]Microcontrolador Arduino[cite: 7].
* [cite_start]Servomotor de 180 grados[cite: 8].
* [cite_start]Sensor Ultrasónico (Pines Trig 9 y Echo 10)[cite: 9].
* [cite_start]Pantalla OLED I2C (128x64 píxeles)[cite: 10].
* [cite_start]Baterías para alimentación del circuito[cite: 11].

---

## ⚙️ Diseño y Selección de Actuadores

[cite_start]Para el desarrollo del mecanismo de rotación del radar, se evaluaron tres alternativas distintas de motores[cite: 3]. [cite_start]El objetivo principal fue lograr un emparejamiento exacto entre el ángulo físico de barrido y la representación gráfica en la pantalla[cite: 4]:

* [cite_start]**Servomotor de 180° (Seleccionado):** Al permitir control directo por ángulos, facilitó el emparejamiento posicional con la pantalla, eliminando los errores de desfase[cite: 5].
* [cite_start]**Motor Paso a Paso (Descartado):** Exigía el uso de dos fuentes de alimentación independientes (una directa al motor y otra para el Arduino), aumentando la complejidad del hardware[cite: 5].
* [cite_start]**Servomotor 360° de Rotación Continua (Descartado):** Dado que funciona mediante control de aceleración, resultó sumamente difícil sincronizar la posición física exacta con la pantalla OLED para completar el radar de 360 grados[cite: 5].

---

## 🔧 Calibración y Ajustes del Sensor

[cite_start]Durante la fase de pruebas, se detectó que el sensor captaba múltiples elementos muy cercanos, generando ruido en la lectura[cite: 13]. [cite_start]Para solucionar esto, se implementaron los siguientes ajustes paramétricos[cite: 14]:

1. [cite_start]**Acortamiento de rango:** La distancia de detección original, que podía alcanzar los 2 metros, fue limitada a un máximo de 30 centímetros mediante software[cite: 14].
2. [cite_start]**Umbral de tolerancia:** Se estableció un límite lógico para ignorar cambios milimétricos en el entorno inmediato, evitando que la pantalla gráfica registre falsos positivos constantes[cite: 15]. Adicionalmente, el código implementa un filtro de ordenamiento por burbuja para extraer la mediana de un set de 7 muestras en cada lectura.

---

## 💻 Análisis del Funcionamiento y Lógica (Firmware)

[cite_start]El comportamiento del radar está definido por el código en Arduino IDE, el cual utiliza las librerías `Wire.h` y `Adafruit` para el control del display OLED, junto con `Servo.h` para el movimiento[cite: 17]. [cite_start]El flujo se divide en las siguientes etapas principales[cite: 18]:

* [cite_start]**Inicialización (Setup):** Inicializa la comunicación I2C con la dirección 0x3C de la pantalla OLED[cite: 20]. [cite_start]Muestra un mensaje de bienvenida y configura los pines del servo (Pin 3) y el sensor ultrasónico (Trig en salida, Echo en entrada)[cite: 21].
* [cite_start]**Medición de Distancia (getDistance):** Genera un pulso corto de 10 microsegundos por el pin Trig[cite: 22]. [cite_start]Luego, cuenta el tiempo de respuesta del Echo limitando el tiempo de espera a 20ms para evitar interrupciones o bloqueos del flujo del programa[cite: 23]. [cite_start]Si la medición es errónea o sobrepasa los 30 cm, la lectura se restringe automáticamente a la distancia máxima[cite: 24].
* [cite_start]**Barrido (Loop):** El servomotor hace un recorrido incremental de 0 a 180 grados con saltos de 2 en 2 grados, para posteriormente regresar[cite: 25]. [cite_start]En cada paso angular, captura la distancia y llama a la función de renderizado[cite: 26].
* [cite_start]**Interfaz de Usuario (drawRadar):** Limpia el buffer gráfico y utiliza funciones trigonométricas (seno y coseno) para traducir el grado del servo a coordenadas en el eje X y Y del display[cite: 27]. [cite_start]Dibuja los semicírculos del radar (radios de 15, 35 y 53 pixeles), la línea central, y la línea de escaneo del ángulo actual; cuando detecta una intrusión real menor a 30 cm, renderiza un círculo blanco relleno[cite: 28]. [cite_start]Finalmente, proyecta la información del ángulo y la distancia en formato numérico en las esquinas[cite: 29].

---

## 🚀 Cómo usar este repositorio

1. Clona el repositorio: `git clone https://github.com/TU_USUARIO/radar-ultrasonico-oled.git`
2. Abre el archivo `src/radar_ultrasonico/radar_ultrasonico.ino` en el Arduino IDE.
3. Asegúrate de tener instaladas las librerías `Adafruit_GFX`, `Adafruit_SSD1306` y `Servo`.
4. Compila y sube el código a tu placa Arduino.
