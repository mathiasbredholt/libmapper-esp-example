#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "mpr/mpr.h"
#include "protocol_examples_common.h"

void handler(mpr_sig sig, mpr_sig_evt evt, mpr_id inst, int length,
             mpr_type type, const void *value, mpr_time time) {
  printf("%d\n", *((int *)value));
}

void app_main() {
// Connect to Wifi
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());

  // Create libmpr device

  mpr_dev dev = mpr_dev_new("test", 0);

  // Create libmpr signals
  mpr_sig sig1 = mpr_sig_new(dev, MPR_DIR_IN, "input", 1, MPR_INT32, 0, 0, 0, 0,
                             handler, MPR_SIG_UPDATE);
  mpr_sig sig2 = mpr_sig_new(dev, MPR_DIR_OUT, "output", 1, MPR_INT32, 0, 0, 0,
                             0, handler, MPR_SIG_UPDATE);

  // Create map between signals, requires the device to be initialized,
  // so poll the device until it is ready
  while (!mpr_dev_get_is_ready(dev)) {
    mpr_dev_poll(dev, 25);
  }
  mpr_obj_push(mpr_map_new(1, &sig2, 1, &sig1));

  int i = 0;
  while (1) {
    // Poll device and receive signal values
    mpr_dev_poll(dev, 0);

    // Update and send signal value
    mpr_sig_set_value(sig2, 0, 1, MPR_INT32, &i, MPR_NOW);
    ++i;

    // Block task for 10 milliseconds
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
