/*********************************************************************************************************
*
* File                : touch_panel.c
* Hardware Environment: 
* Build Environment   : RealView MDK-ARM  Version: 4.20
* Version             : V1.0
* By                  : 
*
*                                  (c) Copyright 2005-2011, WaveShare
*                                       http://www.waveshare.net
*                                          All Rights Reserved
*
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "touch_panel.h"
#include "GUI.h"



extern SPI_HandleTypeDef hspi2;
void MX_SPI2_Init(void);

/* Private variables ---------------------------------------------------------*/
//Matrix matrix ;
Matrix  matrix = {
		0x00000104,
		0xFFFE2816,
		0x070E37A4 ,
		0x0001582E ,
		0xFFFFF98E ,
		0xFFCD2856 ,
		0x00052E46
};
Coordinate  display ;


Coordinate ScreenSample[3];

Coordinate DisplaySample[3] = {
                                {30, 45},
                                {220, 45},
                                {160,210}
                              };

/* Private define ------------------------------------------------------------*/
#define THRESHOLD 2
#define TP_hspi hspi2



/*******************************************************************************
* Function Name  : DelayUS
* Description    : 
* Input          : - cnt:
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void DelayUS(uint32_t cnt)
{
  uint32_t i;
  i = cnt * 4;
  while(i--);
}


/*******************************************************************************
* Function Name  : WR_CMD
* Description    : 
* Input          : - cmd: 
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
static void WR_CMD (uint8_t cmd)  
{ 
  HAL_SPI_Transmit(&TP_hspi,&cmd,1,1000);
} 



/*******************************************************************************
* Function Name  : RD_AD
* Description    : 
* Input          : None
* Output         : None
* Return         : 
* Attention      : None
*******************************************************************************/
static int RD_AD(void)  
{ 
  uint8_t buf[2];
  int value;
  HAL_SPI_Receive(&TP_hspi,buf,2,1000);
  value = (uint16_t)((buf[0] << 8) + buf[1]) >> 3;
  return value;
} 


/*******************************************************************************
* Function Name  : Read_X
* Description    : Read ADS7843 ADC X 
* Input          : None
* Output         : None
* Return         : 
* Attention      : None
*******************************************************************************/
int Read_X(void)  
{  
  int i; 
  TP_CS(0); 
  DelayUS(1); 
  WR_CMD(CHX); 
  DelayUS(1); 
  i=RD_AD(); 
  TP_CS(1); 
  return i;    
} 

/*******************************************************************************
* Function Name  : Read_Y
* Description    : Read ADS7843 ADC Y
* Input          : None
* Output         : None
* Return         : 
* Attention      : None
*******************************************************************************/
int Read_Y(void)  
{  
  int i; 
  TP_CS(0); 
  DelayUS(1); 
  WR_CMD(CHY); 
  DelayUS(1); 
  i=RD_AD(); 
  TP_CS(1); 
  return i;     
} 


/*******************************************************************************
* Function Name  : TP_GetAdXY
* Description    : Read ADS7843
* Input          : None
* Output         : None
* Return         : 
* Attention      : None
*******************************************************************************/
void TP_GetAdXY(int *x,int *y)  
{ 
  int adx,ady; 
  adx=Read_X(); 
  DelayUS(1); 
  ady=Read_Y(); 
  *x=adx; 
  *y=ady; 
} 



