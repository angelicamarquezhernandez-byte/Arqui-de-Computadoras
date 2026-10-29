#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>

#define VAVER   (1 << PB0)
#define VAROJ   (1 << PB1)
#define VBVER   (1 << PB2)
#define VBROJ   (1 << PB3)
#define PEATONV (1 << PB4)

// TABLAS MEMORIA FLASH (PROGMEM)

//Tiempos: Normal (1), hora pico (0.5), bajo (2)
const uint16_t tiemposFlash[3] PROGMEM = {
    1000, // Modo 0: normal
     500, // Modo 1: hora pico
    2000  // Modo 2: bajo tráfico
};

// Estructura estado del semáforo
typedef struct {
    uint8_t salidas;
    uint8_t atencion;
    uint8_t led_parpadeo;
    uint8_t sigNormal;
    uint8_t sigPeaton;
} EstadoSemaforo;

// Tabla de transición de estados y salidas
// Estados: S0, S1, S2, S3, S4, S5
const EstadoSemaforo tablaEstados[6] PROGMEM = {
    // S0
    { VAVER | VBROJ, 0, 0, 1, 1 },

    // S1
    { VBROJ, 1, VAVER, 2, 2 },

    // S2
    { VAROJ | VBVER, 0, 0, 3, 3 },

    // S3
    { VAROJ, 1, VBVER, 0, 4 },

    // S4
    { VAROJ | VBROJ, 0, 0, 5, 5 },

    // S5
    { VAROJ | VBROJ | PEATONV, 0, 0, 0, 0 }
};

// Función auxiliar de retardos
void retardo(uint16_t ms) {
    while (ms--) {
        _delay_ms(1);
    }
}

int main(void) {
    // Configurar PB0-PB4 como salidas para LEDs
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) |
            (1 << PB3) | (1 << PB4);

    PORTB &= ~0x1F;

    // Configurar PD2, PD3 y PD4 como entradas
    DDRD &= ~((1 << PD2) | (1 << PD3) | (1 << PD4));
    PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4);

    uint8_t estadoActual = 0;
    uint8_t peticionPeaton = 0;

    while (1) {
        // Leer el DIP switch (invertido)
        uint8_t dip_raw = (~PIND) >> PD2;
        uint8_t modo = dip_raw & 0x03;
        uint8_t swPeaton = (dip_raw >> 2) & 0x01;

        // Registrar solicitud peatonal
        if (swPeaton) {
            peticionPeaton = 1;
        }

        // Limitar modo al rango de 0 a 2
        if (modo > 2) modo = 0;

        // Leer tiempo de transición desde Flash
        uint16_t tiempoBase = pgm_read_word(&tiemposFlash[modo]);

        // Cargar estado actual desde Flash
        EstadoSemaforo estado;
        memcpy_P(&estado, &tablaEstados[estadoActual],
                 sizeof(EstadoSemaforo));

        // Ejecutar salidas según el estado
        if (estado.atencion) {
            uint16_t medio_tiempo = tiempoBase / 4;

            for (uint8_t i = 0; i < 2; i++) {
                PORTB = estado.salidas | estado.led_parpadeo;
                retardo(medio_tiempo);

                PORTB = estado.salidas;
                retardo(medio_tiempo);
            }
        } else {
            PORTB = estado.salidas;
            retardo(tiempoBase);
        }

        // Transición al siguiente estado
        if (peticionPeaton) {
            estadoActual = estado.sigPeaton;

            if (estadoActual == 0) {
                peticionPeaton = 0;
            }
        } else {
            estadoActual = estado.sigNormal;
        }
    }

    return 0;
}
