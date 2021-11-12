//-------------------------------------------------------------------------------------------------
/*
 * Universidad del Valle de Guatemala
 * BE3015 - Electrónica Digital 2
 * Proyecto 3
 * Comunicación I2C y Neopixel
 * Nombre: Diego Andrés Medina Mencos
 * Carné: 19697
 */
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Librerías
//-------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

//Archivo para la SD
File archivo;

//Pantalla TFT
#include <stdint.h>
#include <stdbool.h>
#include <TM4C123GH6PM.h>
#include <SPI.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"
//-------------------------------------------------------------------------------------------------
// Definición de pines
//-------------------------------------------------------------------------------------------------
//Botones
#define boton1 PF_4
#define boton2 PF_0

//Color del texto en la pantalla TFT
#define rojo 0xB000

//Buzzer
#define sound PB_0

//Pines de la pantalla TFT
// El SPI es el 0
//MOSI va a PA_5
//MISO va a PA_4
//SCK va a PA_2

#define LCD_RST PD_0
#define LCD_DC PD_1
#define LCD_CS PA_3


//-------------------------------------------------------------------------------------------------
// Variables Locales
//-------------------------------------------------------------------------------------------------

//Variable para el dato del sensor proveniente del ESP32
String humedad = "";

//Estado para que se muestre solo una vez el menú principal
int estado = 0;

//-------------------------------------------------------------------------------------------------
// Prototipo de funciones
//-------------------------------------------------------------------------------------------------
void memoriaSD(void);
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);

void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);

//-------------------------------------------------------------------------------------------------
// ISR
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Configuración del sistema
//-------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin (115200);
  Serial3.begin (115200);

  //Configuración de los botones
  pinMode(boton1, INPUT_PULLUP);
  pinMode(boton2, INPUT_PULLUP);

  //Configuración de la SD
  pinMode(PB_5, OUTPUT);
  
  SPI.setModule(0);
    // Estamos Inicializando la tarjeta SD
  if (!SD.begin(PB_5)) {
    Serial.println("Ha ocurrido un error!");
    return;
  }
  //Configuración buzzer
  pinMode(sound, OUTPUT);

  //Pantalla 
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  LCD_Init();
  LCD_Clear(0x00);
  //Marco rojo
  FillRect(10, 10, 299, 219, rojo);
  //Fondo de pantalla
  FillRect(20, 20, 279, 199, 0x0000);
  //Se muesta al encender el dispositivo
  String portada = "HeartBip";
  LCD_Print(portada, 80, 110, 2, 0xffff, 0x0000);
  LCD_Bitmap(210, 100, 32, 32, corazon);
  delay(3000);
}