/*******************************************************************************
* Function Name  : DrawCross
* Description    : 
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void DrawCross(uint16_t Xpos,uint16_t Ypos)
{

	writeFastHLine(Xpos-13, Ypos, 10, 0xffff);
	writeFastHLine(Xpos+4, Ypos, 10, 0xffff);
	writeFastVLine(Xpos, Ypos-13, 10, 0xffff);
	writeFastVLine(Xpos, Ypos+4, 10, 0xffff);

}  
  
/*******************************************************************************
* Function Name  : Read_Ads7846
* Description    : Get TouchPanel X Y
* Input          : None
* Output         : None
* Return         : Coordinate *
* Attention      : None
*******************************************************************************/
Coordinate *Read_Ads7846(void)
{
  static Coordinate  screen;
  int m0,m1,m2,TP_X[1],TP_Y[1],temp[3];
  uint8_t count=0;
  int buffer[2][9]={{0},{0}};
  
  do
  {       
    TP_GetAdXY(TP_X,TP_Y);  
    buffer[0][count]=TP_X[0];  
    buffer[1][count]=TP_Y[0];
    count++;  
  }
  while(!TP_INT_IN&& count<9);  /* TP_INT_IN  */
  if(count==9)   /* Average X Y  */ 
  {
    /* Average X  */
    temp[0]=(buffer[0][0]+buffer[0][1]+buffer[0][2])/3;
    temp[1]=(buffer[0][3]+buffer[0][4]+buffer[0][5])/3;
    temp[2]=(buffer[0][6]+buffer[0][7]+buffer[0][8])/3;
   
    m0=temp[0]-temp[1];
    m1=temp[1]-temp[2];
    m2=temp[2]-temp[0];
   
    m0=m0>0?m0:(-m0);
    m1=m1>0?m1:(-m1);
    m2=m2>0?m2:(-m2);
   
    if( m0>THRESHOLD  &&  m1>THRESHOLD  &&  m2>THRESHOLD ) return 0;
   
    if(m0<m1)
    {
      if(m2<m0) 
        screen.x=(temp[0]+temp[2])/2;
      else 
        screen.x=(temp[0]+temp[1])/2;  
    }
    else if(m2<m1) 
      screen.x=(temp[0]+temp[2])/2;
    else 
      screen.x=(temp[1]+temp[2])/2;
   
    /* Average Y  */
    temp[0]=(buffer[1][0]+buffer[1][1]+buffer[1][2])/3;
    temp[1]=(buffer[1][3]+buffer[1][4]+buffer[1][5])/3;
    temp[2]=(buffer[1][6]+buffer[1][7]+buffer[1][8])/3;
    m0=temp[0]-temp[1];
    m1=temp[1]-temp[2];
    m2=temp[2]-temp[0];
    m0=m0>0?m0:(-m0);
    m1=m1>0?m1:(-m1);
    m2=m2>0?m2:(-m2);
    if(m0>THRESHOLD&&m1>THRESHOLD&&m2>THRESHOLD) return 0;
   
    if(m0<m1)
    {
      if(m2<m0) 
        screen.y=(temp[0]+temp[2])/2;
      else 
        screen.y=(temp[0]+temp[1])/2;  
      }
    else if(m2<m1) 
       screen.y=(temp[0]+temp[2])/2;
    else
       screen.y=(temp[1]+temp[2])/2;
   
    screen.x = screen.x >> 2;
    screen.y = screen.y >> 2;
    return &screen;
  }
  return 0; 
}
   

/*******************************************************************************
* Function Name  : setCalibrationMatrix
* Description    : Calculate K A B C D E F
* Input          : None
* Output         : None
* Return         : 
* Attention      : None
*******************************************************************************/
FunctionalState setCalibrationMatrix( Coordinate * displayPtr,
                          Coordinate * screenPtr,
                          Matrix * matrixPtr)
{

  FunctionalState retTHRESHOLD = ENABLE ;
  /* K=(X0-X2) (Y1-Y2)-(X1-X2) (Y0-Y2) */
  matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) - 
                       ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
  if( matrixPtr->Divider == 0 )
  {
    retTHRESHOLD = DISABLE;
  }
  else
  {
    /* A=((XD0-XD2) (Y1-Y2)-(XD1-XD2) (Y0-Y2))/K  */
    matrixPtr->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) - 
                    ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
  /* B=((X0-X2) (XD1-XD2)-(XD0-XD2) (X1-X2))/K  */
    matrixPtr->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) - 
                    ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x)) ;
    /* C=(Y0(X2XD1-X1XD2)+Y1(X0XD2-X2XD0)+Y2(X1XD0-X0XD1))/K */
    matrixPtr->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;
    /* D=((YD0-YD2) (Y1-Y2)-(YD1-YD2) (Y0-Y2))/K  */
    matrixPtr->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) - 
                    ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;
    /* E=((X0-X2) (YD1-YD2)-(YD0-YD2) (X1-X2))/K  */
    matrixPtr->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) - 
                    ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;
    /* F=(Y0(X2YD1-X1YD2)+Y1(X0YD2-X2YD0)+Y2(X1YD0-X0YD1))/K */
    matrixPtr->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y ;
  }
  return( retTHRESHOLD ) ;
}

