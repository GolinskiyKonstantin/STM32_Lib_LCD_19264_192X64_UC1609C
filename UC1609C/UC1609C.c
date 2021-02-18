/*

  ******************************************************************************
  * @file 			( фаил ):   UC1609C.c
  * @brief 		( описание ):  	
  ******************************************************************************
  * @attention 	( внимание ):	author: Golinskiy Konstantin	e-mail: golinskiy.konstantin@gmail.com
  ******************************************************************************
  
*/

/* Includes ----------------------------------------------------------*/
#include "UC1609C.h"


// Display  Size ------------------------------------------------

const uint8_t UC1609C_WIDTH = UC1609C_width;
const uint8_t UC1609C_HEIGHT = UC1609C_height;

//---------------------------------------------------------------


uint8_t buffer[ ( UC1609C_WIDTH * UC1609C_HEIGHT / 8 ) + 1 ];  						// буфер кадра, create a full screen buffer (192 * 64/8) + 1
uint8_t bufferWidth = UC1609C_WIDTH;
uint8_t bufferHeight = UC1609C_HEIGHT;
	

//===================================================================================================================


/*
	******************************************************************************
	* @brief	 ( описание ):  отправка команд и данных на дисплей
	* @param	( параметры ):	1- команда, 2 - данные
	* @return  ( возвращает ):	
	******************************************************************************
*/
static void UC1609C_sendCommand( uint8_t command, uint8_t value )
{	
	uint8_t data = command | value;
	
	HAL_GPIO_WritePin( DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET );
	
	HAL_SPI_Transmit( &UC1609C_SPI_HAL, &data, 1, HAL_MAX_DELAY );
	
	HAL_GPIO_WritePin( DC_GPIO_Port, DC_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  сброс дисплея ( нужен при инициализации )
	* @param	( параметры ):	
	* @return  ( возвращает ):	
	******************************************************************************
*/
static void UC1609C_reset( void ){
	HAL_GPIO_WritePin( RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET );
	HAL_Delay( 150 );															// mS delay . datasheet says 3uS
	HAL_GPIO_WritePin( RST_GPIO_Port, RST_Pin, GPIO_PIN_SET );
	HAL_Delay( 150 );															// mS delay . DataSheet says 5mS
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  инициализация дисплея
	* @param	( параметры ):	
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_init( void )
{	
	HAL_Delay( 50 );															//3mS delay, datasheet
	HAL_GPIO_WritePin( DC_GPIO_Port, DC_Pin, GPIO_PIN_SET );
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
	
	UC1609C_reset();
	
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );
	
	HAL_Delay( 10 );

	UC1609C_sendCommand( UC1609C_TEMP_COMP_REG, UC1609C_TEMP_COMP_SET ); 		// Temperature Compensation Register, TC[1:0] = 00b= -0.00%/ C
	
	//--------------------------------------------------------------------------------------------------------------------------
	//UC1609C_sendCommand( UC1609C_ADDRESS_CONTROL, UC1609_ADDRESS_SET ); 		// set RAM address control for UC1609
	UC1609C_sendCommand( UC1609C_ADDRESS_CONTROL, UC1609C_ADDRESS_SET ); 		// set RAM address control for UC1609C
	//--------------------------------------------------------------------------------------------------------------------------
	
	UC1609C_sendCommand( UC1609C_FRAMERATE_REG, UC1609C_FRAMERATE_SET );
	UC1609C_sendCommand( UC1609C_BIAS_RATIO, UC1609C_BIAS_RATIO_SET );  		// set bias ratio to default
	UC1609C_sendCommand( UC1609C_POWER_CONTROL,  UC1609C_PC_SET ); 
  
	HAL_Delay( 100 );															//  mS delay
  
	UC1609C_sendCommand( UC1609C_GN_PM, 0 );									// set gain and potentiometer - double byte command
	
	//--------------------------------------------------------------------------------------------------------------------------
	UC1609C_sendCommand( UC1609C_GN_PM, 0x1E ); 								// Контрасность по умолчанию, default = 0x49 , range 0x00 to 0xFE set gain and potentiometer
	//--------------------------------------------------------------------------------------------------------------------------
	
	UC1609C_sendCommand( UC1609C_DISPLAY_ON, 0x01 ); 							// turn on display
	
	//--------------------------------------------------------------------------------------------------------------------------
	UC1609C_sendCommand( UC1609C_LCD_CONTROL, UC1609C_ROTATION_NORMAL ); 		// Ротация по умолчанию, rotate to normal 
	//--------------------------------------------------------------------------------------------------------------------------
	
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  установка контрасности дисплея
	* @param	( параметры ):	значение от 0....255 ( по умолчанию 30 )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_contrast (uint8_t bits) 
{
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );
	
	UC1609C_sendCommand( UC1609C_GN_PM, 0 );
	UC1609C_sendCommand( UC1609C_GN_PM, bits ); 								// Контрасность по умолчанию, default = 0x49 , range 0x00 to 0xFE set gain and potentiometer
	
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  включение и отключение информации на дисплее ( с сохранением информации )
	* @param	( параметры ):	1 - ON    0 - OFF
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_enable (uint8_t bits) 
{
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );
	
	UC1609C_sendCommand( UC1609C_DISPLAY_ON, bits );
	
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  скроллинг дисплея вертикально
	* @param	( параметры ):	значение от 0 до 64
									на сколько строк сдвигаем 
										от 0 до 64 двигаем вверх
										от 64 до 0 двигаем вниз
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_scroll (uint8_t bits) 
{
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );
	
	UC1609C_sendCommand( UC1609C_SCROLL, bits );
	
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  ротация дисплея
	* @param	( параметры ):	Param1: 4 possible values 000 010 100 110 (defined)
								указываем параметр ротации:
									UC1609C_ROTATION_FLIP_TWO
									UC1609C_ROTATION_NORMAL
									UC1609C_ROTATION_FLIP_ONE
									UC1609C_ROTATION_FLIP_THREE
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_rotate(uint8_t rotatevalue) 
{
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );
	
	switch ( rotatevalue )
	{
      case 0: 
	  {
		  rotatevalue = 0; 
		  break;
	  }
      case 0x02: 
	  {
		  rotatevalue = UC1609C_ROTATION_FLIP_ONE; 	
		  break;
	  }
      case 0x04:
	  {
		  rotatevalue = UC1609C_ROTATION_NORMAL; 
		  break;
	  }
      case 0x06: 
	  {
		  rotatevalue = UC1609C_ROTATION_FLIP_TWO; 	
		  break;
	  }
      default: 
	  {
		  rotatevalue = UC1609C_ROTATION_NORMAL; 	
		  break;
	  }
	}
	UC1609C_sendCommand( UC1609C_LCD_CONTROL, rotatevalue );
	
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  инверсия дисплея
	* @param	( параметры ):	Param1: bits, 1 invert , 0 normal
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_invertDisplay (uint8_t bits) 
{
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );
	
	UC1609C_sendCommand( UC1609C_INVERSE_DISPLAY, bits );
	
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  вывод на дисплей всех точек сразу ( закрашиваем полностью дисплей )
	* @param	( параметры ): 	параметр:	1- все точки включены, 0 - все точки выключены
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_allPixelsOn(uint8_t bits) 
{
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );
	
	UC1609C_sendCommand( UC1609C_ALL_PIXEL_ON, bits );

	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  заполняем весь дисплей битами ( только дисплей буффер не трогаем )
								( например если 0x00 то все пустое, 0xFF то все закрашено, 0x55 ( 0в01010101 ) то все через строчку )
	* @param	( параметры ):	1- команда, 2 - данные
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_fillScreen(uint8_t dataPattern) 
{
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );
	
	uint16_t numofbytes = UC1609C_WIDTH * (	UC1609C_HEIGHT / 8 ); 							// width * height
	
	for ( uint16_t i = 0; i < numofbytes; i++ ) 
	{
		HAL_SPI_Transmit( &UC1609C_SPI_HAL, &dataPattern, 1, HAL_MAX_DELAY );
	}
	
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  вывод буфера на дисплей
	* @param	( параметры ):	параметры:
								//Param1: x offset 0-192
								//Param2: y offset 0-64
								//Param3: width 0-192
								//Param4 height 0-64
								//Param5 сам буфер
	* @return  ( возвращает ):	
	******************************************************************************
*/
static void UC1609C_buffer( int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t* data ) 
{
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET );

	uint8_t tx, ty; 
	uint16_t offset = 0; 
	uint8_t column = (x < 0) ? 0 : x;
	uint8_t page = (y < 0) ? 0 : y/8;

	  for ( ty = 0; ty < h; ty = ty + 8 ) 
	  {
		if ( y + ty < 0 || y + ty >= UC1609C_HEIGHT )
		{
			continue;
		}
		
		UC1609C_sendCommand( UC1609C_SET_COLADD_LSB, ( column & 0x0F )); 
		UC1609C_sendCommand( UC1609C_SET_COLADD_MSB, ( column & 0XF0 ) >> 4 ); 
		UC1609C_sendCommand( UC1609C_SET_PAGEADD, page++ ); 
	 
		for (tx = 0; tx < w; tx++) 
		{
			  if (x + tx < 0 || x + tx >= UC1609C_WIDTH)
			  {
				continue;
			  }
			  
			  offset = ( w * ( ty / 8 )) + tx; 
			  HAL_SPI_Transmit( &UC1609C_SPI_HAL, (uint8_t*)&data[offset++], 1, HAL_MAX_DELAY );
		}
	  }
  
	HAL_GPIO_WritePin( CS_GPIO_Port, CS_Pin, GPIO_PIN_SET );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  вывод информации из буфера на дисплей
								( вызывается каждый раз как нужно вывести данные на дисплей )
								( например вывели текс и после вызываем функцию UC1609C_update( void ) )
	* @param	( параметры ):	
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_update( void ) 
{
	uint8_t x = 0; 
	uint8_t y = 0; 
	uint8_t w = bufferWidth; 
	uint8_t h = bufferHeight;
	
	UC1609C_buffer( x,  y,  w,  h, (uint8_t*)buffer );
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  рисует пиксель по указанным координатам ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	Param1: x offset 0-192
								Param2: y offset 0-64
								Param3: вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/

void UC1609C_drawPixel( int16_t x, int16_t y, uint8_t colour ) 
{
	if (( x < 0 ) || ( x >= bufferWidth ) || ( y < 0 ) || ( y >= bufferHeight )) {
		return;
	}
      uint16_t tc = ( bufferWidth * ( y / 8 )) + x; 
	
      switch ( colour )
      {
        case FOREGROUND:
		{
			buffer[tc] |= ( 1 << ( y & 7 )); 
			break;
		}
        case BACKGROUND:  
		{
			buffer[tc] &= ~( 1 << ( y & 7 ));
			break;
		}
        case INVERSE:
		{
			buffer[tc] ^= ( 1 << ( y & 7 ));
			break;
		}
      }
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  отчиска буфера кадра ( но не очищает сам дисплей )
	* @param	( параметры ):
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_clearBuffer( void )
{
	memset( buffer, 0x00, ( bufferWidth * ( bufferHeight / 8 ))  ); 
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  вывод картинки ( из массива ) на экран ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	параметры:
									//Param1: x offset 0-192
									//Param2: y offset 0-64
									//Param3: width 0-192
									//Param4 height 0-64
									//Param5 массив с картинкой
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_bitmap(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t* data, uint8_t colour) 
{
	int16_t byteWidth = (w + 7) / 8; 									// Bitmap scanline pad = whole byte
	uint8_t byte = 0;

	for(int16_t j=0; j<h; j++, y++)
	{
		for(int16_t i=0; i<w; i++)
		{
			if(i & 7)
			{
			   byte <<= 1;
			}
			else
			{
			   byte = (*(const unsigned char *)(&data[j * byteWidth + i / 8]));
			}
			if(byte & 0x80) UC1609C_drawPixel(x+i, y, colour);
		}
	}
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  отчиска самого дисплея ( буфер кадра не очищается )
	* @param	( параметры ):	
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_clearDisplay( void )
{

	UC1609C_fillScreen(0x00);
}			
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  прорисовка одного символа
	* @param	( параметры ):	1- координата x
								2- координата y
								3- сам символ
								4- указываем шрифт символа
								5- множитель размера символа
								6- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
static void UC1609C_DrawChar(int16_t x, int16_t y, unsigned char ch, FontDef_t* Font, uint8_t multiplier, uint8_t color)
{
	
	uint16_t i, j;
	
	uint16_t b;
	
	int16_t X = x, Y = y;
	
	int16_t xx, yy;
	
	if( multiplier < 1 ){
		multiplier = 1;
	}
	
	/* Check available space in LCD */
	if ( UC1609C_WIDTH >= ( x + Font->FontWidth) || UC1609C_HEIGHT >= ( y + Font->FontHeight)){
	
		/* Go through font */
		for (i = 0; i < Font->FontHeight; i++) {
			
			if( ch < 127 ){			
				b = Font->data[(ch - 32) * Font->FontHeight + i];
			}
			
			else if( (uint8_t) ch > 191 ){
				// +96 это так как латинские символы и знаки в шрифтах занимают 96 позиций
				// и если в шрифте который содержит сперва латиницу и спец символы и потом 
				// только кирилицу то нужно добавлять 95 если шрифт 
				// содержит только кирилицу то +96 не нужно
				b = Font->data[((ch - 192) + 96) * Font->FontHeight + i];
			}
			
			else if( (uint8_t) ch == 168 ){	// 168 символ по ASCII - Ё
				// 160 эллемент ( символ Ё ) 
				b = Font->data[( 160 ) * Font->FontHeight + i];
			}
			
			else if( (uint8_t) ch == 184 ){	// 184 символ по ASCII - ё
				// 161 эллемент  ( символ ё ) 
				b = Font->data[( 161 ) * Font->FontHeight + i];
			}
			//-------------------------------------------------------------------------------
			
			
			for (j = 0; j < Font->FontWidth; j++) {
				
				if ((b << j) & 0x8000) {
					
					for (yy = 0; yy < multiplier; yy++){
						for (xx = 0; xx < multiplier; xx++){
								UC1609C_drawPixel(X+xx, Y+yy, color);
						}
					}
					
				}
				// если фон очищать то оставляем если чтоб фон оставался старый то коментируем эту часть --------------------------------------------
				//-----------------------------------------------------------------------------------------------------------------------------------
				else{
					
					for (yy = 0; yy < multiplier; yy++){
						for (xx = 0; xx < multiplier; xx++){
								UC1609C_drawPixel(X+xx, Y+yy, !color);
						}
					}
				}
				//-----------------------------------------------------------------------------------------------------------------------------------
				//-----------------------------------------------------------------------------------------------------------------------------------
				
				X = X + multiplier;
			}
			
			X = x;
			Y = Y + multiplier;
		}
		
	}
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  вывод на экран строки ( латиница и кириллица ) ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	1- координата x
								2- координата y
								3- сама строка
								4- указываем шрифт символа
								5- множитель размера символа
								6- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_Print(int16_t x, int16_t y, char* str, FontDef_t* Font, uint8_t multiplier, uint8_t color) {
	
	if( multiplier < 1 ){
		multiplier = 1;
	}
	
	unsigned char buff_char;
	
	uint16_t len = strlen(str);
	
	while (len--) {
		
		//---------------------------------------------------------------------
		// проверка на кириллицу UTF-8, если латиница то пропускаем if
		// Расширенные символы ASCII Win-1251 кириллица (код символа 128-255)
		// проверяем первый байт из двух ( так как UTF-8 ето два байта )
		// если он больше либо равен 0xC0 ( первый байт в кириллеце будет равен 0xD0 либо 0xD1 именно в алфавите )
		if ( (uint8_t)*str >= 0xC0 ){	// код 0xC0 соответствует символу кириллица 'A' по ASCII Win-1251
			
			// проверяем какой именно байт первый 0xD0 либо 0xD1
			switch ((uint8_t)*str) {
				case 0xD0: {
					// увеличиваем массив так как нам нужен второй байт
					str++;
					// проверяем второй байт там сам символ
					if ((uint8_t)*str == 0x81) { buff_char = 0xA8; break; }		// байт символа Ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					if ((uint8_t)*str >= 0x90 && (uint8_t)*str <= 0xBF){ buff_char = (*str) + 0x30; }	// байт символов А...Я а...п  делаем здвиг на +48
					break;
				}
				case 0xD1: {
					// увеличиваем массив так как нам нужен второй байт
					str++;
					// проверяем второй байт там сам символ
					if ((uint8_t)*str == 0x91) { buff_char = 0xB8; break; }		// байт символа ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
					if ((uint8_t)*str >= 0x80 && (uint8_t)*str <= 0x8F){ buff_char = (*str) + 0x70; }	// байт символов п...я	елаем здвиг на +112
					break;
				}
			}
			// уменьшаем еще переменную так как израсходывали 2 байта для кириллицы
			len--;
			
			UC1609C_DrawChar( x, y, buff_char, Font, multiplier, color);
		}
		//---------------------------------------------------------------------
		else{			
			UC1609C_DrawChar( x, y, *str, Font, multiplier, color);
		}
		
		x = x + (Font->FontWidth * multiplier);
		/* Increase string pointer */
		str++;
	}
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  рисуем линию ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	1- координата x1
								2- координата y1
								3- координата x2
								4- координата y2
								5- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c) {
	
	int16_t dx, dy, sx, sy, err, e2, i, tmp; 
	
	/* Check for overflow */
	if (x0 >= UC1609C_WIDTH) {
		x0 = UC1609C_WIDTH - 1;
	}
	if (x1 >= UC1609C_WIDTH) {
		x1 = UC1609C_WIDTH - 1;
	}
	if (y0 >= UC1609C_HEIGHT) {
		y0 = UC1609C_HEIGHT - 1;
	}
	if (y1 >= UC1609C_HEIGHT) {
		y1 = UC1609C_HEIGHT - 1;
	}
	
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
	sx = (x0 < x1) ? 1 : -1; 
	sy = (y0 < y1) ? 1 : -1; 
	err = ((dx > dy) ? dx : -dy) / 2; 

	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			UC1609C_drawPixel(x0, i, c);
		}
		
		/* Return from function */
		return;
	}
	
	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}
		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}
		
		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			UC1609C_drawPixel(i, y0, c);
		}
		
		/* Return from function */
		return;
	}
	
	while (1) {
		UC1609C_drawPixel(x0, y0, c);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err; 
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		} 
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		} 
	}
}
//----------------------------------------------------------------------------------



