/**
  *********************************************************************************************************
	*@file    : main.c
	*@author  : S Varun and Tarran Kumar
	*@brief   : Main program for implementing a "Basic Phone Project" on STM32f407 Discovery Kit using 
	            GSM-A6 Module.
  **********************************************************************************************************
*/

#include "stm32f4xx_hal.h"                     //STM32F4xx MUC Header File
#include "STM32F407_KeypadDriver.h"            //Keypad Header File
#include "GSM_A6_Driver_STM32F407.h"           //GSM Header File
#include "STM32F407_I2C_LCD16x02_Driver.h"     //LCD Header File


/*Incoming SMS are stored in "Incoming_SMS_Message" , which is declared in GSM driver*/
extern char Incoming_SMS_Message[100];

/*Function Prototype of Mobile Phone Specific functions*/
void Phone_Home_Display(void);
void Phone_Make_Call(void);
void Phone_Send_SMS(void);
void Phone_Receive_Call(void);
void Phone_Receive_SMS(void);
void phone_move_display(char * str);
void Store_Phone_Number(char First_KeyPress_Val);


/*character array for storing the phone number entered via keypad*/
char phone_num[10];


int main(void)
{
	
	/********************** All Devices Initialization - START *********************/
	
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
	
	/* Initialize  LCD */
	LCD_Init();
	LCD_Send_String_On_Line1("Welcome");
	LCD_Send_String_On_Line2("Initializing...");
	
	/* Initialize  Keypad */
	KEYPAD_Init();
	
	/* Initialize GSM A6 Module */
	GSM_Init();
	
	/********************** All Devices Initialization - END *********************/
  
	/*Display the start/home screen on LCD */
	start:
	
		Phone_Home_Display();
		
		char Key = KEYPAD_NOT_PRESSED;
		
		while(1)
		{
			/*Check for the  Keypress */
			Key = KEYPAD_Read();
			if(Key != KEYPAD_NOT_PRESSED)
			{
				if(Key == '1')         //if key = 1 , then make a call
				{
					Phone_Make_Call();  
				}
				else if(Key == '2')   //if key =2, then send sms
				{
					Phone_Send_SMS();
				}
				else if(Key=='3')
				{
					LCD_Clear_Then_Display("Enter the text: ");				
					char key = KEYPAD_NOT_PRESSED;
					char key1[1000];
					/*Check Key press continoulsy */
					while(key == KEYPAD_NOT_PRESSED)
					{
						key = KEYPAD_Read();
						HAL_Delay(100);
					}
					int i=0;
					key1[i++]=key;
					LCD_Clear_Then_Display(key1);			
					while(key != '*'&& i<=1000)
					{
					key = KEYPAD_Read();
					if(key != KEYPAD_NOT_PRESSED)
					{
						if(key=='#')
						{
						LCD_Send_Cmd(LCD_Shift_cur_pos_left );
						LCD_Send_Data(' ');
						LCD_Send_Cmd(LCD_Shift_cur_pos_left );
						i--;
						}
					else if(key=='*')
					{
						HAL_Delay(100);
						for(int j=0;j<15;j++)
						{
						LCD_Send_Cmd(LCD_MOVE_DISPLAY_RIGHT);
						HAL_Delay(500);
						}
						LCD_Clear_Then_Display(key1);
						HAL_Delay(1000);
						
						goto start;
						
					}
					else if(key=='0')
					{
					key1[i++]=' ';
					LCD_Send_Data(' ');
					HAL_Delay(100);
					}
						
					else {
					key1[i++]=key;
					LCD_Send_Data(key);
					HAL_Delay(100);
					}
					}
					}
					/* ASk user "Press *" in order to Send SMS */
					key = KEYPAD_NOT_PRESSED;
					
					while(key == KEYPAD_NOT_PRESSED)
					{
						key = KEYPAD_Read();
						HAL_Delay(100);
					}
					
					
				}
					
				else
				{
					LCD_Clear_Then_Display("Invalid Choice");
					HAL_Delay(2000);
					Phone_Home_Display();
				}
		}
		
		/*Check GSM for Incoming Calls */
		if(GSM_Compare_GSMData_With("RING"))
		{	

			Phone_Receive_Call();
		}
		
		/*Check GSM for Incoming SMS*/
		if(GSM_Compare_GSMData_With("MESSAGE"))
		{
			Phone_Receive_SMS();
		}
		
    HAL_Delay(100);

   }
	
}	

/********************************** Mobile Phone Specific APIs - Start *************************************/


/**
 * @brief  Displays the start/home screen on LCD
 * @param  none
 * @retval None
 */
void Phone_Home_Display(void)
{
	LCD_Clear_Then_Display("Press 1 to Call");
	LCD_Send_String_On_Line2("Press 2 to SMS");
}