/*******************************************************************************
* Function Name  : getDisplayPoint
* Description    : Touch panel X Y to display X Y
* Input          : None
* Output         : None
* Return         : 
* Attention      : None
*******************************************************************************/
FunctionalState getDisplayPoint(Coordinate * displayPtr,
                     Coordinate * screenPtr,
                     Matrix * matrixPtr )
{
  FunctionalState retTHRESHOLD =ENABLE ;
  /*
  An=168
  */
  if( matrixPtr->Divider != 0 )
  {
    /* XD = AX+BY+C */        
    displayPtr->x = ( (matrixPtr->An * screenPtr->x) + 
                      (matrixPtr->Bn * screenPtr->y) + 
                       matrixPtr->Cn 
                    ) / matrixPtr->Divider ;
    /* YD = DX+EY+F */        
    displayPtr->y = ( (matrixPtr->Dn * screenPtr->x) + 
                      (matrixPtr->En * screenPtr->y) + 
                       matrixPtr->Fn 
                    ) / matrixPtr->Divider ;
  }
  else
  {
    retTHRESHOLD = DISABLE;
  }
  return(retTHRESHOLD);
} 

//***********************************************************************************************
#if 0
static void print_data( int32_t data) {

	  lcdSetTextColor(0xffff, 0);
	  lcdSetTextFont(&Font24);

	  LCD_ClrScr(COLOR_565_BLACK);

	  my_htoa32(idx , data);

	  lcdSetCursor(20, 100);
	  lcdPrintf((char *) idx);
	  HAL_Delay(555);
	  while((HAL_GPIO_ReadPin(LCDTP_IRQ_GPIO_Port, LCDTP_IRQ_Pin)) == 1);
}
#endif
//***************************************************************************************************
/*******************************************************************************
* Function Name  : TouchPanel_Calibrate
* Description    : 
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void TouchPanel_Calibrate(void)
{
  uint8_t i;
  Coordinate * Ptr;
  uint8_t logic = 0; // condition to check timelapse
  uint8_t test;
  uint32_t tick1, tick2, tickcntr;


  TP_CS(0);
  HAL_Delay(10);
  TP_CS(1);
  HAL_Delay(10);



  for(i=0;i<3;i++)
  {
	  lcdSetTextFont(&Font12);
	  LCD_ClrScr(COLOR_565_BLACK);
	  lcdSetTextColor(COLOR_565_WHITE, COLOR_565_BLACK );
	  lcdSetCursor(5,5);
	  lcdPrintf("        Touch crosshair to calibrate");
	  lcdSetTextFont(&Font24);
	  lcdSetCursor(0,95);
	  lcdPrintf("   Waveshare LCD");

	  lcdSetCursor(0,140);
	  lcdPrintf("  ILI9341 VERSION");
	  HAL_Delay(20);
	  DrawCross(DisplaySample[i].x,DisplaySample[i].y);
	  test = 0;
	  tickcntr = 0;
	  tick1 = HAL_GetTick();

	  while(test == 0){
		  if(HAL_GPIO_ReadPin(LCDTP_IRQ_GPIO_Port, LCDTP_IRQ_Pin) == 0) { // do calibrate
			  logic =1;
			  test = 1;
		  }
		  if(logic == 0) {
			  while((tick2 = HAL_GetTick()) == tick1){;}
			  tick1 = tick2;
			  tickcntr++;
			  if(tickcntr > 600) { return;} // wait time is over
		  }
	  }
    logic = 1;

    HAL_Delay(2000); // debounce tactic
    do
    {
      Ptr=Read_Ads7846();
    }
    while( Ptr == (void*)0 );
    ScreenSample[i].x= Ptr->x; ScreenSample[i].y= Ptr->y;
    LCD_ClrScr(COLOR_565_BLACK);

    HAL_Delay(2000); // debounce tactic
  }
  setCalibrationMatrix( &DisplaySample[0],&ScreenSample[0],&matrix );
  LCD_ClrScr(COLOR_565_BLACK);


//===================================================================================================
//===================================================================================================

#if 0
  print_data(matrix.An)  ;
  print_data(matrix.Bn)  ;
  print_data(matrix.Cn)  ;
  print_data(matrix.Dn)  ;
  print_data(matrix.En)  ;
  print_data(matrix.Fn)  ;
  print_data(matrix.Divider)  ;
  LCD_ClrScr(COLOR_565_BLACK);
#endif

//===================================================================================================
//===================================================================================================


} 
//============================================================================================================================
static void init_paint(void) {
	LCD_ClrScr(COLOR_565_BLACK);
	lcdSetTextFont(&Font12);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);


	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_FillRect(5, 200, 30, 30);
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(40, 200, 30, 30);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	BSP_LCD_FillRect(75, 200, 30, 30);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(110, 200, 30, 30);
	BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
	BSP_LCD_FillRect(145, 200, 30, 30);
	BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
	BSP_LCD_FillRect(180, 200, 30, 30);
	BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
	BSP_LCD_FillRect(215, 200, 30, 30);
	BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
	BSP_LCD_FillRect(250, 200, 30, 30);

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);


	lcdSetTextFont(&Font24);
	lcdSetCursor(290, 205);
	lcdPrintf("C");

	BSP_LCD_DrawHLine(  0, 196, 320);

	BSP_LCD_DrawVLine(  1, 198,  35);
	BSP_LCD_DrawVLine( 37, 198,  35);
	BSP_LCD_DrawVLine( 72, 198,  35);
	BSP_LCD_DrawVLine(107, 198,  35);
	BSP_LCD_DrawVLine(142, 198,  35);

	BSP_LCD_DrawVLine(177, 198,  35);
	BSP_LCD_DrawVLine(212, 198,  35);
	BSP_LCD_DrawVLine(247, 198,  35);
	BSP_LCD_DrawVLine(282, 198,  35);
	BSP_LCD_DrawVLine(317, 198,  35);

	BSP_LCD_DrawHLine(  1, 232, 320);

}
void paint_proc(void) {

    init_paint();

    lcdSetTextFont(&Font12);

	lcdSetCursor(5,5);
	lcdPrintf( "Touch Panel Paint" );
	lcdSetCursor(5,20);
	lcdPrintf( "Example");


	while (1)
	{

		getDisplayPoint(&display, Read_Ads7846(), &matrix );
		if(((display.y < 190) && (display.y >= 2)))
		{
			if((display.x >= 318) || (display.x < 2))
			{}
			else
			{
				BSP_LCD_FillCircle(display.x, display.y, 2);
			}
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 180) && (display.x <= 210))
		{
			BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 215) && (display.x <= 245))
		{
			BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 250) && (display.x <= 280))
		{
			BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 5) && (display.x <= 35))
		{
			BSP_LCD_SetTextColor(LCD_COLOR_RED);
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 40) && (display.x <= 70))
		{
			BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 75) && (display.x <= 105))
		{
			BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 110) && (display.x <= 140))
		{
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 145) && (display.x <= 175))
		{
			BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
		}
		else if ((display.y <= 230) && (display.y >= 190) && (display.x >= 285) && (display.x <= 315))
		{
			u16 bckp = BSP_LCD_GetTextColor();
            init_paint();
            BSP_LCD_SetTextColor(bckp);
            while((HAL_GPIO_ReadPin(LCDTP_IRQ_GPIO_Port, LCDTP_IRQ_Pin)) == 0);
            HAL_Delay(50);
		}

	}
}
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
