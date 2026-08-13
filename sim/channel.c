/* channel.c — the radio half of the world.
 *
 * The propagation model is the one in tools/budget.py, inverted. budget.py asks
 * "given a link budget, how far?"; the simulator asks "given a distance, what
 * arrives?". They must be the same curve read in opposite directions, and
 * scenario_selftest() checks that they are: if this file and budget.py ever
 * disagree, the run fails before any scenario prints a number.
 *
 * WHY TWO-RAY AND NOT FREE SPACE
 * ------------------------------
 * At 915 MHz with both antennas at wrist height the breakpoint is
 * d_bp = 4*pi*h1*h2/lambda = 38 m. Past it the ground reflection turns path loss
 * into d^4. Almost the entire useful range of a wrist device therefore lives in
 * the d^4 regime, and a free-space model would overstate range by roughly the
 * square root of everything. This is the single most common way LoRa range gets
 * overestimated, so it is the first thing the simulator gets right.
 */
#include "sim.h"
#include <math.h>

/* M_PI is a common extension rather than an ISO C constant. Keep the propagation
 * model self-contained so the bench builds under strict C11 toolchains. */
#define SIM_PI 3.14159265358979323846

/* Sensitivity is not a lookup table, it is thermal noise plus receiver noise
 * figure plus the demodulator's SNR limit:
 *
 *     S = -174 dBm/Hz + 10log10(BW) + NF + SNR_limit(SF)
 *
 * Written this way rather than as the datasheet's six numbers because the noise
 * figure is a DESIGN VARIABLE — an LNA moves it — and a table cannot express
 * that. tools/frontier.py recovers NF = 6.03 dB from the SX1262's own published
 * column and the check below reproduces every one of its figures to 0.01 dB, so
 * this is the datasheet, extended, not a replacement for it. */
double sim_sens_nf(int sf, double nf_db)
{
    double snr_limit = -7.5 - 2.5 * (double)(sf - 7);
    return -174.0 + 10.0 * log10(125000.0) + nf_db + snr_limit;
}

/* The shipping receiver. Defined as the chip alone, so every number the bench
 * reports is what the hardware in the BOM does today; the LNA appears as an
 * explicit, priced step in the hardware ladder rather than as a silent bonus. */
double sim_sens_dbm(int sf)
{
    return sim_sens_nf(sf, SIM_NF_CHIP);
}

/* SX1262 transmit current against output power, datasheet typicals at 3.3 V
 * with the DC-DC enabled. Linear between the published points, which is close
 * enough for an energy budget and honest about being an interpolation. */
double sim_i_tx_ma(double dbm)
{
    const double p[] = { 10.0, 14.0, 17.0, 20.0, 22.0 };
    const double i[] = { 30.0, 45.0, 62.0, 90.0, 118.0 };
    if (dbm <= p[0]) return i[0];
    for (int k = 1; k < 5; k++)
        if (dbm <= p[k])
            return i[k-1] + (i[k] - i[k-1]) * (dbm - p[k-1]) / (p[k] - p[k-1]);
    return i[4];
}

static double lambda_m(void) { return SIM_C / SIM_FREQ_HZ; }

static double d_breakpoint_m(void)
{
    return 4.0 * SIM_PI * SIM_H_WRIST_M * SIM_H_WRIST_M / lambda_m();
}

static double fspl_db(double d_m)
{
    if (d_m < 1.0) d_m = 1.0;
    return 32.44 + 20.0 * log10(SIM_FREQ_HZ / 1e6) + 20.0 * log10(d_m / 1000.0);
}

double sim_path_loss_db(const sim_channel *c, double d_m)
{
    double dbp = d_breakpoint_m();
    if (d_m < 1.0) d_m = 1.0;
    if (d_m <= dbp) return fspl_db(d_m) + c->clutter_db;
    return fspl_db(dbp) + 40.0 * log10(d_m / dbp) + c->clutter_db;
}

double sim_dist_m(const sim_node *a, const sim_node *b)
{
    double dx = a->x - b->x, dy = a->y - b->y;
    return sqrt(dx * dx + dy * dy);
}

/* Received power at dst of a transmission from src. `extra_shadow` is drawn once
 * per transmission by the caller, so every receiver of the same frame sees the
 * same fade — shadowing is a property of the transmitter's surroundings, not a
 * per-link dice roll. */
double sim_rssi_dbm(const sim_world *w, int src, int dst, double extra_shadow)
{
    const sim_node *a = &w->n[src], *b = &w->n[dst];
    double g_tx = a->band ? SIM_G_BAND_DBI : SIM_G_CAPSULE_DBI;
    double g_rx = b->band ? SIM_G_BAND_DBI : SIM_G_CAPSULE_DBI;
    double d = sim_dist_m(a, b);
    return SIM_TX_DBM + g_tx + g_rx - sim_path_loss_db(&w->ch, d) - extra_shadow;
}

/* The inverse, for the self-test: the distance at which the link budget runs
 * out. This is budget.py's two_ray_range_m() written in C. */
double sim_range_at(const sim_channel *c, int sf, double g_dbi,
                    double tx_dbm, double nf_db)
{
    double budget = tx_dbm + g_dbi + g_dbi - sim_sens_nf(sf, nf_db) - SIM_MARGIN_DB;
    double dbp    = d_breakpoint_m();
    double excess = budget - c->clutter_db - fspl_db(dbp);
    if (excess <= 0.0) return dbp * pow(10.0, excess / 20.0);
    return dbp * pow(10.0, excess / 40.0);
}

double sim_range_m(const sim_channel *c, int sf, double g_dbi)
{
    double budget = SIM_TX_DBM + g_dbi + g_dbi - sim_sens_dbm(sf) - SIM_MARGIN_DB;
    double dbp    = d_breakpoint_m();
    double excess = budget - c->clutter_db - fspl_db(dbp);
    if (excess <= 0.0) return dbp * pow(10.0, excess / 20.0);
    return dbp * pow(10.0, excess / 40.0);
}