/**
 * @brief  This functiond Handles the "Making a call" functionality
 * @param  none
 * @retval None
 */
void Phone_Make_Call(void)
{
	/*Pattern match for call receive & call end, these string pattern indicates whethere
	call is ended or received*/
	char call_received[5] ={'R','"',',','0'};
	char call_received_test2[8]= {'C','A','L','L','"',',','1'};
	char call_end[6]="ERROR";
	
	LCD_Clear_Then_Display("Enter Phone no:");
	LCD_Send_String_On_Line2("or Press B:Exit");
	
	char key = KEYPAD_NOT_PRESSED;
	
	/*Check Key press continoulsy */
	while(key == KEYPAD_NOT_PRESSED)
	{
		key = KEYPAD_Read();
		HAL_Delay(100);
	}
	
	/* Now Check which key is pressed */
	if(key == 'B')
	{
		/*If so, exit */
		GSM_Clear_RX_buffer();
		Phone_Home_Display();
		return;
	}
	else
	{
		/*Store Phone number */
		Store_Phone_Number(key);

		/* ASk user "Press C" in order to Call */
		key = KEYPAD_NOT_PRESSED;
		
		LCD_Send_String_On_Line2("Press C to call");
		
		while(key == KEYPAD_NOT_PRESSED)
		{
			key = KEYPAD_Read();
			HAL_Delay(100);
		}
		
		/* If C is pressed make a call else exit */
		if(key == 'C')
		{
			LCD_Clear_Then_Display("Calling......");
			LCD_Send_String_On_Line2(phone_num);
			
			/*Call GSM_Make_Call function with phone num as parameter*/
			GSM_Make_Call(phone_num);
			
			/* To display counting integer - time elapsed(seconnds)*/
			int i=0,num=0,j,k;
			int digit[4] ={0};
	    char character[5]= "0000";
			
			char key = KEYPAD_NOT_PRESSED;
			
			HAL_Delay(250);
			while(1)
			{
				/*Check whether call received, and call is in progress */
				if(GSM_Compare_GSMData_With(call_received) && GSM_Compare_GSMData_With(call_received_test2))
				{
					/*If yes, the display integer as time elapsed*/
					  i =num;
			      LCD_Clear_Then_Display(character);
					  j=3,k=0;
						while(i>0)
						{
							digit[j--] = i%10;
							i = i/10;
						}
						while(k<4)
						{
							character[k] = digit[k]+'0';
							k++;
						}
						
						num++;
					  HAL_Delay(600);
				}
				
				/*If at any momeny, "B" is pressed then cut the call*/	
			 key = KEYPAD_Read();
			 if(key != KEYPAD_NOT_PRESSED)
			 {
				 if(key == 'B')
				 {
					 GSM_HangUp_Call();
					 LCD_Clear_Then_Display("Call Ended!!");
				   GSM_Clear_RX_buffer();
				   HAL_Delay(1500);
				   Phone_Home_Display();
				   return;
				 }
					 
			 }
					
				/*Check if call endede, if yes display same and exit*/
				if(GSM_Compare_GSMData_With(call_end))
				{
					LCD_Clear_Then_Display("Call Ended!!");
					GSM_Clear_RX_buffer();
					HAL_Delay(1500);
					Phone_Home_Display();
					return;
				}
				HAL_Delay(100);
			}	
			
	 }
	 else
	 {
			LCD_Clear_Then_Display("Invalid input");
			Phone_Home_Display();
			return;
	  }
		
	}
	
}
/***************************************************************************************************/



/**
 * @brief  This function handles sending an sms
 * @param  none
 * @retval None
 */