/*
	******************************************************************************
	* @brief	 ( описание ):  рисуем прямоугольник пустотелый ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	1- координата x1
								2- координата y1
								3- ширина
								4- высота
								5- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_DrawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t c) {
	/* Check input parameters */
	if (
		x >= UC1609C_WIDTH ||
		y >= UC1609C_HEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= UC1609C_WIDTH) {
		w = UC1609C_WIDTH - x;
	}
	if ((y + h) >= UC1609C_HEIGHT) {
		h = UC1609C_HEIGHT - y;
	}
	
	/* Draw 4 lines */
	UC1609C_DrawLine(x, y, x + w, y, c);         /* Top line */
	UC1609C_DrawLine(x, y + h, x + w, y + h, c); /* Bottom line */
	UC1609C_DrawLine(x, y, x, y + h, c);         /* Left line */
	UC1609C_DrawLine(x + w, y, x + w, y + h, c); /* Right line */
}
//----------------------------------------------------------------------------------
/*
	******************************************************************************
	* @brief	 ( описание ):  рисуем прямоугольник закрашеный ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	1- координата x1
								2- координата y1
								3- ширина
								4- высота
								5- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_DrawFilledRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t c) {
	
	uint8_t i;
	
	/* Check input parameters */
	if (
		x >= UC1609C_WIDTH ||
		y >= UC1609C_HEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= UC1609C_WIDTH) {
		w = UC1609C_WIDTH - x;
	}
	if ((y + h) >= UC1609C_HEIGHT) {
		h = UC1609C_HEIGHT - y;
	}
	
	/* Draw lines */
	for (i = 0; i <= h; i++) {
		/* Draw lines */
		UC1609C_DrawLine(x, y + i, x + w, y + i, c);
	}
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  рисуем триугольник пустотелый ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	1- координата x1
								2- координата y1
								3- координата x2
								4- координата y2
								5- координата x3
								6- координата y3
								7- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_DrawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint8_t color) {
	/* Draw lines */
	UC1609C_DrawLine(x1, y1, x2, y2, color);
	UC1609C_DrawLine(x2, y2, x3, y3, color);
	UC1609C_DrawLine(x3, y3, x1, y1, color);
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  рисуем триугольник заполненый ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	1- координата x1
								2- координата y1
								3- координата x2
								4- координата y2
								5- координата x3
								6- координата y3
								7- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_DrawFilledTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint8_t color) {
	
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;
	
	deltax = ABS(x2 - x1);
	deltay = ABS(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay){
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	} else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		UC1609C_DrawLine(x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  рисуем круг пустотелый ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	1- координата x1
								2- координата y1
								3- радиус
								4- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t c) {
	
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    UC1609C_drawPixel(x0, y0 + r, c);
    UC1609C_drawPixel(x0, y0 - r, c);
    UC1609C_drawPixel(x0 + r, y0, c);
    UC1609C_drawPixel(x0 - r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        UC1609C_drawPixel(x0 + x, y0 + y, c);
        UC1609C_drawPixel(x0 - x, y0 + y, c);
        UC1609C_drawPixel(x0 + x, y0 - y, c);
        UC1609C_drawPixel(x0 - x, y0 - y, c);

        UC1609C_drawPixel(x0 + y, y0 + x, c);
        UC1609C_drawPixel(x0 - y, y0 + x, c);
        UC1609C_drawPixel(x0 + y, y0 - x, c);
        UC1609C_drawPixel(x0 - y, y0 - x, c);
    }
}
//----------------------------------------------------------------------------------


/*
	******************************************************************************
	* @brief	 ( описание ):  рисуем круг заполненый ( не забываем вызывать UC1609C_update(); )
	* @param	( параметры ):	1- координата x1
								2- координата y1
								3- радиус
								4- вид пикселя ( FOREGROUND or BACKGROUND  or INVERSE )
	* @return  ( возвращает ):	
	******************************************************************************
*/
void UC1609C_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint8_t c) {
	
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    UC1609C_drawPixel(x0, y0 + r, c);
    UC1609C_drawPixel(x0, y0 - r, c);
    UC1609C_drawPixel(x0 + r, y0, c);
    UC1609C_drawPixel(x0 - r, y0, c);
    UC1609C_DrawLine(x0 - r, y0, x0 + r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        UC1609C_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
        UC1609C_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);

        UC1609C_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
        UC1609C_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
    }
}
//----------------------------------------------------------------------------------



/************************ (C) COPYRIGHT GKP *****END OF FILE****/
