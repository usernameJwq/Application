#include "Application.h"

#include "../base/log.h"

#define APP_VERSION "1.0.0"

int main() {
    init_logger();
    LOG::info("APP VERSION {}", APP_VERSION);

    return 0;
}
