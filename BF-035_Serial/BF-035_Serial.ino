// Copyright 2021 BotanicFields, Inc.
// BF-035 AquesTalk pico LSI into the Grove of M5
// example

// select one from M5STACK, M5ATOM
#define M5STACK
//#define M5ATOM

#ifdef M5STACK
  #include <M5Stack.h>
#endif

#ifdef M5ATOM
  #include <M5Atom.h>
#endif

#include "BF_AquesTalkPicoSerial.h"
AquesTalkPicoSerial aqtp;

const char* preset_msg[] = {
// msssage must be less than 127 characters
// ....:....1....:....2....:....3....:....4....:....5....:....6....:....7....:....8....:....9....:....A....:....8....:....C....:..
  "re'-jide_su.yo'-koso wata'ku_sitati,sho'kka-e\r",
  "iti'jide_su.ittai,do'-natte/iru'node_suka zo'rutaisa\r",
  "ni'jide_su.ma'da,deki'nainde_suka sinigamiha'kase\r",
  "sa'njide_su.sho'kka-no,se'kai,seifuku/ke'ikakuwa cha_ku'cha_kuto,_susun'deima_su\r",
  "yo'jide_su.omedeto-/goza'i/ma'_su jikken'wa,dai'seiko-de_su\r",
  "go'jide_su.sho'kka-no/tikara'wa,muge'n/na'node_su\r",
  "roku'jide_su.urotae'/nai'de,kudasa'i,jigo'kutai_si\r",
  "siti'jide_su.wata'_ku_sitati,sho'kka-no/shuku'teki kamen'raida-o/tao'_sunode_su\r",
  "hati'jide_su.sa'-/yukina'sai sho'kka-no/mono'tatiyo\r",
  "ku'jide_su.kiki'masho- ana'tano,niho'n,seifuku/ke'ikakuo\r",
  "jyu'-jide_su.oda'marinasai,jigo'kutai_si su'beteno/sippa'iwa,ana'tano/se'ide_su\r",
  "jyu-iti'jide_su.sho'kka-wa,su'beteo/haka'i_si,se'kaio/_si'hai_sima_su\r",
  "jyu-ni'jide_su.kaizo-/ni'ngenga,se'kaio/ugo'ka_si sono/kaizo-/ni'ngen+o,_si'hai/suru'noga,wata'ku_side_su\r",
  "ohayo-/gozaima'_su.ta'datini,saga_si/da'_sunode_su\r",
  "oyasumi/nasa'i.se'kaiwa,sho'kka-no/mono'de_su\r",
};

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// main
enum play_command_t {
  play_stop,
  play_current,
  play_next,
  play_previous,
  play_continuous,  // play continuously from current
  play_forward,     // play continuously from next
  play_backward,    // play continuously from previous
};
play_command_t play_command(play_stop);
int msg_selected(0);

const unsigned int loop_period_ms(100);
      unsigned int loop_last_ms;

