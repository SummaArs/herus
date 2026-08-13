idf_component_register(
    SRCS "app_main.c"
         "test_radio.c"
         "../hal_esp32s3.c"
         "../../sx1262.c"
         "../../../core/hv.c"
         "../../../core/sbc.c"
         "../../../core/lexicon.c"
         "../../../core/hcp.c"
         "../../../net/crypto.c"
         "../../../net/session.c"
         "../../../net/region.c"
         "../../../net/weave.c"
         "../../../net/beat.c"
         "../../../net/link.c"
    INCLUDE_DIRS "." ".." "../.." "../../../core" "../../../net"
    REQUIRES driver esp_timer nvs_flash console unity
)
# HV_LUT_POPCOUNT: neither the Xtensa LX7 nor the Cortex-M33 has a population-count
# instruction, so the byte-table path is what the MCU actually executes. Building
# without it on target would be a compile error waiting to happen, not a speedup.
target_compile_definitions(${COMPONENT_LIB} PRIVATE HV_LUT_POPCOUNT)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)
