/* Standalone C11 host for HERUS Host JSONL v1. No Python or HERUS imports. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int rotated = 0, step_no = 0, mode = 0, level = 0;
static void response(const char *s) { puts(s); fflush(stdout); }
static int has(const char *line, const char *needle) { return strstr(line, needle) != NULL; }
static const char *action_delta(const char *line) {
  if (!rotated && has(line, "target_x")) return "mode";
  if (!rotated && has(line, "target_y")) return "level";
  if (rotated && has(line, "target_new")) return "mode";
  if (rotated && has(line, "target_level")) return "level";
  return NULL;
}
int main(void) {
  char line[1024];
  while (fgets(line, sizeof line, stdin)) {
    if (has(line, "\"op\"") && has(line, "ready")) { response("{\"protocol\":\"herus-host-jsonl-v1\",\"status\":\"OK\"}"); continue; }
    if (has(line, "\"op\"") && has(line, "describe")) {
      response(rotated ? "{\"actions\":[\"target_level\",\"target_new\"],\"protocol\":\"herus-host-jsonl-v1\",\"status\":\"OK\"}" : "{\"actions\":[\"target_x\",\"target_y\"],\"protocol\":\"herus-host-jsonl-v1\",\"status\":\"OK\"}"); continue;
    }
    if (has(line, "\"op\"") && has(line, "reset")) { mode=0; level=0; ++step_no; response("{\"status\":\"OK\"}"); continue; }
    if (has(line, "\"op\"") && has(line, "rotate")) { rotated=1; ++step_no; response("{\"status\":\"OK\"}"); continue; }
    if (has(line, "\"op\"") && has(line, "quit")) { response("{\"status\":\"OK\"}"); return 0; }
    if (has(line, "\"op\"") && has(line, "delay")) { double seconds=0.0; const char *p=strstr(line,"seconds"); if(p) sscanf(p,"seconds%*[^0-9]%lf",&seconds); if(seconds>0) { volatile unsigned long n=(unsigned long)(seconds*30000000.0); while(n--){} } response("{\"status\":\"OK\"}"); continue; }
    if (has(line, "{not-json")) { response("{\"reason\":\"invalid_json\",\"status\":\"ERROR\"}"); continue; }
    if (has(line, "\"op\"") && has(line, "probe")) {
      const char *delta=action_delta(line); int before_mode=mode, before_level=level; ++step_no;
      if (delta && strcmp(delta, "mode") == 0) {
        mode = 1;
      }
      if (delta && strcmp(delta, "level") == 0) {
        level = 1;
      }
      char out[512]; const char *action = strstr(line,"target_new") ? "target_new" : strstr(line,"target_level") ? "target_level" : strstr(line,"target_x") ? "target_x" : "target_y";
      snprintf(out,sizeof out,"{\"after\":{\"level\":%d,\"mode\":%d},\"before\":{\"level\":%d,\"mode\":%d},\"context\":{\"zone\":1},\"risk\":%d,\"status\":\"OK\",\"step\":%d}",level,mode,before_level,before_mode,delta?0:1,step_no); (void)action; response(out); continue;
    }
    response("{\"reason\":\"unknown_operation\",\"status\":\"ERROR\"}");
  }
  return 0;
}
