#include <Arduino.h>
#include <SPI.h>

// Pines provisionales: verificar los conductores antes de alimentar el encoder.
constexpr int PIN_SCK = 18;
constexpr int PIN_MISO = 19;
constexpr int PIN_MOSI = 23;
constexpr int PIN_CS = 27;
constexpr uint32_t SPI_HZ = 1000000;
constexpr uint16_t REG_ANGLE = 0x3FFF;
constexpr uint16_t REG_CLEAR_ERROR = 0x0001;
constexpr uint16_t DATA_MASK = 0x3FFF;
constexpr uint16_t ERROR_FLAG = 0x4000;
constexpr uint16_t NOP = 0x0000;

// La paridad del AS5048A es par y abarca los 16 bits de la trama.
bool hasEvenParity(uint16_t word) {
  bool odd = false;
  for (uint8_t i = 0; i < 16; ++i) {
    odd ^= (word & 1U) != 0;
    word >>= 1;
  }
  return !odd;
}

uint16_t readCommand(uint16_t address) {
  uint16_t command = (address & DATA_MASK) | 0x4000;  // Bit 14: lectura.
  if (!hasEvenParity(command)) {
    command |= 0x8000;  // Bit 15: completar paridad par.
  }
  return command;
}

uint16_t transferFrame(uint16_t command) {
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE1));
  digitalWrite(PIN_CS, LOW);
  delayMicroseconds(1);  // Margen para el establecimiento de CS.
  const uint16_t response = SPI.transfer16(command);
  delayMicroseconds(1);
  digitalWrite(PIN_CS, HIGH);
  SPI.endTransaction();
  delayMicroseconds(1);  // Separar las tramas mediante CS alto.
  return response;
}

uint16_t readRegister(uint16_t address) {
  // La primera respuesta pertenece al comando anterior y se descarta.
  transferFrame(readCommand(address));
  // El NOP recibe la respuesta al comando que acabamos de enviar.
  return transferFrame(NOP);
}

void clearSpiError() {
  const uint16_t response = readRegister(REG_CLEAR_ERROR);
  if (!hasEvenParity(response)) {
    Serial.printf("ERROR: paridad invalida al limpiar error SPI (0x%04X).\n",
                  static_cast<unsigned int>(response));
    return;
  }
  // Esta respuesta puede conservar EF: informa del error anterior al borrado.
  Serial.printf("Limpieza SPI enviada; registro de error: 0x%04X.\n",
                static_cast<unsigned int>(response & DATA_MASK));
  // La siguiente lectura de angulo comprobara si EF ha desaparecido.
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nGM4108 - AS5048A Encoder Test");
  Serial.println("Inicializando SPI...");

  // Dejar el dispositivo sin seleccionar antes de iniciar el bus.
  digitalWrite(PIN_CS, HIGH);
  pinMode(PIN_CS, OUTPUT);
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  delay(10);
  Serial.println();
}

void loop() {
  const uint16_t response = readRegister(REG_ANGLE);
  const bool parityOk = hasEvenParity(response);
  const bool spiError = (response & ERROR_FLAG) != 0;

  if (!parityOk) {
    Serial.printf("ERROR: paridad de respuesta invalida (0x%04X). Revisar cableado.\n",
                  static_cast<unsigned int>(response));
  } else if (spiError) {
    Serial.printf("ERROR: el AS5048A indica error SPI, EF=1 (0x%04X).\n",
                  static_cast<unsigned int>(response));
    clearSpiError();
  } else {
    const uint16_t rawAngle = response & DATA_MASK;
    const float degrees = rawAngle * (360.0f / 16384.0f);
    Serial.printf("Angulo: %.2f grados\n", degrees);
  }

  delay(100);  // Aproximadamente 10 lecturas por segundo.
}
