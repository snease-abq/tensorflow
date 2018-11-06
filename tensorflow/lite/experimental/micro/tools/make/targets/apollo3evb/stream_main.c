/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <stdint.h>
#include "am_mcu_apollo.h"              // Defines AM_CMSIS_REGS
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// The entry point for the application.
//
//*****************************************************************************
extern int main(int argc, char**argv);

void DebugLog(const char* s) { am_util_stdio_printf( "%s", s); }
void DebugLogInt32(int32_t i) { am_util_stdio_printf( "%d", i); }
void DebugLogUInt32(uint32_t i) { am_util_stdio_printf( "%d", i); }
void DebugLogHex(uint32_t i) { am_util_stdio_printf( "0x%8x", i); }
void DebugLogFloat(float i) { am_util_stdio_printf( "%f", i); }

//*****************************************************************************
// BUTTON0 pin configuration settings.
//*****************************************************************************
const am_hal_gpio_pincfg_t g_deepsleep_button0 =
{
    .uFuncSel = 3,
    .eIntDir = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .eGPInput = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};

//*****************************************************************************
// GPIO ISR
//*****************************************************************************
void
am_gpio_isr(void)
{
    //
    // Delay for debounce.
    //
    am_util_delay_ms(200);

    //
    // Clear the GPIO Interrupt (write to clear).
    //
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));

    //
    // Toggle LED 0.
    //
#ifdef AM_BSP_NUM_LEDS
    am_devices_led_toggle(am_bsp_psLEDs, 0);
#endif
}

int _main(void)
{
    am_util_id_t sIdDevice;
    uint32_t ui32StrBuf;

    //
    // Set the clock frequency.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();


#if defined(AM_BSP_NUM_BUTTONS)  &&  defined(AM_BSP_NUM_LEDS)
    //
    // Configure the button pin.
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_BUTTON0, g_deepsleep_button0);

    //
    // Clear the GPIO Interrupt (write to clear).
    //
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));

    //
    // Enable the GPIO/button interrupt.
    //
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BUTTON0));

    //
    // Configure the LEDs.
    //
    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);

    //
    // Turn the LEDs off, but initialize LED1 on so user will see something.
    //
    for (int ix = 0; ix < AM_BSP_NUM_LEDS; ix++)
    {
        am_devices_led_off(am_bsp_psLEDs, ix);
    }

    am_devices_led_on(am_bsp_psLEDs, 1);
#endif // defined(AM_BSP_NUM_BUTTONS)  &&  defined(AM_BSP_NUM_LEDS)

#if AM_CMSIS_REGS
    NVIC_EnableIRQ(GPIO_IRQn);
    NVIC_EnableIRQ(RTC_IRQn);
#else // AM_CMSIS_REGS
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_RTC);
#endif // AM_CMSIS_REGS

    //
    // Enable interrupts to the core.
    //
    am_hal_interrupt_master_enable();


    //
    // Initialize the printf interface for UART output
    //
    am_bsp_uart_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Hello World!\n\n");

    //
    // Print the device info.
    //
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n", sIdDevice.pui8DeviceName);


    am_util_stdio_printf("Qualified: %s\n",
                         sIdDevice.sMcuCtrlDevice.ui32Qualified ?
                         "Yes" : "No");

    am_util_stdio_printf("Device Info:\n"
                         "\tPart number: 0x%08X\n"
                         "\tChip ID0:    0x%08X\n"
                         "\tChip ID1:    0x%08X\n"
                         "\tRevision:    0x%08X (Rev%c%c)\n",
                         sIdDevice.sMcuCtrlDevice.ui32ChipPN,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID0,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID1,
                         sIdDevice.sMcuCtrlDevice.ui32ChipRev,
                         sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );

    //
    // If not a multiple of 1024 bytes, append a plus sign to the KB.
    //
    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32FlashSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tFlash size:  %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32FlashSize,
                         sIdDevice.sMcuCtrlDevice.ui32FlashSize / 1024,
                         &ui32StrBuf);

    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32SRAMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tSRAM size:   %7d (%d KB%s)\n\n",
                         sIdDevice.sMcuCtrlDevice.ui32SRAMSize,
                         sIdDevice.sMcuCtrlDevice.ui32SRAMSize / 1024,
                         &ui32StrBuf);

    //
    // Print the compiler version.
    //
    am_util_stdio_printf("App Compiler:    %s\n", COMPILER_VERSION);
#ifdef AM_PART_APOLLO3
    am_util_stdio_printf("HAL Compiler:    %s\n", g_ui8HALcompiler);
    am_util_stdio_printf("HAL SDK version: %d.%d.%d\n",
                         g_ui32HALversion.s.Major,
                         g_ui32HALversion.s.Minor,
                         g_ui32HALversion.s.Revision);
    am_util_stdio_printf("HAL compiled with %s-style registers\n",
                         g_ui32HALversion.s.bAMREGS ? "AM_REG" : "CMSIS");

    am_util_stdio_printf("&sIdDevice: 0x%x, &ui32StrBuf: 0x%x\n", &sIdDevice, &ui32StrBuf);
    am_hal_security_info_t secInfo;
    char sINFO[32];
    uint32_t ui32Status;
#endif // AM_PART_APOLLO3

    while(1)
    {
    }

    //main(0, NULL);
}
