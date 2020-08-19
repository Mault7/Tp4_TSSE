/**
 * Validacion de inicializacion con el i2c
 * Validacion de Lectura de datos con i2c
 * Validacion de escritura de datos con i2c
 * validacion de enmascaramiento de datos de los reistros i2c
 * Validacion de lectura de temperatura por puerto i2c
 */

#include "unity.h"
#include "max30105.h"
#include "stdbool.h"
#include "mock_sapi.h"

void test_inicializacion_con_el_i2c(void)
{

    bool_t valor_esperado = TRUE;

    //when
    i2cInit_IgnoreAndReturn(valor_esperado);

    //then
    bool_t espectativa_de_valor = MAX30105Init();
    TEST_ASSERT_EQUAL(valor_esperado, espectativa_de_valor);
}

void test_de_lectura_de_datos_con_i2c(void)
{
    bool_t valor_esperado = 20;
    uint8_t id_del_i2c = 50;
    uint8_t registro_para_leer = 10;

    //WHEN
    i2cRead_IgnoreAndReturn(valor_esperado);

    //tTHEN
    uint8_t espectativa_de_valor = ReadRegister(id_del_i2c, registro_para_leer);
    TEST_ASSERT_EQUAL(valor_esperado, espectativa_de_valor);
}

void test_de_escritura_de_datos_en_el_puerto_i2c(void)
{
    bool_t valor_esperado = 30;
    uint8_t id_del_i2c = 50;
    uint8_t registro_para_leer_y_escribir = 10;
    uint8_t valor_de_escritura_en_el_registro = valor_esperado;

    //WHEN
    i2cWrite_IgnoreAndReturn(valor_esperado);

    //THEN
    uint8_t espectativa_de_valor_escritura_de_i2c = WriteRegister(id_del_i2c, registro_para_leer_y_escribir, valor_de_escritura_en_el_registro);
    TEST_ASSERT_EQUAL(valor_esperado, espectativa_de_valor_escritura_de_i2c);

    i2cRead_IgnoreAndReturn(espectativa_de_valor_escritura_de_i2c);
    uint8_t espectativa_de_valor_de_lectura_i2c = ReadRegister(id_del_i2c, registro_para_leer_y_escribir);
    TEST_ASSERT_EQUAL(valor_esperado, espectativa_de_valor_de_lectura_i2c);
}

void test_enmascaramiento_de_datos_con_lectura_y_escritura_de_datos_i2c(void)
{

    uint8_t id_del_i2c = 50;
    uint8_t registro_para_leer_y_escribir = 40;
    uint8_t valor_esperado_de_lecturai2c = 30;
    uint8_t parametro_de_enmascaramiento = 20;
    uint8_t mascara_de_bits = 10;
    bool_t valor_esperado = ((valor_esperado_de_lecturai2c & mascara_de_bits) | parametro_de_enmascaramiento);

    //when
    i2cRead_IgnoreAndReturn(valor_esperado_de_lecturai2c);
    i2cWrite_IgnoreAndReturn((valor_esperado_de_lecturai2c & mascara_de_bits) | parametro_de_enmascaramiento);

    //then
    uint8_t espectativa_de_valor_de_enmascaramiento = bitmask(registro_para_leer_y_escribir, mascara_de_bits, parametro_de_enmascaramiento);
    TEST_ASSERT_EQUAL(espectativa_de_valor_de_enmascaramiento, espectativa_de_valor_de_enmascaramiento);
}

void test_de_sensado_de_temperatura_por_comunicacion_i2c(void)
{

    uint8_t temperatura_deseada = 38;
    uint8_t lectura_deseada_del_registro = 36;
    //When
    i2cWrite_IgnoreAndReturn(1);
    delayRead_IgnoreAndReturn(TRUE);
    i2cRead_IgnoreAndReturn(lectura_deseada_del_registro);

    //Then
    uint8_t espectativa_de_valor_de_temperatura = readTemperature();
    TEST_ASSERT_EQUAL(temperatura_deseada, espectativa_de_valor_de_temperatura);
}