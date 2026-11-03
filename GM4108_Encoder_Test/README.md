# GM4108_Encoder_Test

Proyecto independiente de prueba del AS5048A con ESP32-DevKitC
(ESP32-WROOM-32E), Arduino y SPI. Sin control de motor ni bibliotecas externas.
El motor permanece sin alimentar y se gira manualmente.

## Cableado provisional

Con todo desconectado, comprobar fisicamente el pinout del conjunto del fabricante.
No deducir las señales por los colores del cable. Verificar que la placa del
encoder admite alimentacion a 3,3 V y como estan conectados sus pines de
alimentacion; el chip AS5048A tiene requisitos especificos para operar a 3,3 V.

| Encoder | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| SCK | GPIO18 |
| MISO | GPIO19 |
| MOSI | GPIO23 |
| CS | GPIO27 |

Los pines se cambian en las constantes al principio de `src/main.cpp`.

## Compilar, cargar y abrir el monitor

1. Abrir esta carpeta (`GM4108_Encoder_Test`) en VS Code con PlatformIO IDE.
2. Ejecutar **PlatformIO: Build**. En terminal, dentro de esta carpeta:

   ```sh
   pio run
   ```

3. Conectar el ESP32 por USB con un cable de datos y ejecutar
   **PlatformIO: Upload** o:

   ```sh
   pio device list
   pio run --target upload
   ```

   Si hay varios puertos, elegir el del ESP32 con
   `pio run --target upload --upload-port /dev/cu.PUERTO_ESP32`.
   Si la carga se queda en `Connecting...`, mantener BOOT pulsado al iniciar
   la conexion y soltarlo cuando empiece la escritura.

4. Ejecutar **PlatformIO: Serial Monitor** o:

   ```sh
   pio device monitor --baud 115200
   ```

   Con varios puertos, añadir `--port /dev/cu.PUERTO_ESP32`.
   Pulsar EN/RESET para volver a ver la cabecera. Cerrar el monitor con Ctrl+C
   antes de volver a cargar.

## Comprobacion fisica pendiente

- Verificar el cableado y la alimentacion antes de conectar el encoder.
- Confirmar la cabecera a 115200 baudios y lecturas aproximadamente cada 100 ms.
- Girar lentamente el rotor una vuelta: el angulo debe variar de forma continua,
  salvo el salto esperado entre 359,98 y 0 grados. El sentido y el cero dependen
  del montaje. El rango calculado es de 0 a 359,978 grados (16384 posiciones).
- Comprobar que una vuelta mecanica recorre aproximadamente 360 grados, que
  el valor se estabiliza al detener el rotor y que no aparecen errores recurrentes.
- Ante errores, desconectar la alimentacion y revisar pinout, masa, CS, MISO,
  MOSI y SCK. El programa valida paridad y EF y solicita limpiar el error SPI.
  El registro de error usa bit 0 para trama, bit 1 para comando invalido y bit 2
  para paridad del comando.

Una respuesta fija de cero puede tener paridad valida incluso con MISO a nivel
bajo: imprimir un angulo no demuestra por si solo que el encoder esta conectado
y funcionando. Es necesario observar la respuesta real al giro del rotor.

## Protocolo

SPI modo 1, MSB primero, 1 MHz. Cada trama tiene 16 bits y su propio pulso CS.
El comando de lectura incluye direccion, bit de lectura y paridad par. La segunda
trama envia NOP para recibir la respuesta diferida. Solo se muestra el angulo si
la paridad de los 16 bits es correcta y EF esta a cero. La lectura de `0x0001`
solicita limpiar EF; la siguiente lectura comprueba el resultado.

Referencia: [hoja de datos AS5048A/AS5048B de ams OSRAM, alojada por MikroElektronika](https://download.mikroe.com/documents/datasheets/AS5048A_datasheet.pdf).

## Validacion realizada

Compilacion `pio run` completada con resultado `SUCCESS` usando PlatformIO Core
6.2.0, espressif32 8.0.0 y Arduino ESP32 3.3.12. Binario generado en
`.pio/build/esp32dev/firmware.bin`. Uso: 22212 bytes de RAM y 300544 bytes de flash.

No se ha cargado el programa ni se han observado lecturas reales. La compilacion
no verifica las conexiones, la alimentacion, el montaje magnetico ni el
comportamiento real del encoder.
