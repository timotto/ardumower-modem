#include <AUnit.h>

#include "../src/settings.h"
#include "test_helpers.h"

test(settings_defaults)
{
  ArduMower::Modem::Settings::Settings uut("/test/settings");

  // uut.begin() would overwrite defaults
  
  assertEqual( uut.general.name, "Ardumower" );
  assertEqual( uut.general.encryption, true );
  assertEqual( uut.general.password, (unsigned int)123456 );
}

test(settings_save_and_begin)
{
  ArduMower::Modem::Settings::Settings instance1("/test/settings");
  ArduMower::Modem::Settings::Settings instance2("/test/settings");

  instance1.general.name = "A name";
  assertTrue( instance1.save() );

  instance2.begin();
  assertEqual( instance2.general.name, String("A name"));

  instance2.general.name = "A different name";
  assertTrue( instance2.save() );

  instance1.begin();
  assertEqual( instance1.general.name, String("A different name"));
}

test(settings_valid)
{
  ArduMower::Modem::Settings::Settings uut("/test/settings");
  String invalid;

  // default is valid
  assertTrue( uut.valid(invalid) );

  // depends on general
  uut.general.name = "";
  assertFalse( uut.valid(invalid) );
  assertEqual( invalid, String("general.name") );
  uut.general.name = "a-legal-name";
  assertTrue( uut.valid(invalid) );

  // depends on web
  uut.web.use_password = true;
  uut.web.username = "";
  assertFalse( uut.valid(invalid) );
  assertEqual( invalid, String("web.username") );
  uut.web.use_password = false;
  assertTrue( uut.valid(invalid) );

  // depends on wifi
  uut.wifi.mode = 1;
  uut.wifi.sta_ssid = "";
  assertFalse( uut.valid(invalid) );
  assertEqual( invalid, String("wifi.sta_ssid") );
  uut.wifi.mode = 0;
  assertTrue( uut.valid(invalid) );
}

test(settings_valid_general)
{
  ArduMower::Modem::Settings::Settings uut("/test/settings");
  String invalid;

  uut.general.name = "";
  assertFalse( uut.valid(invalid) );
  assertEqual( invalid, String("general.name") );

  uut.general.name = "legal-name";
  assertTrue( uut.valid(invalid) );
}

test(settings_valid_name)
{
  ArduMower::Modem::Settings::Settings uut("/test/settings");
  String invalid;

  uut.general.name = "with spaces";
  assertFalse( uut.valid(invalid) );

  uut.general.name = "with-dash-at-end-";
  assertFalse( uut.valid(invalid) );

  uut.general.name = "-with-dash-at-start";
  assertFalse( uut.valid(invalid) );

  uut.general.name = "with.dots";
  assertFalse( uut.valid(invalid) );

  uut.general.name = "with/slash";
  assertFalse( uut.valid(invalid) );

  uut.general.name = "with+plus";
  assertFalse( uut.valid(invalid) );

  uut.general.name = "123-starts-with-number";
  assertFalse( uut.valid(invalid) );

  uut.general.name = "a-25-character-long-nameX";
  assertFalse( uut.valid(invalid) );

  uut.general.name = "a-24-character-long-name";
  assertTrue( uut.valid(invalid) );
}

#define assertValidWeb(uut,protect,user,pass,expect) { \
  uut.web.use_password = protect; \
  uut.web.username = user; \
  uut.web.password = pass; \
  assertEqual( uut.valid(invalid), expect);}
test(settings_valid_web)
{
  ArduMower::Modem::Settings::Settings uut("/test/settings");
  String invalid;

  // when no password is required, the values don't matter
  assertValidWeb(uut, false, "some user", "some password", true);
  assertValidWeb(uut, false, "", "some password", true);
  assertValidWeb(uut, false, "some user", "", true);

  // when a password is required, the values must be set
  assertValidWeb(uut, true, "some user", "some password", true);
  assertValidWeb(uut, true, "", "some password", false);
  assertEqual( invalid, String("web.username") );
  assertValidWeb(uut, true, "some user", "", false);
  assertEqual( invalid, String("web.password") );
}