void setup()
{
#ifdef M5STACK
  const bool lcd_enable(true);
  const bool sd_enable(true);
  const bool serial_enable(true);
  const bool i2c_enable(true);
  M5.begin(lcd_enable, sd_enable, serial_enable, i2c_enable);

  // Port UART
  Serial2.begin( 9600);  // RX = GPIO16, TX = GPIO17, SERIAL_8N1, 9600bps
//  Serial2.begin(38400);  // RX = GPIO16, TX = GPIO17, SERIAL_8N1, 38400bps
#endif

#ifdef M5ATOM
  const bool serial_enable(true);
  const bool i2c_enable(true);
  const bool display_enable(true);
  M5.begin(serial_enable, i2c_enable, !display_enable);
  // Port UART to Grove
  const int serial2_rx(32);  // GPIO32
  const int serial2_tx(26);  // GPIO26
  Serial2.begin( 9600, SERIAL_8N1, serial2_rx, serial2_tx);
//  Serial2.begin(38400, SERIAL_8N1, serial2_rx, serial2_tx);
#endif

  while (!Serial2) {
    Serial.println("[AquesTalk LSI] Waiting Serial2");
    delay(100);
  }
  aqtp.Begin(Serial2);

  // "true" if sleep pin is connected
  if (/*true*/ false) {
//    const int aqtp_sleep_pin(5);  // GPIO5 for sleep pin of Grove board
    const int aqtp_sleep_pin(19);  // GPIO19 for sleep pin of Grove board
    pinMode(aqtp_sleep_pin, OUTPUT);
    digitalWrite(aqtp_sleep_pin, HIGH);

    // "true" to set speed automatic for ATP3011 & not safe mode
    if (/*true*/ false) {
      digitalWrite(aqtp_sleep_pin, LOW);
      delay(500);
      digitalWrite(aqtp_sleep_pin, HIGH);
      delay(80);
      aqtp.Send("?");
      for (int i = 0; i < 10; ++i) {
        aqtp.ShowRes();
        delay(200);
      }
    }
  }

  // "true" to set serial-speed into EEPROM of ATP3012
  if (/*true*/ false) {
//    aqtp.WriteSerialSpeed(  9600);
    aqtp.WriteSerialSpeed( 38400);
//    aqtp.WriteSerialSpeed( 76800);
//    aqtp.WriteSerialSpeed(115200);
    for (int i = 0; i < 10; ++i) {
      aqtp.ShowRes();
      delay(200);
    }
  }

  // "true" to write preset message into EEPROM
  if (/*true*/ false)
    aqtp.WritePresetMsg(preset_msg, sizeof(preset_msg)/sizeof(preset_msg[0]));

  // "true" to dump EEPROM to the serial monitor
  if (/*true*/ false)
    aqtp.DumpEeprom();

  aqtp.Send("#V\r");  // read version
  for (int i = 0; i < 10; ++i) {
    aqtp.ShowRes();
    delay(200);
  }
  aqtp.Send("#J\r");  // chime sound J
  for (int i = 0; i < 10; ++i) {
    aqtp.ShowRes();
    delay(200);
  }
  aqtp.Send("#K\r");  // chime sound K
  for (int i = 0; i < 10; ++i) {
    aqtp.ShowRes();
    delay(200);
  }

  // play control
  play_command = play_stop;
  msg_selected = 0;

  // loop control
  loop_last_ms = millis();
}

void loop()
{
  // buttons
  M5.update();

#ifdef M5STACK
  if (M5.BtnA.wasReleased()) {
    if (aqtp.Busy()) {
      aqtp.Send("$");  // Abort
    }
    play_command = play_previous;
  }
  if (M5.BtnB.wasReleased()) {
    if (aqtp.Busy()) {
      aqtp.Send("$");  // Abort
      play_command = play_stop;
    }
    else {
      play_command = play_current;
    }
  }
  if (M5.BtnC.wasReleased()) {
    if (aqtp.Busy()) {
      aqtp.Send("$");  // Abort
    }
    play_command = play_forward;
  }
#endif

#ifdef M5ATOM
  if (M5.Btn.wasReleased()) {
    if (aqtp.Busy()) {
      aqtp.Send("$");  // Abort
      play_command = play_stop;
    }
    else {
      play_command = play_continuous;
    }
  }
#endif

  // play messages
  int num_of_msg = sizeof(preset_msg)/sizeof(preset_msg[0]);

  switch (play_command) {
    case play_current:
      if (!aqtp.Busy()) {
        aqtp.Send(preset_msg[msg_selected]);
        play_command = play_stop;
      }
      break;
    case play_next:
      if (!aqtp.Busy()) {
        if (++msg_selected >= num_of_msg)
          msg_selected = 0;
        aqtp.Send(preset_msg[msg_selected]);
        play_command = play_stop;
      }
      break;
    case play_previous:
      if (!aqtp.Busy()) {
        if (--msg_selected < 0)
          msg_selected = num_of_msg - 1;
        aqtp.Send(preset_msg[msg_selected]);
        play_command = play_stop;
      }
      break;
    case play_continuous:
      if (!aqtp.Busy()) {
        delay(500);
        aqtp.Send(preset_msg[msg_selected]);
        if (++msg_selected >= num_of_msg)
          msg_selected = 0;
      }
      break;
    case play_forward:
      if (!aqtp.Busy()) {
        if (++msg_selected >= num_of_msg)
          msg_selected = 0;
        delay(500);
        aqtp.Send(preset_msg[msg_selected]);
      }
      break;
    case play_backward:
      if (!aqtp.Busy()) {
        if (--msg_selected < 0)
          msg_selected = num_of_msg - 1;
        delay(500);
        aqtp.Send(preset_msg[msg_selected]);
      }
      break;
    default:
        play_command = play_stop;
      break;
  }
  aqtp.ShowRes(2);

  // loop control
  unsigned int delay_ms(0);
  unsigned int elapse_ms = millis() - loop_last_ms;
  if (elapse_ms < loop_period_ms) {
    delay_ms = loop_period_ms - elapse_ms;
  }
  delay(delay_ms);
  loop_last_ms = millis();
//  Serial.printf("loop elapse = %dms\n", elapse_ms);  // for monitoring elapsed time
}