#include <gpiod.h> 
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>

static const char* g_controller = "gpiochip0";
static const char* g_consumer = "puzzle";
static int g_line_1_value = 1;

static void toggle_line(int line)
{
	gpiod_ctxless_set_value(g_controller, line, g_line_1_value, false, g_consumer,  NULL, NULL);
	g_line_1_value = 1 - g_line_1_value;
}

// Edge detection event callback
// 
static int falling_edge_event_multiple_offsets(int event, unsigned int offset, const struct timespec *timestamp, void *unused)
{
	struct timespec my_ts;
	struct tm*      my_tm;
	char*           edge_event;
	clock_gettime(CLOCK_REALTIME, &my_ts);
	my_tm = localtime(&my_ts.tv_sec);
	switch (event)
	{
		case GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE:
			edge_event = "Falling edge";
			toggle_line(27);
			break;
		case GPIOD_CTXLESS_EVENT_CB_RISING_EDGE:
			edge_event = " Rising edge";
			break;
		case GPIOD_CTXLESS_EVENT_CB_TIMEOUT:
			edge_event = "    Timed out";
			break;
	}
	fprintf(stderr, "%02d:%02d:%02d|%s, line: %d\n", 
		my_tm->tm_hour, my_tm->tm_min, my_tm->tm_sec, edge_event, offset);
	return GPIOD_CTXLESS_EVENT_CB_RET_OK;	 
}

int main(int argc, char** argv)
{
	fprintf(stderr, "Monitoring GPIO falling/rising edge events.\n");

	unsigned int  offsets[]  = { 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
	unsigned int* p_offsets  = &offsets[0];   
	unsigned int  lines      = sizeof(offsets) / sizeof(offsets[0]);

	gpiod_ctxless_event_monitor_multiple(g_controller, GPIOD_CTXLESS_EVENT_BOTH_EDGES, p_offsets, lines, 0, 
		NULL, NULL, NULL, falling_edge_event_multiple_offsets, NULL);

	return EXIT_SUCCESS;
   return 0;
}
