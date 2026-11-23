#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LedController {
  private:
    Adafruit_NeoPixel _strip;
    uint16_t _numPixels;
    uint8_t _cor[3];
    uint8_t _brilho;
    bool _ligado;

  public:
    LedController(uint16_t numPixels, uint8_t pin, neoPixelType type = NEO_RGB + NEO_KHZ800);
    
    void begin();
    void enviaConfig();
    void render();
    void refreshCompleto();
    
    // Controles de estado
    void ligar();
    void desligar();
    void aumentarBrilho(uint8_t passo = 25);
    void diminuirBrilho(uint8_t passo = 25, uint8_t minimo = 10);
    void setCor(uint8_t r, uint8_t g, uint8_t b, bool forcarLigar = false);
    
    bool isLigado() const;
};

#endif