void Phone_Send_SMS(void)
{
	LCD_Clear_Then_Display("Enter Phone no:");
	LCD_Send_String_On_Line2("or Press B:Exit");
	
	char key = KEYPAD_NOT_PRESSED;
	char key1[1000];
	/*Check Key press continoulsy */
	while(key == KEYPAD_NOT_PRESSED)
	{
		key = KEYPAD_Read();
		HAL_Delay(100);
	}
	
	
	/* Now Check which key is pressed */
	if(key == 'B')
	{
		/*If yes, the exit*/
		Phone_Home_Display();
		return;
	}
	else
	{
		/*Store Phone number */
		Store_Phone_Number(key);
		LCD_Clear_Then_Display("Enter the message: ");
		/* ASk user To enter message in order to Send SMS */
		HAL_Delay(10000);
		int i=0;
		key = KEYPAD_Read();
		LCD_Clear_Then_Display("");
		LCD_Send_Cmd(LCD_Shift_cur_pos_left );
		LCD_Send_Data(key);
		key1[i++]=key;
		while(key != '*'&& i<=1000)
		{
		key = KEYPAD_Read();
		if(key != KEYPAD_NOT_PRESSED)
		{
			if(key=='#')
			{
			LCD_Send_Cmd(LCD_Shift_cur_pos_left );
			LCD_Send_Data('_');
			LCD_Send_Cmd(LCD_Shift_cur_pos_left );
			i--;
			}
		else if(key=='*'){
		break;}
		else if(key=='0')
		{
		key1[i++]=' ';
		LCD_Send_Data(' ');
		HAL_Delay(100);
		}
			
		else {
		key1[i++]=key;
		LCD_Send_Data(key);
		HAL_Delay(100);
		}
		}
		}
		/* ASk user "Press *" in order to Send SMS */
		key = KEYPAD_NOT_PRESSED;
		
		LCD_Clear_Then_Display("Press * to Send");
		LCD_Send_String_On_Line2("Message :-)");
		
		while(key == KEYPAD_NOT_PRESSED)
		{
			key = KEYPAD_Read();
			HAL_Delay(100);
		}
	  
		/* If C is pressed Send SMS*/
		if(key == '*')
		{
			LCD_Clear_Then_Display("Sending SMS......");
			LCD_Send_String_On_Line2("Test Message");
			GSM_Send_SMS(key1,phone_num); //As of now hardcoded SMS is sent.
			HAL_Delay(1000);
			LCD_Clear_Then_Display("Message Sent");
			HAL_Delay(900);
			GSM_Clear_RX_buffer();
			Phone_Home_Display();
			return;
		}
	  else
		{
			LCD_Clear_Then_Display("Invalid input");
			Phone_Home_Display();
			return;
		}
	
   }
}
/***************************************************************************************************/




/**
 * @brief  This function handles the "Call receiving" Functionality
 * @param  none
 * @retval None
 */
void Phone_Receive_Call(void)
{
	/*Pattern match for whether call is received successfully, call ended*/
	char call_received[8] ="CONNECT";
	char call_end[6]="ERROR";
	
	LCD_Clear_Then_Display("Incoming Call...");
	LCD_Send_String_On_Line2("C:Recieve B:Hang");
	
	char key = KEYPAD_NOT_PRESSED;
	uint32_t *pClkCtrlReg =   (uint32_t*)0x40023830;
	uint32_t *pPortDModeReg = (uint32_t*)0x40020C00;
	uint32_t *pPortDOutReg =  (uint32_t*)0x40020C14;

	//1. enable the clock for GPOID peripheral in the AHB1ENR (SET the 3rd bit position)
	*pClkCtrlReg |= ( 1 << 3);
	*pPortDModeReg &= ~( 3 << 24);
	*pPortDModeReg |= ( 1 << 24);
		*pPortDModeReg &= ~( 3 << 26);
	*pPortDModeReg |= ( 1 << 26);
		*pPortDModeReg &= ~( 3 << 28);
	*pPortDModeReg |= ( 1 << 28);
	*pPortDModeReg |= ( 1 << 30);
	/*Check Key press continoulsy */
	while(key == KEYPAD_NOT_PRESSED)
	{			
				
		for(uint32_t i=0 ; i < 200000 ; i++ );
				//3.SET 12th bit of the output data register to make I/O pin-12 as HIGH
				*pPortDOutReg |= ( 1 << 12);
		for(uint32_t i=0 ; i < 200000 ; i++ );
				*pPortDOutReg |= ( 1 << 13);
		for(uint32_t i=0 ; i < 200000 ; i++ );
			*pPortDOutReg |= ( 1 << 14);
		for(uint32_t i=0 ; i < 200000 ; i++ );
		*pPortDOutReg |= ( 1 << 15);
				//introduce small human observable delay
				//This loop executes for 10K times
				for(uint32_t i=0 ; i < 200000 ; i++ );

				//Tun OFF the LED
				*pPortDOutReg &= ~( 1 << 12);
		for(uint32_t i=0 ; i < 200000 ; i++ );
				*pPortDOutReg &= ~( 1 << 13);
		for(uint32_t i=0 ; i < 200000 ; i++ );
			*pPortDOutReg &= ~( 1 << 14);
		for(uint32_t i=0 ; i < 200000 ; i++ );
		*pPortDOutReg &= ~( 1 << 15);

			if(GSM_Compare_GSMData_With(call_end))
				{
					LCD_Clear_Then_Display("Call Ended!!");
				  GSM_Clear_RX_buffer();
				  HAL_Delay(1500);
				  Phone_Home_Display();
				  return;
			  }		
		key = KEYPAD_Read();
		HAL_Delay(100);
	}
	
	while(1)
	{
		/* Now Check which key is pressed */
	  if(key == 'B')
	  {
			GSM_HangUp_Call();
		  GSM_Clear_RX_buffer();
		  Phone_Home_Display();
		  return;
 	  }
	
    if(key == 'C')
		{
			GSM_Receive_Call();
		  /* To display counting integer - time elapsed(seconnds)*/
		  int i=0,num=0,j,k;
		  int digit[4] ={0};
	    char character[5]= "0000";
		
		  char key = KEYPAD_NOT_PRESSED;
		  HAL_Delay(250);
		
		  while(1)
			{
				/*Check if call received or not*/
			  if(GSM_Compare_GSMData_With(call_received))
				{
					/*If yes, then display time elapsed */
				  i = num;
				  LCD_Clear_Then_Display(character);
				
				  j=3,k=0;
				  while(i>0)
					{
						digit[j--] = i%10;
					  i = i/10;
				  }
					
				  while(k<4)
				  {
						character[k] = digit[k]+'0';
					  k++;
					}
						
				  num++;
				  HAL_Delay(650);		
			  }
			
			  /*If at any momeny, "B" is pressed then cut the call*/			 
			  key = KEYPAD_Read();
			  if(key != KEYPAD_NOT_PRESSED)
				{
					if(key == 'B')
					{
						GSM_HangUp_Call();
					  LCD_Clear_Then_Display("Call Ended!!");
				    GSM_Clear_RX_buffer();
				    HAL_Delay(1500);
				    Phone_Home_Display();
				    return;
				  }
				 	 
			  }
			 
			  /*Check if call ended, if so display same and exit*/
			  if(GSM_Compare_GSMData_With(call_end))
				{
					LCD_Clear_Then_Display("Call Ended!!");
				  GSM_Clear_RX_buffer();
				  HAL_Delay(1500);
				  Phone_Home_Display();
				  return;
			  }		
				
        HAL_Delay(100);			
		
	    }
			
    }	
		
  }
	
}
/***************************************************************************************************/



