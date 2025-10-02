#include "stdinc.h"
#include "esp32-hal-timer.h"
#include "esp_cpu.h"

static TaskHandle_t handle_task2ms;

void IRAM_ATTR isr2ms()
{
  BaseType_t hasWoken = pdFALSE;
  vTaskNotifyGiveFromISR(handle_task2ms, &hasWoken);
  if (hasWoken) portYIELD_FROM_ISR();
}

void task2ms(void *p)
{
  for (;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    uint32_t t1 = esp_cpu_get_cycle_count();
    loop2ms();
    uint32_t dtl = esp_cpu_get_cycle_count() - t1;
    uint16_t dt = dtl / 240;
    sensorData.taskTime[ttmin] = min(sensorData.taskTime[ttmin], dt);
    sensorData.taskTime[ttmax] = max(sensorData.taskTime[ttmax], dt);
    sensorData.taskTime[ttsum] += dt;
    sensorData.taskTime[ttnum]++;
  }
}

void setupSystem()
{
  xTaskCreatePinnedToCore(task2ms, "task2ms", 4096, nullptr, 16, &handle_task2ms, 1);
  hw_timer_t *hwtim = timerBegin(1000000);
  timerAttachInterrupt(hwtim, &isr2ms);
  timerAlarm(hwtim, 2000, true, 0);
}
