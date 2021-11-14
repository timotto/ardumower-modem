#pragma once

#include <functional>

class AsyncLooperClass
{
private:
  bool on;
  bool started;
  std::function<void(void)> loop;

private:
  void awaitStart()
  {
    while (!started)
      delay(1);
  }

  void awaitStop()
  {
    while (started)
      delay(1);
  }

public:
  AsyncLooperClass() : on(false), started(false), loop([]() {}) {}

  void start(std::function<void(void)> _loop)
  {
    loop = _loop;
    on = true;
    awaitStart();
  }

  void stop()
  {
    on = false;
    awaitStop();
  }

  void task()
  {
    while (true)
    {
      vPortYield();
      if (!on)
      {
        started = false;
        continue;
      }
      loop();
      started = true;
    }
  }
};

AsyncLooperClass AsyncLooper;

void asyncLooperTask(void *p)
{
  AsyncLooper.task();
}

void setup_asyncLooper()
{
  xTaskCreatePinnedToCore(asyncLooperTask, "asyncLooperTask", 4096, NULL, 1, NULL, 1);
}
