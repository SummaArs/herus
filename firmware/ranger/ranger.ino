/* ranger.ino — Herus Phase 0 range probe.
 *
 * PURPOSE
 * -------
 * Answer the two questions that decide whether Herus is a wrist device, in one
 * weekend, before a single line of product firmware is written:
 *
 *   Q1  What is the real wrist-to-wrist PDR versus distance at SF9, with a
 *       34-byte frame, in your actual city?
 *       KILL CRITERION: < 150 m urban at >= 50% PDR -> the wrist is the wrong
 *       place for a sub-GHz radio. Pivot to a lapel or backpack clip.
 *
 *   Q2  How much further does a header-less, CRC-less frame get detected than a
 *       validated one? That gap is the entire Tier 0.5 prize, and the master
 *       document estimates it at 2-4 dB rather than the algebra's 25% BER.
 *       KILL CRITERION: < 2 dB equivalent -> drop Tier 0.5, keep CRC'd ids.
 *
 * Two passes, not one. Walk the route in MODE_PDR, then walk it again in
 * MODE_PROBE. Alternating configurations mid-flight needs the receiver to track
 * a schedule, and a timing bug in the field costs a whole day. Two clean passes
 * cost an hour.
 *
 * HARDWARE
 * --------
 * 2x LilyGO T3-S3 (ESP32-S3 + SX1262). One TX, one RX.
 * Library: RadioLib (Arduino Library Manager).
 *
 * NO GPS AND NO DISPLAY ARE REQUIRED. Frames go out on a strict 2.000 s
 * cadence, so elapsed time IS distance bookkeeping: start a stopwatch with the
 * TX, note the time at each measured distance mark, and tools/fieldlog.py maps
 * time to distance. Fewer dependencies, fewer field failures.
 *
 * BUILD
 * -----
 *   Role and mode are compile-time. Flash four combinations across the weekend:
 *     -DROLE_TX -DMODE_PDR      -DROLE_RX -DMODE_PDR
 *     -DROLE_TX -DMODE_PROBE    -DROLE_RX -DMODE_PROBE
 *   Or edit the two #defines below and use the IDE.
 */

#include <RadioLib.h>

/* ---- role and mode -------------------------------------------------- */
#if !defined(ROLE_TX) && !defined(ROLE_RX)
#define ROLE_TX                 /* <-- flip to ROLE_RX for the other board */
#endif
#if !defined(MODE_PDR) && !defined(MODE_PROBE)
#define MODE_PDR                /* <-- flip to MODE_PROBE for the second pass */
#endif

/* ---- pin map -------------------------------------------------------- *
 * VERIFY THESE AGAINST YOUR BOARD REVISION BEFORE THE FIRST POWER-UP.
 * LilyGO has shipped several T3-S3 pinouts. A wrong BUSY or DIO1 pin gives a
 * silent begin() failure that looks exactly like a dead radio, and you will
 * lose an afternoon to it. Cross-check against the schematic in
 * LilyGO/LilyGo-LoRa-Series/schematic/ for your exact revision.            */
#define PIN_SCK    5
#define PIN_MISO   3
#define PIN_MOSI   6
#define PIN_CS     7
#define PIN_RST    8
#define PIN_BUSY  34
#define PIN_DIO1  33
#define TCXO_V   1.8f          /* T3-S3 uses a TCXO; 0 would mean XTAL */

/* ---- radio parameters — must mirror docs/02-PROTOCOL.md ------------- */
#define FREQ_MHZ      915.0
#define BW_KHZ        125.0
#define SF               9     /* the legal ceiling for a 34 B frame     */
#define CR               5     /* 4/5                                    */
#define SYNC_WORD     0x12     /* private, never the LoRaWAN public word */
#define TX_DBM          14     /* ANATEL / FCC conducted target          */
#define PREAMBLE         8     /* symbols; 32.8 ms at SF9                */
#define FRAME_LEN       34     /* P1: every meaning-tier frame, exactly  */
#define PERIOD_MS     2000UL   /* strict cadence = distance bookkeeping  */

SX1262 radio = new Module(PIN_CS, PIN_DIO1, PIN_RST, PIN_BUSY);

/* A fixed, known 34-byte pattern. In MODE_PROBE the CRC is off, so frames
 * arrive corrupted; comparing against a known constant is what lets us measure
 * raw BER. Sequence numbers cannot serve that purpose because the sequence
 * field would itself be corrupt. */
