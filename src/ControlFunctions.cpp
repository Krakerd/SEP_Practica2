#ifndef CtrlFuncImp
#define CtrlFuncImp
#include <Arduino.h>
#include "GlobalStructures.h"

//**************************************************************************************************************
//           FUNCIONES DE CONTROL
//**************************************************************************************************************
estadosAlimentacion estadoUPS(float voltaje, float margen, float valor)
{
    /**
     * @brief Funcion para el control de los estados de la alimentacion
     * @param voltaje voltaje deseado, debe ser el limite inferior de nuestra franja aceptada
     * @param margen margen de sobre voltaje aceptado para la alimentacion
     * @param valor Valor de voltaje real, NO VOLTAJE MEDIDO POR EL ADC
     *
     * @return Un estado de entre ALIMENTACION_OK, ALIMENTACION_SOBRE, ALIMENTACION_DESC
     */
    static estadosAlimentacion resultado = estadosAlimentacion::ALIMENTACION_OK;
    if (voltaje > (valor + margen))
    {
        resultado = estadosAlimentacion::ALIMENTACION_SOBRE;
    }
    if (voltaje < valor)
    {
        resultado = estadosAlimentacion::ALIMENTACION_DESC;
    }
    if (voltaje <= (valor + margen) && voltaje >= valor)
    {
        resultado = estadosAlimentacion::ALIMENTACION_OK;
    }
    return resultado;
}

void botonPermutaEstados(bool estadoBoton, unsigned long tiempoPara1, unsigned long tiempoPara2, unsigned long *tPrev, estadosCalefaccion *estadoSistema, estadosCalefaccion estado1, estadosCalefaccion estado2)
{
    /**
     * @brief Boton que cambia entre dos estados de calefaccion (On, Off, Viaje) con tiempos de manera
     *      Asincrona
     * @param estadoBoton boolean que indica si el boton esta pulsado o no
     * @param tiempoPara1 tiempo que debe estar pulsado el boton para pasar al estado 1 (ms)
     * @param tiempoPara2 tiempo que debe estar pulsado el boton para pasar al estado 2 (ms)
     * @param tPrev puntero que indica el tiempo previo, necesario para poder comprar (ms)
     * @param estadoSistema puntero que apunta al estado del sistema, puede ser On, Off o Viaje
     * @param estado1 estado 1 de entre los dos estados a los que se puede cambiar
     * @param estado2 estado 2 de entre los dos estados a los que se puede cambiar
     *
     * @return VOID
     */
    unsigned long tactualBoton = millis();
    if (*estadoSistema == estado1)
    {
        if (tactualBoton - *tPrev > tiempoPara2)
        {
            if (estadoBoton == HIGH)
            {
                *estadoSistema = On;
                *tPrev = tactualBoton;
            }
        }
    }
    else if (*estadoSistema == estado2)
    {
        if (tactualBoton - *tPrev > tiempoPara1)
        {
            if (estadoBoton == HIGH)
            {
                *estadoSistema = Off;
                *tPrev = tactualBoton;
            }
        }
    }
    if (estadoBoton == HIGH)
        *tPrev = tactualBoton;
}

void activacionElectrovalvula(int pin, unsigned long tactual, unsigned long *prev, unsigned long T, estadosValvula *valvula, estadosValvula *estadoPrev)
{
    /**
     * @brief Permutar el estado de una valvula
     *
     * @param pin define el pin al que se encuentra conectada la valvula
     * @param tactual tiempo actual del sistema desde el arranque (ms)
     * @param prev tiempo previo, define el inicio de la permutacion de estado (ms)
     * @param T tiempo de permutacion (ms)
     * @param valvula estado de la valvula, puede ser Cerrado, Abierto o Cambiando
     * @param valvulaAnterior estado anterior de la valvula, necesario para saber a donde cambiar
     *
     * @return VOID
     */
    switch (*valvula)
    {
    case estadosValvula::Cerrado:
        digitalWrite(pin, HIGH);
        *prev = tactual;
        *estadoPrev = *valvula;
        *valvula = estadosValvula::Cambiando;
        break;
    case estadosValvula::Cambiando:
        if (tactual - *prev > T)
        {
            digitalWrite(pin, LOW);
            if (*estadoPrev == estadosValvula::Abierto)
            {
                *valvula = estadosValvula::Cerrado;
            }
            else if (*estadoPrev == estadosValvula::Cerrado)
            {
                *valvula = estadosValvula::Abierto;
            }
        }
        break;
    case estadosValvula::Abierto:
        digitalWrite(pin, HIGH);
        *prev = tactual;
        *estadoPrev = *valvula;
        *valvula = estadosValvula::Cambiando;
        break;
    }
}

