/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tusb.h"
#include "bsp/board_api.h"
#include "usb.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void cdc_task(void);

/*------------- MAIN -------------*/
void init_pico_usb_drive() {
    set_tud_msc_ejected(false);
    board_init();
    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);
    if (board_init_after_tusb) {
       board_init_after_tusb();
    }
}

void pico_usb_drive_heartbeat() {
    tud_task(); // tinyusb device task
    led_blinking_task();
    cdc_task();
}

void in_flash_drive() {
  init_pico_usb_drive();
  while(!tud_msc_ejected()) {
    pico_usb_drive_heartbeat();
    if_swap_drives();
  }
  for (int i = 0; i < 10; ++i) { // sevaral hb till end of cycle
    pico_usb_drive_heartbeat();
    sleep_ms(50);
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
// Invoked when device is mounted
void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

// Invoked to determine max LUN
uint8_t tud_msc_get_maxlun_cb(void) {
  return 4;
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  // if ( tud_cdc_connected() )
  {
    // connected and there are data available
    if ( tud_cdc_available() )
    {
      // read data
      char buf[64];
      uint32_t count = tud_cdc_read(buf, sizeof(buf));
      (void) count;

      // Echo back
      // Note: Skip echo by commenting out write() and write_flush()
      // for throughput test e.g
      //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
      tud_cdc_write(buf, count);
      tud_cdc_write_flush();
    }
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if ( dtr )
  {
    // Terminal connected
  }else
  {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

static volatile bool backspacePressed = false;
static volatile bool enterPressed = false;
static volatile bool plusPressed = false;
static volatile bool minusPressed = false;
static volatile bool ctrlPressed = false;
static volatile bool tabPressed = false;

void handleScancode(uint32_t ps2scancode) {
    switch (ps2scancode) {
      case 0x0E:
        backspacePressed = true;
        break;
      case 0x8E:
        backspacePressed = false;
        break;
      case 0x1C:
        enterPressed = true;
        break;
      case 0x9C:
        enterPressed = false;
        break;
      case 0x4A:
        minusPressed = true;
        break;
      case 0xCA:
        minusPressed = false;
        break;
      case 0x4E:
        plusPressed = true;
        break;
      case 0xCE:
        plusPressed = false;
        break;
      case 0x1D:
        ctrlPressed = true;
        break;
      case 0x9D:
        ctrlPressed = false;
        break;
      case 0x0F:
        tabPressed = true;
        break;
      case 0x8F:
        tabPressed = false;
        break;
    }
}

#include "emulator.h"
static volatile bool usbStarted = false;
static const char* path = "\\XT\\video.ram";
static FATFS fs;
static FIL file;
static bool save_video_ram() {
    gpio_put(PICO_DEFAULT_LED_PIN, true);
    FRESULT result = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (result != FR_OK) {
        return false;
    }
    UINT bw;
    result = f_write(&file, VIDEORAM, sizeof(VIDEORAM), &bw);
    if (result != FR_OK) {
        return false;
    }
    f_close(&file);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    return true;
}
static bool restore_video_ram() {
    gpio_put(PICO_DEFAULT_LED_PIN, true);
    FRESULT result = f_open(&file, path, FA_READ);
    if (result == FR_OK) {
      UINT bw;
      result = f_read(&file, VIDEORAM, sizeof(VIDEORAM), &bw);
      if (result != FR_OK) {
        return false;
      }
    }
    f_close(&file);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    return true;
}

#include "vga.h"

int overclock() {
  if (tabPressed && ctrlPressed) {
    if (plusPressed) return 1;
    if (minusPressed) return -1;
  }
  return 0;
}

void if_usb() {
    if (usbStarted) {
        return;
    }
    if (backspacePressed && enterPressed) {
        usbStarted = true;
        save_video_ram();
        enum graphics_mode_t ret = graphics_set_mode(TEXTMODE_80x30);
        set_start_debug_line(0);
        clrScr(1);
        logMsg("");
        logMsg("                      You are in USB mode now.");
        logMsg("  To return back to PC/XT emulation, just eject an USB-drive from your PC...");
        logMsg("");
        in_flash_drive();
        set_start_debug_line(25);
        restore_video_ram();
        if (ret == TEXTMODE_80x30) {
          clrScr(1);
        }
        usbStarted = false;
        graphics_set_mode(ret);
    }
}
static bool already_swapped_fdds = false;
void if_swap_drives() {
    if (usbStarted) {
        return;
    }
    if (backspacePressed && tabPressed && ctrlPressed) {
        if (already_swapped_fdds) {
            insertdisk(0, fdd0_sz(), fdd0_rom(), "\\XT\\fdd0.img");
            insertdisk(1, fdd1_sz(), fdd1_rom(), "\\XT\\fdd1.img");
            already_swapped_fdds = false;
            return;
        }
        insertdisk(1, fdd0_sz(), fdd0_rom(), "\\XT\\fdd0.img");
        insertdisk(0, fdd1_sz(), fdd1_rom(), "\\XT\\fdd1.img");
        already_swapped_fdds = true;
    }
}