static uint8_t pattern[FRAME_LEN];

static void build_pattern(void)
{
    /* splitmix64-ish fill: high entropy, zero storage, identical on both ends */
    uint64_t s = 0x4845525553303530ULL;   /* "HERUS050" */
    for (int i = 0; i < FRAME_LEN; i++) {
        s += 0x9E3779B97F4A7C15ULL;
        uint64_t z = s;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        pattern[i] = (uint8_t)((z ^ (z >> 31)) & 0xFF);
    }
}

static int popcount8(uint8_t x)
{
    int n = 0;
    while (x) { n += x & 1; x >>= 1; }
    return n;
}

void setup()
{
    Serial.begin(115200);
    delay(300);
    build_pattern();

    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

    int st = radio.begin(FREQ_MHZ, BW_KHZ, SF, CR, SYNC_WORD,
                         TX_DBM, PREAMBLE, TCXO_V, false);
    if (st != RADIOLIB_ERR_NONE) {
        Serial.printf("# FATAL radio.begin = %d — check the pin map\n", st);
        while (true) delay(1000);
    }

#ifdef MODE_PROBE
    /* Tier 0.5 configuration. Both gates removed:
     *   - implicit header: an explicit header carries its own CRC, so a
     *     corrupt header drops the frame before the payload is ever seen
     *   - payload CRC off: a CRC turns a recoverable frame into a discarded one
     * What remains is preamble detection, which is the thing we are measuring. */
    radio.implicitHeader(FRAME_LEN);
    radio.setCRC(0);
    const char *mode = "PROBE";
#else
    radio.explicitHeader();
    radio.setCRC(2);
    const char *mode = "PDR";
#endif

    /* CSV header. '#' lines are comments for tools/fieldlog.py. */
    Serial.printf("# herus-ranger mode=%s sf=%d bw=%.0f len=%d period=%lu\n",
                  mode, SF, BW_KHZ, FRAME_LEN, PERIOD_MS);
#ifdef ROLE_TX
    Serial.println("# role=TX");
    Serial.println("t_ms,seq");
#else
    Serial.println("# role=RX");
    Serial.println("t_ms,seq,rssi,snr,biterr,status");
#endif
}

#ifdef ROLE_TX

void loop()
{
    static uint32_t seq = 0;
    static uint32_t next = 0;

    /* Strict cadence. Drifting here corrupts the time-to-distance mapping,
     * which is the only distance reference this test has. */
    uint32_t now = millis();
    if (next == 0) next = now;
    if ((int32_t)(now - next) < 0) { delay(1); return; }
    next += PERIOD_MS;

    uint8_t buf[FRAME_LEN];
    memcpy(buf, pattern, FRAME_LEN);
    /* seq in the first two bytes is for MODE_PDR bookkeeping only; in
     * MODE_PROBE the receiver ignores it and counts bit errors instead. */
    buf[0] = (uint8_t)(seq & 0xFF);
    buf[1] = (uint8_t)((seq >> 8) & 0xFF);

    radio.transmit(buf, FRAME_LEN);
    Serial.printf("%lu,%lu\n", (unsigned long)now, (unsigned long)seq);
    seq++;
}

#else  /* ROLE_RX */

void loop()
{
    uint8_t buf[FRAME_LEN];
    int st = radio.receive(buf, FRAME_LEN);
    uint32_t now = millis();

    if (st == RADIOLIB_ERR_NONE
#ifdef MODE_PROBE
        || st == RADIOLIB_ERR_CRC_MISMATCH   /* expected: CRC is disabled */
#endif
        ) {
        uint32_t seq = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8);
        int biterr = 0;
#ifdef MODE_PROBE
        /* bytes 2.. are the known pattern; 0..1 carry seq and are excluded */
        for (int i = 2; i < FRAME_LEN; i++)
            biterr += popcount8((uint8_t)(buf[i] ^ pattern[i]));
#endif
        Serial.printf("%lu,%lu,%.1f,%.2f,%d,ok\n",
                      (unsigned long)now, (unsigned long)seq,
                      radio.getRSSI(), radio.getSNR(), biterr);
    } else if (st != RADIOLIB_ERR_RX_TIMEOUT) {
        Serial.printf("%lu,,,,,err%d\n", (unsigned long)now, st);
    }
}

#endif
