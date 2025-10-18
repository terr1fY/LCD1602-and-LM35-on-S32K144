/**
 ******************************************************************************
 * @file      main.c
 * @brief     Project to read temperature from an LM35 sensor using ADC
 * and display the result on a 1602 I2C LCD.
 * @author    [Vo Minh Luong]
 * @date      Oct 17, 2025
 ******************************************************************************
 */

/*============================================================================*/
/* Includes                                   */
/*============================================================================*/
#include "S32K144.h"
#include "peripherals_lpi2c_config_1.h"
#include "clock_config.h"	// clockMan1_InitConfig0
#include "pin_mux.h"		// NUM_OF_CONFIGURED_PINS0
#include "lpi2c_driver.h"   // LPI2C low-level driver
#include "adc_driver.h"     // ADC driver
#include "osif.h"           // OS Interface for OSIF_TimeDelay()
#include "stdio.h"          // Standard I/O for sprintf()

/*============================================================================*/
/* Defines                                   */
/*============================================================================*/

// HD44780 LCD Controller Commands
#define LCD_CLEAR_DISPLAY   0x01
#define LCD_RETURN_HOME     0x02
#define LCD_ENTRY_MODE_SET  0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_FUNCTION_SET    0x20
#define LCD_SET_DDRAM_ADDR  0x80

// Application-specific constants for better readability
#define ADC_VREF_MV         5000.0f  // V-Reference for ADC in millivolts (5V)
#define ADC_MAX_VALUE       4095.0f  // Max value for a 12-bit ADC

/*============================================================================*/
/* Global Variables                                */
/*============================================================================*/

// State structure for the LPI2C0 master driver instance
lpi2c_master_state_t g_lpi2c0MasterState;

// Global variables to hold sensor data
int    g_temperature_celsius; // Use integer for precision
uint16_t g_adc_result;

/*============================================================================*/
/* Private Function Prototypes                         */
/*============================================================================*/

void WDOG_disable(void);
void LCD_SendByte(uint8_t data, uint8_t rs_bit);
void LCD_SendCommand(uint8_t command);
void LCD_SendData(uint8_t data);
void LCD_SendString(char *str);
void LCD_Init(void);

/*============================================================================*/
/* Main Function                                  */
/*============================================================================*/

int main(void)
{
    // Buffer to hold the formatted temperature string
    char temp_string[16];

    /*--------------------------------------------------*/
    /* 1. One-Time System Initialization       */
    /*--------------------------------------------------*/
    WDOG_disable();
    CLOCK_DRV_Init(&clockMan1_InitConfig0);
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    // Initialize LPI2C0 in master mode
    LPI2C_DRV_MasterInit(INST_LPI2C0, &lpi2c0_MasterConfig0, &g_lpi2c0MasterState);

    // Initialize the LCD display
    LCD_Init();

    // --- ADC Initialization Block ---
    adc_chan_config_t adc_channel_config;
    ADC_DRV_InitChanStruct(&adc_channel_config);
    adc_channel_config.channel = ADC_INPUTCHAN_EXT12; // Corresponds to your configured ADC pin

    /*--------------------------------------------------*/
    /* 2. Display Static Content on LCD           */
    /*--------------------------------------------------*/
    // This is done once to prevent screen flickering inside the main loop
    LCD_SendCommand(LCD_CLEAR_DISPLAY);
    OSIF_TimeDelay(2);
    LCD_SendCommand(0x80); // Move cursor to the beginning of the first line
    LCD_SendString("Temperature:");
    
    /*--------------------------------------------------*/
    /* 3. Main Loop                     */
    /*--------------------------------------------------*/
    while (1)
    {     
        // --- Read Temperature from Sensor --
        ADC_DRV_ConfigChan(0, 0, &adc_channel_config);
        ADC_DRV_WaitConvDone(0);
        ADC_DRV_GetChanResult(0, 0, &g_adc_result);

        // --- Calculate Temperature ---
        // Formula: Temp (°C) = ( (ADC_Result / ADC_Max_Value) * V_Ref (mV) ) / 10 (mV/°C)
        g_temperature_celsius = (((float)g_adc_result / ADC_MAX_VALUE) * ADC_VREF_MV) / 10.0f;

        // --- Format and Display Temperature on LCD ---
        // Convert the integer value to a formatted string (e.g., "27 C")
        sprintf(temp_string, "%d C ", g_temperature_celsius);

        // Move cursor to the beginning of the second line
        LCD_SendCommand(0xC0);

        // Send the formatted string to the LCD
        LCD_SendString(temp_string);

        // Wait for 1 second before the next measurement
        OSIF_TimeDelay(1000);
    }

    return 0;
}

