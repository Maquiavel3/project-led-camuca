#include <IRremote.hpp>
#include "LedController.h"

// Definições de Hardware
#define IR_PIN    2
#define DATA_PIN  6
#define N_PIXELS  30

// Códigos do Controle Remoto IR
enum IrComandos : uint8_t {
  CMD_DESLIGAR   = 0x1F,
  CMD_LIGAR      = 0x0D,
  CMD_BRILHO_UP  = 0x09,
  CMD_BRILHO_DN  = 0x1D,
  CMD_AZUL       = 0x11,
  CMD_VERMELHO   = 0x19,
  CMD_VERDE      = 0x1B,
  CMD_ROXO       = 0x0E,
  CMD_BRANCO     = 0x15
};

LedController fitaLed(N_PIXELS, DATA_PIN, NEO_RGB + NEO_KHZ800);

uint8_t  ultimoCmd = 0;
uint32_t tRefresh  = 0;

// --- Implementação dos métodos de LedController ---
LedController::LedController(uint16_t numPixels, uint8_t pin, neoPixelType type)
  : _strip(numPixels, pin, type), _numPixels(numPixels), _brilho(128), _ligado(true) {
  _cor[0] = 255;
  _cor[1] = 255;
  _cor[2] = 255;
}

void LedController::begin() {
  _strip.begin();
  delay(200); // Tempo para os CIs TM1903 iniciarem
  refreshCompleto();
}

void LedController::enviaConfig() {
  for (uint16_t i = 0; i < _numPixels; i++) {
    _strip.setPixelColor(i, 0xFF, 0x52, 0x94);
  }
  _strip.show();
  delayMicroseconds(100);
}

void LedController::render() {
  uint8_t r = 0, g = 0, b = 0;
  if (_ligado) {
    r = (uint16_t)_cor[0] * _brilho / 255;
    g = (uint16_t)_cor[1] * _brilho / 255;
    b = (uint16_t)_cor[2] * _brilho / 255;
  }
  if (r > 254) r = 254; // Trava específica para o protocolo TM1903
  
  for (uint16_t i = 0; i < _numPixels; i++) {
    _strip.setPixelColor(i, r, g, b);
  }
  _strip.show();
}

void LedController::refreshCompleto() {
  enviaConfig();
  render();
}

void LedController::ligar() { _ligado = true; }
void LedController::desligar() { _ligado = false; }

void LedController::aumentarBrilho(uint8_t passo) {
  _brilho = min((int)_brilho + passo, 255);
}

void LedController::diminuirBrilho(uint8_t passo, uint8_t minimo) {
  _brilho = max((int)_brilho - passo, (int)minimo);
}

void LedController::setCor(uint8_t r, uint8_t g, uint8_t b, bool forcarLigar) {
  _cor[0] = r;
  _cor[1] = g;
  _cor[2] = b;
  if (forcarLigar) _ligado = true;
}

bool LedController::isLigado() const { return _ligado; }

// --- Fluxo Principal Arduino ---
void setup() {
  Serial.begin(115200);
  fitaLed.begin();
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {
    bool repeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;
    uint8_t cmd = repeat ? ultimoCmd : IrReceiver.decodedIRData.command;
    IrReceiver.resume();

    bool mudou = true;
    if (repeat && cmd != CMD_BRILHO_UP && cmd != CMD_BRILHO_DN) {
      mudou = false;
    } else {
      ultimoCmd = cmd;
      Serial.print("Comando: 0x");
      Serial.println(cmd, HEX);

      switch (cmd) {
        case CMD_DESLIGAR:  fitaLed.desligar(); break;
        case CMD_LIGAR:     fitaLed.ligar(); break;
        case CMD_BRILHO_UP: fitaLed.aumentarBrilho(); break;
        case CMD_BRILHO_DN: fitaLed.diminuirBrilho(); break;
        case CMD_AZUL:      fitaLed.setCor(255, 0, 0); break;
        case CMD_VERMELHO:  fitaLed.setCor(0, 255, 0); break;
        case CMD_VERDE:     fitaLed.setCor(0, 0, 255); break;
        case CMD_ROXO:      fitaLed.setCor(180, 255, 0, true); break;
        case CMD_BRANCO:    fitaLed.setCor(255, 255, 255, true); break;
        default:            mudou = false;
      }
    }

    if (mudou) {
      fitaLed.render();
      tRefresh = millis();
    }
  }

  // Keep-alive: reenvia o estado atual periodicamente sem alterá-lo
  if (IrReceiver.isIdle() && (millis() - tRefresh > 200)) {
    fitaLed.render();
    tRefresh = millis();
  }
}
