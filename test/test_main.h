#include <AUnit.h>

#define DEBUG_LEVEL DBG
#include "../src/log.h"
#include "test_helpers.h"
#include "checksum_test.h"
#include "encrypt_test.h"
#include "http_adapter_test.h"
#include "json_test.h"
#include "modem_cli_test.h"
#include "mower_adapter_test.h"
#include "prometheus_adapter_test.h"
#include "reader_test.h"
#include "router_test.h"
#include "settings_test.h"
#include "ui_test.h"

void setup() {
  setup_serial();
  setup_network();
  setup_asyncLooper();
  setup_fake_ardumower();
  start_wait();
}

void loop() {
  aunit::TestRunner::run();
}