void histesis(t_heating_floor *piso)
{
    /**
     * @brief Control de histeresis de la zona
     */
    if (piso->temperatura > piso->temperaturaObjetivo + piso->histeresis)
    {
        piso->necesitaCalefaccion = false;
    }
    else if (piso->temperatura < piso->temperaturaObjetivo - piso->histeresis)
    {
        piso->necesitaCalefaccion = true;
    }
}

void cerradoSistema(t_heating_system *sistema)
{
    /**
     * @brief Cierra el sistema de calefaccion
     */
    unsigned long tiempoActual = millis();
    // Serial.println("IMPRIMIR CIERRE");
    if (sistema->pisos[0].valvula != estadosValvula::Cerrado)
    {
        activacionElectrovalvula(sistema->pisos[0].pinValvula, tiempoActual, &sistema->pisos[0].tPrevValvula, 1000, &sistema->pisos[0].valvula, &sistema->pisos[0].valvulaAnterior);
    }
    else
        sistema->pisos[0].valvulaAnterior = Cerrado; // Seguridad para cambio de estados
    if (sistema->pisos[1].valvula != estadosValvula::Cerrado)
    {
        activacionElectrovalvula(sistema->pisos[1].pinValvula, tiempoActual, &sistema->pisos[1].tPrevValvula, 1000, &sistema->pisos[1].valvula, &sistema->pisos[1].valvulaAnterior);
    }
    else
        sistema->pisos[1].valvulaAnterior = Cerrado; // Seguridad para cambio de estados
    if (sistema->valvulaPrincipal != estadosValvula::Cerrado)
    {
        activacionElectrovalvula(sistema->pinPrincipal, tiempoActual, &sistema->tPrevValvula, 1000, &sistema->valvulaPrincipal, &sistema->valvulaPrincipalAnterior);
        digitalWrite(sistema->pinCaldera, LOW);
    }
    else
        sistema->valvulaPrincipalAnterior = Cerrado; // Seguridad para cambio de estados
    if (sistema->bombaPrincipal == 1)
        sistema->temperaturaAcumulador = 0;
}

void controlZona(t_heating_floor *zona, bool activacion, unsigned long time)
{
    /**
     * @brief Realiza el control de la valvula de zona
     *
     * @param zona puntero que otorga la zona
     * @param activacion indica si la zona debe estar activada o no
     * @param time tiempo actua necesario para la activacion
     *
     * @return VOID
     */
    if (activacion)
    {
        if (zona->valvula != estadosValvula::Abierto)
        {
            activacionElectrovalvula(zona->pinValvula, time, &zona->tPrevValvula, 1000, &zona->valvula, &zona->valvulaAnterior);
        }
        else
            zona->valvula = Abierto;
    }
    else
    {
        if (zona->valvula != estadosValvula::Cerrado)
        {
            activacionElectrovalvula(zona->pinValvula, time, &zona->tPrevValvula, 1000, &zona->valvula, &zona->valvulaAnterior);
        }
    }
}

//**************************************************************************************************************
//           FUNCIONES DE USO GENERAL
//**************************************************************************************************************
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
float getCommandFloat(String command)
{
    String valorS = command.substring(command.lastIndexOf(' '));
    float valor = valorS.toFloat();
    return valor;
}
void Imprimir(char nombre[], float valor)
{
    // FUNCION HECHA PARA IMPRIMIR CON TELEPLOT (EXTENSION DE VSCODE)
    Serial.print(">");
    Serial.print(nombre);
    Serial.print(":");
    Serial.println(valor);
}
void ImprimirArduino(char nombre[], float valor)
{
    // FUNCION HECHA PARA IMPRIMIR CON SERIAL PLOTTER (ARDUINO)
    Serial.print(nombre);
    Serial.print(":");
    Serial.print(valor);
    Serial.print(",");
}
#endif