//-------------------------------------------------------------------------------------------------
// Loop principal
//-------------------------------------------------------------------------------------------------
void loop() {
  //Menú principal 
  if (estado == 0){
    //Cambia el estado a 1
    //Fondo de pantalla
    FillRect(20, 20, 279, 199, 0x0000);
    //Pantalla de carga
    LCD_Bitmap(50, 50, 32, 23, ritmo);
    LCD_Bitmap(50, 110, 32, 23, ritmo);
    LCD_Bitmap(50, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(110, 50, 32, 23, ritmo);
    LCD_Bitmap(110, 110, 32, 23, ritmo);
    LCD_Bitmap(110, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(170, 50, 32, 23, ritmo);
    LCD_Bitmap(170, 110, 32, 23, ritmo);
    LCD_Bitmap(170, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(230, 50, 32, 23, ritmo);
    LCD_Bitmap(230, 110, 32, 23, ritmo);
    LCD_Bitmap(230, 170, 32, 23, ritmo);
    delay(200);
    //Fondo de pantalla
    FillRect(20, 20, 279, 199, 0x0000);
    String opciones = "Escoga una opcion:";
    LCD_Print("Escoga una", 70, 60, 2, 0xffff, 0x0000);
    LCD_Print("opcion:", 100, 80, 2, 0xffff, 0x0000);
    String opcion1 = "1.Medicion";
    LCD_Print(opcion1, 40, 110, 2, 0xffff, 0x0000);
    String opcion2 = "2.Guardar dato";
    LCD_Print(opcion2, 40, 150, 2, 0xffff, 0x0000);
    estado = 1;
  }
  
  //Si se presiona el botón 1, se le dice al ESP32 que lea el dato
  if(digitalRead(boton1)==0){
    delay(150);
    Serial3.println("1");
    //Fondo de pantalla
    FillRect(20, 20, 279, 199, 0x0000);
    //Pantalla de carga
    LCD_Bitmap(50, 50, 32, 23, ritmo);
    LCD_Bitmap(50, 110, 32, 23, ritmo);
    LCD_Bitmap(50, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(110, 50, 32, 23, ritmo);
    LCD_Bitmap(110, 110, 32, 23, ritmo);
    LCD_Bitmap(110, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(170, 50, 32, 23, ritmo);
    LCD_Bitmap(170, 110, 32, 23, ritmo);
    LCD_Bitmap(170, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(230, 50, 32, 23, ritmo);
    LCD_Bitmap(230, 110, 32, 23, ritmo);
    LCD_Bitmap(230, 170, 32, 23, ritmo);
    delay(200);
    //Datos del sensor recibidos del ESP32
    if(Serial3.available()>0){
    //Se lee el dato y se guarda en una variable
    humedad = Serial3.readStringUntil('\n');
    }
    //Fondo de pantalla
    FillRect(20, 20, 279, 199, 0x0000);
    //Si se escoge realizar una medición
    LCD_Print("Ritmo cardiaco:", 90, 80, 2, 0xffff, 0x0000);
    LCD_Print(humedad, 110, 120, 2, 0xffff, 0x0000);
    LCD_Print("bpm", 190, 120, 2, 0xffff, 0x0000);
    //Si no hay tanta humedad solo aparece una gota
    if(humedad.toInt() < 34){
      LCD_Bitmap(107, 160, 32, 32, corazonEnfermo);
      LCD_Bitmap(144, 160, 32, 32, corazonEnfermo);
      LCD_Bitmap(180, 160, 32, 32, corazonEnfermo);
    }
    //Si hay una humedad moderada aparecen dos gotas
    if(34 < humedad.toInt() && humedad.toInt() < 68){
      LCD_Bitmap(107, 160, 32, 32, corazonOpaco);
      LCD_Bitmap(144, 160, 32, 32, corazonOpaco);
      LCD_Bitmap(180, 160, 32, 32, corazonOpaco);
    }
    //Si hay una humedad moderada aparecen dos gotas
    if(humedad.toInt() > 68){
      LCD_Bitmap(107, 160, 32, 32, corazon);
      LCD_Bitmap(144, 160, 32, 32, corazon);
      LCD_Bitmap(180, 160, 32, 32, corazon);
    }
    //Sonido de medición 
    tone(sound,440,100);
    delay(100);
    tone(sound,523.25,100);
    delay(100);
    tone(sound,587.33,100);
    delay(200);
    tone(sound,587.33,100);
    delay(200);
    tone(sound,587.33,100);
    delay(100);
    tone(sound,659.25,100);
    delay(100);
    tone(sound,698.45,100);
    delay(200);
    tone(sound,698.45,100);
    delay(200);
    tone(sound,698.45,100);
    delay(100);
    tone(sound,783.99,100);
    delay(100);
    tone(sound,659.25,100);
    delay(200);
    tone(sound,659.25,100);
    delay(200);
    tone(sound,587.33,100);
    delay(100);
    tone(sound,523.25,100);
    delay(100);
    tone(sound,523.25,100);
    delay(100);
    tone(sound,587.33,100);
    delay(300);

    noTone(sound);   
    estado = 0;
  }

  //Si se presiona el botón 2, se guardan los datos en la memoria SD
  if(digitalRead(boton2)==0){
    delay(150);
    memoriaSD();
    Serial3.println("2");
    //Fondo de pantalla
    LCD_Bitmap(50, 50, 32, 23, ritmo);
    LCD_Bitmap(50, 110, 32, 23, ritmo);
    LCD_Bitmap(50, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(110, 50, 32, 23, ritmo);
    LCD_Bitmap(110, 110, 32, 23, ritmo);
    LCD_Bitmap(110, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(170, 50, 32, 23, ritmo);
    LCD_Bitmap(170, 110, 32, 23, ritmo);
    LCD_Bitmap(170, 170, 32, 23, ritmo);
    delay(200);
    LCD_Bitmap(230, 50, 32, 23, ritmo);
    LCD_Bitmap(230, 110, 32, 23, ritmo);
    LCD_Bitmap(230, 170, 32, 23, ritmo);
    delay(200);
    //Fondo de pantalla
    FillRect(20, 20, 279, 199, 0x0000);
    //Si se escoge guardar un dato
    LCD_Print("El dato se", 75, 80, 2, 0xffff, 0x0000);
    LCD_Print("ha guardado", 70, 120, 2, 0xffff, 0x0000);
    LCD_Print("exitosamente!", 60, 160, 2, 0xffff, 0x0000);
    //Sonido de guardado
    tone(sound,440,100);
    delay(100);
    tone(sound,523.25,100);
    delay(100);
    tone(sound,587.33,100);
    delay(200);
    tone(sound,587.33,100);
    delay(200);
    tone(sound,587.33,100);
    delay(100);
    tone(sound,659.25,100);
    delay(100);
    tone(sound,698.45,100);
    delay(200);
    tone(sound,698.45,100);
    delay(200);
    tone(sound,698.45,100);
    delay(100);
    tone(sound,783.99,100);
    delay(100);
    tone(sound,659.25,100);
    delay(200);
    tone(sound,659.25,100);
    delay(200);
    tone(sound,587.33,100);
    delay(100);
    tone(sound,523.25,100);
    delay(100);
    tone(sound,587.33,100);
    delay(400);
    noTone(sound); 
    estado = 0;   
  }

  
  delay(5000);
}

//-------------------------------------------------------------------------------------------------
// Función para guardar los datos en la memoria SD
//-------------------------------------------------------------------------------------------------
void memoriaSD(void){
    //Se abre el documento para escribir
    archivo = SD.open("Humedad.csv", FILE_WRITE);
  
  // Si se abrió el documento, entonces se escriben los datos
  if (archivo) {
    //Se muestra en el monitor que se están guardando los datos
    Serial.println("Se guardó el siguiente dato: ");
    Serial.print("Humedad: ");
    Serial.println(humedad);

    //Se imprimen los datos en la memoria SD
    archivo.print("10/11/21");
    archivo.print(",");
    archivo.println(humedad);
   
    //Cerramos el documento
    archivo.close();
    Serial.println("Dato guardado exitosamente.");
  } 
  else {
    //Si no se logra abrir el archivo, aparece un mensaje
    Serial.println("No se pudo abrir el archivo");
  }
  
}

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_DC, LOW);
  SPI.transfer(cmd);
}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_DC, HIGH);
  SPI.transfer(data);
}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  unsigned int i;
  for (i = 0; i < h; i++) {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_DC, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    }
  digitalWrite(LCD_CS, HIGH);
}