#define assertValidWifi(uut,m,staSsid,staPsk,apSsid,apPsk,expect) { \
  uut.wifi.mode = m; \
  uut.wifi.sta_ssid = staSsid; \
  uut.wifi.sta_psk = staPsk; \
  uut.wifi.ap_ssid = apSsid; \
  uut.wifi.ap_psk = apPsk; \
  assertEqual( uut.valid(invalid), expect);}
test(settings_valid_wifi)
{
  ArduMower::Modem::Settings::Settings uut("/test/settings");
  String invalid;

  int mode;

  // when off
  mode = 0;
  // then ap ssid & psk must be set
  assertValidWifi(uut, mode, "", "", "ap ssid", "ap psk", true);
  assertValidWifi(uut, mode, "", "", "ap ssid", "", false);
  assertEqual( invalid, String("wifi.ap_psk") );
  assertValidWifi(uut, mode, "", "", "", "ap psk", false);
  assertEqual( invalid, String("wifi.ap_ssid") );
  // sta settings don't matter
  assertValidWifi(uut, mode, "sta ssid", "", "ap ssid", "ap psk", true);
  assertValidWifi(uut, mode, "", "sta psk", "ap ssid", "ap psk", true);

  // when ap
  mode = 2;
  // then ap ssid & psk must be set
  assertValidWifi(uut, mode, "", "", "ap ssid", "ap psk", true);
  assertValidWifi(uut, mode, "", "", "ap ssid", "", false);
  assertEqual( invalid, String("wifi.ap_psk") );
  assertValidWifi(uut, mode, "", "", "", "ap psk", false);
  assertEqual( invalid, String("wifi.ap_ssid") );
  // sta settings don't matter
  assertValidWifi(uut, mode, "sta ssid", "", "ap ssid", "ap psk", true);
  assertValidWifi(uut, mode, "", "sta psk", "ap ssid", "ap psk", true);


  // when sta
  mode = 1;
  // then sta ssid & psk must be set
  assertValidWifi(uut, mode, "sta ssid", "sta psk", "ap ssid", "ap psk", true);
  assertValidWifi(uut, mode, "sta ssid", "", "ap ssid", "ap psk", false);
  assertEqual( invalid, String("wifi.sta_psk") );
  assertValidWifi(uut, mode, "", "sta psk", "ap ssid", "ap psk", false);
  assertEqual( invalid, String("wifi.sta_ssid") );
  // ap settings don't matter
  assertValidWifi(uut, mode, "sta ssid", "sta psk", "ap ssid", "", true);
  assertValidWifi(uut, mode, "sta ssid", "sta psk", "", "ap psk", true);

  // invalid mode
  assertValidWifi(uut, 5, "sta ssid", "sta psk", "ap ssid", "ap psk", false);
  assertEqual( invalid, String("wifi.mode") );
}

test(settings_valid_mqtt)
{
  ArduMower::Modem::Settings::Settings uut("/test/settings");
  String invalid;

  // server is required
  uut.mqtt.enabled = true;
  uut.mqtt.server = "";
  assertFalse( uut.mqtt.valid(invalid) );
  assertEqual( invalid, String("mqtt.server") );

  uut.mqtt.server = "192.168.2.3";
  assertTrue( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "192.168.2.3.4";
  assertFalse( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "some-server";
  assertTrue( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "server.with.dots";
  assertTrue( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "server-with-dashes";
  assertTrue( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "server-with-numers-123";
  assertTrue( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "server with spaces";
  assertFalse( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "server-with-dash-at-end-";
  assertFalse( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "-server-with-dash-at-start";
  assertFalse( uut.mqtt.valid(invalid) );

  uut.mqtt.server = "123server-with-numer-at-start";
  assertFalse( uut.mqtt.valid(invalid) );

  // when off values don't matter
  uut.mqtt.enabled = false;
  uut.mqtt.server = "";
  assertTrue( uut.mqtt.valid(invalid) );
}

test(properties_version_is_not_empty)
{
  const char * versionString = ArduMower::Modem::Settings::Properties.version();
  assertTrue(versionString != nullptr);

  const size_t actualLength = strlen(versionString);
  const size_t unexpectedLength = 0;
  assertMore(actualLength, unexpectedLength);

  Serial.printf("props.version = %s\n", versionString);
}
