#include "stub.h"
typedef int (*esp_console_cmd_func_t)(int argc, char **argv);
typedef struct { const char *command, *help, *hint; esp_console_cmd_func_t func; } esp_console_cmd_t;
typedef struct { const char *prompt; int max_cmdline_length; } esp_console_repl_config_t;
typedef struct { int dummy; } esp_console_repl_t;
typedef struct { int dummy; } esp_console_dev_usb_serial_jtag_config_t;
typedef struct { int dummy; } esp_console_dev_uart_config_t;
#define ESP_CONSOLE_REPL_CONFIG_DEFAULT() (esp_console_repl_config_t){ "esp>", 128 }
#define ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT() (esp_console_dev_usb_serial_jtag_config_t){0}
#define ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT() (esp_console_dev_uart_config_t){0}
static inline esp_err_t esp_console_cmd_register(const esp_console_cmd_t *c) { (void)c; return ESP_OK; }
static inline esp_err_t esp_console_register_help_command(void) { return ESP_OK; }
static inline esp_err_t esp_console_new_repl_usb_serial_jtag(const esp_console_dev_usb_serial_jtag_config_t *d, const esp_console_repl_config_t *r, esp_console_repl_t **o) { (void)d;(void)r; static esp_console_repl_t s; *o=&s; return ESP_OK; }
static inline esp_err_t esp_console_new_repl_uart(const esp_console_dev_uart_config_t *d, const esp_console_repl_config_t *r, esp_console_repl_t **o) { (void)d;(void)r; static esp_console_repl_t s; *o=&s; return ESP_OK; }
static inline esp_err_t esp_console_start_repl(esp_console_repl_t *r) { (void)r; return ESP_OK; }
