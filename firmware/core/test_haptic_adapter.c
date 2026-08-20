#include "haptic_adapter.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int pass;
    int fail;
} score_t;

typedef struct {
    uint8_t read_go;
    uint8_t read_status;
    uint8_t last_reg;
    uint8_t last_data[HA_WAVEFORM_SLOTS];
    size_t last_length;
    unsigned writes;
    unsigned reads;
    unsigned fail_write_at;
    unsigned fail_read_at;
    uint8_t sequence[8];
    unsigned sequence_length;
} mock_bus_t;

static void check(score_t *score, int condition, const char *label)
{
    printf("  %s  %s\n", condition ? "PASS" : "FAIL", label);
    if (condition) score->pass++; else score->fail++;
}

static int mock_write(void *context, uint8_t address7, uint8_t reg,
                      const uint8_t *data, size_t length)
{
    mock_bus_t *mock = context;
    (void)address7;
    mock->writes++;
    if (mock->sequence_length < sizeof(mock->sequence))
        mock->sequence[mock->sequence_length++] = reg;
    if (mock->fail_write_at != 0u && mock->writes == mock->fail_write_at)
        return -1;
    mock->last_reg = reg;
    mock->last_length = length;
    if (length > sizeof(mock->last_data)) return -1;
    memcpy(mock->last_data, data, length);
    return 0;
}

static int mock_read(void *context, uint8_t address7, uint8_t reg,
                     uint8_t *data, size_t length)
{
    mock_bus_t *mock = context;
    (void)address7;
    mock->reads++;
    if (mock->fail_read_at != 0u && mock->reads == mock->fail_read_at)
        return -1;
    if (length != 1u) return -1;
    if (reg == HA_REG_GO) data[0] = mock->read_go;
    else if (reg == HA_REG_STATUS) data[0] = mock->read_status;
    else return -1;
    return 0;
}

static hl_profile_t profile_fixture(void)
{
    hl_profile_t profile;
    memset(&profile, 0, sizeof(profile));
    profile.version = HL_VERSION_1;
    profile.actuator = HL_ACTUATOR_LRA;
    profile.profile_version = 3u;
    profile.effect_sync = 1u;
    profile.effect_mark = 2u;
    profile.effect_end = 3u;
    for (uint8_t i = 0u; i < HL_CODEBOOK_SIZE; i++)
        profile.effect_code[i] = (uint8_t)(10u + i);
    return profile;
}

static hl_event_t event_fixture(void)
{
    hl_event_t event;
    memset(&event, 0, sizeof(event));
    event.version = HL_VERSION_1;
    event.scope = HL_SCOPE_COM;
    event.class_code = HL_CLASS_NOTICE;
    event.state = HL_STATE_PENDING;
    event.urgency = HL_URGENCY_U1;
    event.fragment_total = 1u;
    return event;
}

int main(void)
{
    score_t score = { 0, 0 };
    mock_bus_t mock;
    ha_bus_t bus;
    ha_config_t config;
    ha_device_t device;
    hl_profile_t profile = profile_fixture();
    hl_event_t event = event_fixture();
    hl_encoded_t encoded;
    hl_profile_t bad_profile;
    memset(&mock, 0, sizeof(mock));
    bus.write = mock_write;
    bus.read = mock_read;
    bus.context = &mock;
    config.address7 = HA_DRV2605L_ADDR7;
    config.active_mode = HA_MODE_ACTIVE;
    config.standby_mode = HA_MODE_STANDBY;
    check(&score, hl_encode(&event, &profile, &encoded) == HL_OK,
          "fixture event encodes before adapter use");
    check(&score, ha_init(&device, &bus, &config) == HA_OK &&
                    ha_state(&device) == HA_STATE_IDLE,
          "adapter initializes in idle with an injected bus");
    check(&score, ha_validate_frame(&encoded, &profile) == HA_OK,
          "adapter validates the complete frame against the profile");
    bad_profile = profile;
    bad_profile.actuator = HL_ACTUATOR_ERM;
    check(&score, ha_validate_frame(&encoded, &bad_profile) == HA_E_FRAME &&
                    mock.writes == 0u,
          "incompatible profile is rejected before any I2C write");
    check(&score, ha_play(&device, &encoded, &profile) == HA_OK &&
                    ha_state(&device) == HA_STATE_PLAYING &&
                    mock.sequence_length == 3u &&
                    mock.sequence[0] == HA_REG_MODE &&
                    mock.sequence[1] == HA_REG_WAVEFORM_BASE &&
                    mock.sequence[2] == HA_REG_GO,
          "playback writes mode, eight waveform slots and GO in order");
    check(&score, mock.last_reg == HA_REG_GO && mock.last_length == 1u &&
                    mock.last_data[0] == HA_GO_BIT,
          "GO write is explicit and bounded");
    check(&score, ha_play(&device, &encoded, &profile) == HA_E_BUSY,
          "second playback is refused while the first is active");
    mock.read_go = HA_GO_BIT;
    check(&score, ha_poll(&device) == HA_E_BUSY &&
                    ha_state(&device) == HA_STATE_PLAYING,
          "polling reports busy without claiming completion");
    mock.read_go = 0u;
    mock.read_status = 0u;
    check(&score, ha_poll(&device) == HA_OK &&
                    ha_state(&device) == HA_STATE_DONE,
          "completed playback requires GO clear and clean status");

    memset(&mock, 0, sizeof(mock));
    bus.context = &mock;
    check(&score, ha_init(&device, &bus, &config) == HA_OK &&
                    ha_play(&device, &encoded, &profile) == HA_OK,
          "fresh adapter can begin a second controlled playback");
    check(&score, ha_abort(&device) == HA_OK &&
                    ha_state(&device) == HA_STATE_ABORTED &&
                    mock.last_reg == HA_REG_MODE &&
                    mock.last_data[0] == HA_MODE_STANDBY,
          "abort writes standby and enters an explicit aborted state");
    check(&score, ha_poll(&device) == HA_E_STATE,
          "aborted playback cannot be reported as complete");

    memset(&mock, 0, sizeof(mock));
    mock.fail_write_at = 2u;
    bus.context = &mock;
    check(&score, ha_init(&device, &bus, &config) == HA_OK &&
                    ha_play(&device, &encoded, &profile) == HA_E_BUS &&
                    ha_state(&device) == HA_STATE_FAULT &&
                    mock.writes == 2u,
          "I2C write failure enters fault without continuing playback");
    check(&score, ha_play(&device, &encoded, &profile) == HA_E_STATE,
          "faulted adapter blocks automatic retry");

    memset(&mock, 0, sizeof(mock));
    mock.fail_read_at = 1u;
    bus.context = &mock;
    check(&score, ha_init(&device, &bus, &config) == HA_OK &&
                    ha_play(&device, &encoded, &profile) == HA_OK &&
                    ha_poll(&device) == HA_E_BUS &&
                    ha_state(&device) == HA_STATE_FAULT,
          "I2C read failure during polling enters fault closed");
    check(&score, ha_abort(&device) == HA_E_STATE,
          "fault state does not claim an abort succeeded");

    event.has_data = 1u;
    check(&score, hl_encode(&event, &profile, &encoded) == HL_E_FRAGMENT,
          "oversized semantic payload never reaches the adapter");

    printf("HAPTIC ADAPTER: %d pass, %d fail\n", score.pass, score.fail);
    return score.fail ? 1 : 0;
}