/*============================================================================*/
/* Private Function Implementations                       */
/*============================================================================*/

/**
 * @brief Disables the Watchdog timer.
 */
void WDOG_disable(void)
{
    WDOG->CNT = 0xD928C520;
    WDOG->TOVAL = 0x0000FFFF;
    WDOG->CS = 0x00002100;
}

/**
 * @brief Sends a single byte to the LCD via I2C.
 * @details This function splits the byte into two 4-bit nibbles and sends them sequentially,
 * toggling the Enable (EN) pin for each nibble.
 * @param data The 8-bit data byte to send.
 * @param rs_bit Register Select bit (0 for command, 1 for data).
 */
void LCD_SendByte(uint8_t data, uint8_t rs_bit)
{
    status_t status;
    uint8_t high_nibble = data & 0xF0;
    uint8_t low_nibble = (data << 4) & 0xF0;
    uint8_t i2c_payload[4];

    // Payload for the high nibble (EN pulse)
    i2c_payload[0] = (high_nibble | rs_bit | 0x08 | 0x04); // Data | RS | Backlight | EN=1
    i2c_payload[1] = (high_nibble | rs_bit | 0x08);        // Data | RS | Backlight | EN=0

    // Payload for the low nibble (EN pulse)
    i2c_payload[2] = (low_nibble | rs_bit | 0x08 | 0x04);  // Data | RS | Backlight | EN=1
    i2c_payload[3] = (low_nibble | rs_bit | 0x08);         // Data | RS | Backlight | EN=0

    // Send the entire 4-byte sequence in a single blocking I2C transaction
    status = LPI2C_DRV_MasterSendDataBlocking(INST_LPI2C0, i2c_payload, 4, true, 100);
    (void)status; // Suppress unused variable warning
}

/**
 * @brief Sends a command to the LCD.
 * @param command The command byte to send.
 */
void LCD_SendCommand(uint8_t command)
{
    LCD_SendByte(command, 0); // RS = 0 for commands
}

/**
 * @brief Sends a data character to the LCD.
 * @param data The character byte to send.
 */
void LCD_SendData(uint8_t data)
{
    LCD_SendByte(data, 1); // RS = 1 for data
}

/**
 * @brief Sends a null-terminated string to the LCD.
 * @param str Pointer to the string.
 */
void LCD_SendString(char *str)
{
    while (*str)
    {
        LCD_SendData(*str++);
    }
}

/**
 * @brief Initializes the LCD into 4-bit communication mode.
 * @details Sends the required sequence of commands to configure the LCD.
 */
void LCD_Init(void)
{
    // Wait for LCD to power up
    OSIF_TimeDelay(50);

    // --- Special initialization sequence for 4-bit mode ---
    LCD_SendByte(0x30, 0);
    OSIF_TimeDelay(5);
    LCD_SendByte(0x30, 0);
    OSIF_TimeDelay(1);
    LCD_SendByte(0x30, 0);
    OSIF_TimeDelay(1);
    LCD_SendByte(0x20, 0); // Set to 4-bit interface
    OSIF_TimeDelay(1);

    // --- Standard configuration ---
    LCD_SendCommand(LCD_FUNCTION_SET | 0x08);    // 4-bit mode, 2 lines, 5x8 font
    LCD_SendCommand(LCD_DISPLAY_CONTROL | 0x04); // Display on, cursor off, blink off
    LCD_SendCommand(LCD_CLEAR_DISPLAY);          // Clear display
    OSIF_TimeDelay(2);                           // This command takes longer to execute
    LCD_SendCommand(LCD_ENTRY_MODE_SET | 0x02);  // Increment cursor, no display shift
    LCD_SendCommand(LCD_RETURN_HOME);            // Return cursor to home position
}