/**
 * @brief  This function handles the "Receiving an Incoming SMS" Functionality
 * @param  none
 * @retval None
 */
void Phone_Receive_SMS(void)
{
	GSM_Receive_SMS();
	
	LCD_Clear_Then_Display("You have received 1 SMS"); 	
	LCD_Send_String_On_Line2("C:Read  B:Exit");
	
	char key = KEYPAD_NOT_PRESSED;
	
	/*Check Key press continoulsy */
	while(key == KEYPAD_NOT_PRESSED)
	{
		key = KEYPAD_Read();
		HAL_Delay(100);
	}
	
	/* Now Check which key is pressed */
	if(key == 'B')
	{
		GSM_Clear_RX_buffer();
		Phone_Home_Display();
		return;
	}
	else if(key == 'C')
	{
	 LCD_Display_Long_Message(Incoming_SMS_Message);
	 HAL_Delay(5000);
	 GSM_Clear_RX_buffer();	
	 Phone_Home_Display();	
	 return;
  }
	else
	{
		Phone_Receive_SMS();
	}
	
}
/***************************************************************************************************/


/**
 * @brief  Storing the phone number 
 * @param  none
 * @retval None
 */
void Store_Phone_Number(char First_KeyPress_Val)
{
	  /*Clear Phone_num[] array */
	  for(int i = 0;i<10;i++)
	     phone_num[i] = '\0';
	
		char key = First_KeyPress_Val;
	  int phone_num_count = 0;
	
		/* Store the 1st digit */
		phone_num[phone_num_count] = key;
	
		phone_num_count++;
		LCD_Clear_Then_Display(phone_num);
		
		/*Now store the rest 9 digits*/
		while(phone_num_count <10)
		{
			key = KEYPAD_Read();
			if(key != KEYPAD_NOT_PRESSED)
			{
			if(key=='#')
			{
			LCD_Send_Cmd(LCD_Shift_cur_pos_left );
			LCD_Send_Data('_');
			LCD_Send_Cmd(LCD_Shift_cur_pos_left );
			phone_num_count--;
			}
			else{
				phone_num[phone_num_count] = key;
				phone_num_count++;
				/*LCD_Clear_Then_Display(phone_num);*/
				LCD_Send_Data(key); //Display the key entered
				HAL_Delay(100);
			}
		}
			HAL_Delay(100);
		}
		
}
/***************************************MOVING DISPLAY*****************************/
/*void phone_move_display(char *str)
{
	LCD_Clear_Then_Display("");
	HAL_Delay(1000);
	LCD_Clear_Then_Display(str);
	HAL_Delay(100);
	for(int i=0;i<16;i++)
	{
		HAL_Delay(100);
	LCD_Send_Cmd(0x05);
		HAL_Delay(100);
	}
	LCD_Clear_Then_Display("");
	LCD_Send_String(str);
}*/


/*********************************** Mobile Phone Specific APIs - END ***************************************/

/* Only One "Sys tick handler" should be in entire project - so defined only in main.c */
void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}
