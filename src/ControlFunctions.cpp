#ifndef CtrlFuncImp
#define CtrlFuncImp
#include <Arduino.h>
#include "GlobalStructures.h"
#include "EeFunctions.h"

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

void botonOnOff(bool estadoBoton, unsigned long tactualBoton, unsigned long tiempoParaOn, unsigned long tiempoParaOff, unsigned long *tPrev, estadosCalefaccion *estadoSistema)
{
    /**
     * @brief Boton que cambia entre dos estados de calefaccion (On, Off) con tiempos de manera
     *      Asincrona
     * @param estadoBoton boolean que indica si el boton esta pulsado o no
     * @param tiempoParaOn tiempo que debe estar pulsado el boton para pasar al estado on (ms)
     * @param tiempoParaOff tiempo que debe estar pulsado el boton para pasar al estado off (ms)
     * @param tPrev puntero que indica el tiempo previo, necesario para poder comprar (ms)
     * @param estadoSistema puntero que apunta al estado del sistema, puede ser On, Off o Viaje
     *
     * @return VOID
     */
    if (estadoBoton == HIGH)
        *tPrev = tactualBoton;
    switch (*estadoSistema)
    {
    case On:
        if (tactualBoton - *tPrev > tiempoParaOff)
        {
            *estadoSistema = CambiandoOff;
        }
        break;
    case CambiandoOff:
        if (estadoBoton == HIGH)
            *estadoSistema = Off;
        break;
    case Off:
        if (tactualBoton - *tPrev > tiempoParaOn)
        {
            *estadoSistema = CambiandoOn;
        }
        break;
    case CambiandoOn:
        if (estadoBoton == HIGH)
            *estadoSistema = On;
        break;
    default:
        break;
    }
}
void botonViaje(bool estadoBoton, unsigned long tactualBoton, unsigned long tEntradaViaje, unsigned long tSalidaViaje, unsigned long *tPrev, estadosCalefaccion *sistema, estadosCalefaccion *sistemaPrevViaje)
{
    /**
     *
     * @brief Boton para la entrada y salida del modo viaje
     * @param estadoBoton indica si el boton esta presionado o no
     * @param tactualBoton tiempo actual del sistema
     * @param tEntradaViaje tiempo para entrar al modo viaje
     * @param tSalidaViaje tiempo para salir del modo viaje
     * @param tPrev puntero tipo unsigned long para el tiempo previo de pulsacion
     * @param sistema puntero al estado de nuestro sistema
     * @param sistemaPrevViaje sistema previo a la entrada del viaje
     *
     * @return VOID
     */
    if (estadoBoton == HIGH)
        *tPrev = tactualBoton;
    switch (*sistema)
    {
    case On:
        if (tactualBoton - *tPrev > tEntradaViaje)

        {
            *sistema = CambiandoViaje;
        }
        break;
    case Off:

        if (tactualBoton - *tPrev > tEntradaViaje)
        {
            *sistema = CambiandoViaje;
        }
        break;
    case CambiandoViaje:
        if (estadoBoton == HIGH)
        {
            *sistema = Viaje;
        }
        break;
    case Viaje:
        if (tactualBoton - *tPrev > tSalidaViaje)
        {
            *sistema = VolviendoViaje;
        }
        break;
    case VolviendoViaje:
        if (estadoBoton == HIGH)
        {
            *sistema = *sistemaPrevViaje;
        }
        break;
    default:
        break;
    }
}
void botonReset(bool estadoBoton, unsigned long tactual, unsigned long T, t_heating_system *sistema)
{
    static unsigned long tPrev;
    static unsigned char estadoSistemaBoton = 0;
    if (estadoBoton == HIGH)
    {
        tPrev = tactual;
    }
    switch (estadoSistemaBoton)
    {
    case 0:
        if (tactual - tPrev >= T)
        {
            estadoSistemaBoton = 1;
            tPrev = tactual;
        }
        break;
    case 1:
        if (estadoBoton == HIGH)
        {
            estadoSistemaBoton = 2;
        }
        break;
    case 2:
        //De_eeprom_a_structura_fabrica(sistema);
        Serial.println("BORRANDO DIGITO DE MOD CONFIG");
        //EEPROM.update(0,0);
        estadoSistemaBoton = 0;
        break;
    default:
        break;
    }
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
void Imprimir(const char nombre[], float valor)
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