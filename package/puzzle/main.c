#include <gpiod.h> 
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

typedef struct led_t
{
	int line;
	int val;
} led_t;

typedef struct button_t
{
	int num;
	int led_len;
	led_t** my_leds;
} button_t;
	
static const char* g_controller = "gpiochip0";
static const char* g_consumer = "puzzle";

static struct led_t leds[] = { {24,0}, {25,0}, {26,0}, {27,0}, {28,0}, {29,0}, {30,0}, {31,0} }; 
static struct button_t buttons[] = { {12, 0, NULL}, {13, 0, NULL}, {14, 0, NULL}, {15, 0, NULL}, {16, 0, NULL}, {17, 0, NULL} };
unsigned const int button_len = 6;

static void toggle_line(led_t* l)
{
	if(gpiod_ctxless_set_value(g_controller, l->line, l->val, false, g_consumer,  NULL, NULL))
	{
		fprintf (stderr, "Couldn't toggle LED line %d - %s\n", l->line, strerror(errno));
		exit (EXIT_FAILURE);
	}
	l->val = 1 - l->val;
}

static void toggle_my_leds(int id)
{	
	for(int i=0;i<button_len;++i)
	{
		button_t *btn = &buttons[i];
		if(btn->num == id)
		{
			for(int j=0; j < btn->led_len; ++j) {
				toggle_line(btn->my_leds[j]);
			}

		}
	}
}

static int is_led_unique(led_t** btn_leds, led_t *candidate_led, int max_leds_index)
{
	for (int i = 0; i < max_leds_index; ++i)
	{
		if (candidate_led == btn_leds[i])
			return 0;
	}
	return 1;
}

static void init_button(button_t *btn)
{
	btn->led_len = (rand() % 2)+1; // 1 or 2 leds per button
	btn->my_leds = malloc(sizeof(led_t*) * btn->led_len);
	for (int i = 0; i < btn->led_len; )
	{
		int random_led = rand() % 7;
		if (is_led_unique(btn->my_leds, &leds[random_led], i))
	       	{
			btn->my_leds[i] = &leds[random_led];
			++i;
		}
	}
}

static void init()
{
	srand(time(0));
	unsigned const int len = sizeof(leds) / sizeof(led_t);

	// reset all leds
	for(int i=0; i<len;++i) {
		toggle_line(&leds[i]);
	}

	// assign leds to buttons
	for (int i = 0; i < button_len; ++i) {
		init_button(&buttons[i]);
	}

	// random button presses
	fprintf(stderr, "Buttons: ");
	for (int i = 0; i < (rand() % 6)+5; ++i) {
		int button_index = (rand() % 6)+12;
		toggle_my_leds(button_index);
		fprintf(stderr, "%d ", button_index);
	}
	fprintf(stderr, "\n");
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
			break;
		case GPIOD_CTXLESS_EVENT_CB_RISING_EDGE:
			edge_event = " Rising edge";
			// HERE is made the toggling.
			toggle_my_leds(offset);
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
	
	init();

	unsigned int  offsets[]  = { 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
	unsigned int* p_offsets  = &offsets[0];   
	unsigned int  lines      = sizeof(offsets) / sizeof(offsets[0]);

	if (gpiod_ctxless_event_monitor_multiple(g_controller, GPIOD_CTXLESS_EVENT_BOTH_EDGES, p_offsets, lines, 0, 
		NULL, NULL, NULL, falling_edge_event_multiple_offsets, NULL))
	{
		fprintf (stderr, "Couldn't open controller %s - %s\n", g_controller, strerror(errno));
		exit (EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
