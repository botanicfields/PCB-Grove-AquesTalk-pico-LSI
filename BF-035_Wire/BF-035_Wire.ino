// Copyright 2021 BotanicFields, Inc.
// BF-035 AquesTalk pico LSI into the Grove of M5
// example

//#define M5STACK
#define M5ATOM

#ifdef M5STACK
#include <M5Stack.h>
#endif
#ifdef M5ATOM
#include <M5Atom.h>
#endif
#include "BF_AquesTalkPicoWire.h"

AquesTalkPicoWire aqtp;

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

// for loop control
const int loop_ms(100);  // 100ms
unsigned int loop_last_ms(0);  // loop control
int msg_select(0);

void setup()
{
#ifdef M5STACK
  const bool lcd_enable(true);
  const bool sd_enable(true);
  const bool serial_enable(true);
  const bool i2c_enable(true);  // SCL = GPIO22, SDA = GPIO21, frequency = 100kHz
  M5.begin(!lcd_enable, sd_enable, serial_enable, i2c_enable);
  aqtp.Begin(Wire);          // default or safe mode
#endif
#ifdef M5ATOM
  const bool serial_enable(true);
  const bool i2c_enable(true);  // SCL = GPIO21, SDA = GPIO25, frequency = 100kHz
  const bool display_enable(true);
  M5.begin(serial_enable, i2c_enable, !display_enable);

  const int wire1_sda(26);       // GPIO26
  const int wire1_scl(32);       // GPIO32
  const int wire1_freq(100000);  // 100kHz
  Wire1.begin(wire1_sda, wire1_scl, wire1_freq);

  // select Wire(system i2c) or Wire1(Grove connector)
  //aqtp.Begin(Wire);
  aqtp.Begin(Wire1);
#endif

  // designate "true" if sleep pin is connected
  if (/*true*/ false) {
    const int aqtp_sleep_pin(5);  // GPIO5  for sleep pin of grove board
    pinMode(aqtp_sleep_pin, OUTPUT);
    digitalWrite(aqtp_sleep_pin, HIGH);
  }

  // designate "true" to write i2c address into EEPROM
  if (/*true*/ false)
    aqtp.WriteI2cAddress(0x2E);  // change i2c address to customize

  // designate "true" to write preset message into EEPROM
  if (/*true*/ false)
    aqtp.WritePresetMsg(preset_msg, sizeof(preset_msg)/sizeof(preset_msg[0]));

  // designate "true" to dump EEPROM to the serial monitor
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

  loop_last_ms = millis();
}

void loop()
{
  M5.update();

  int num_of_msg = sizeof(preset_msg)/sizeof(preset_msg[0]);

#ifdef M5STACK
  if (M5.BtnA.wasReleased()) {
    if (--msg_select < 0)
      msg_select = num_of_msg - 1;
    aqtp.Send(preset_msg[msg_select]);
  }

  if (M5.BtnB.wasReleased()) {
    aqtp.Send(preset_msg[msg_select]);
  }

  if (M5.BtnC.wasReleased()) {
    if (++msg_select >= num_of_msg)
      msg_select = 0;
    aqtp.Send(preset_msg[msg_select]);
  }
#endif
#ifdef M5ATOM
  if (M5.Btn.wasReleased()) {
    aqtp.Send(preset_msg[msg_select]);
    if (++msg_select >= num_of_msg)
      msg_select = 0;
  }
#endif

  aqtp.ShowRes(2);

  delay(loop_ms + loop_last_ms - millis());
  loop_last_ms = millis();
}
