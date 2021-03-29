#include <gpiod.h> 
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct led_t
{
	int line;
	int val;
	struct gpiod_line* line_fd;
} led_t;

typedef struct button_t
{
	int num;
	int led_len;
	led_t** my_leds;
} button_t;
	
static const char* g_controller = "gpiochip0";
static const char* g_consumer = "puzzle";
static struct gpiod_chip* g_gpio_chip;
static const long g_ignore_duplicate_event_ms = 2000L;
static long g_previous_event_time_ms = 0L;
static int g_previous_button = -1;

static struct led_t leds[] = { {24,0,NULL}, {25,0,NULL}, {26,0,NULL}, {27,0,NULL}, {28,0,NULL}, {29,0,NULL}, {30,0,NULL}, {31,0,NULL} }; 
unsigned const int leds_len = sizeof(leds) / sizeof(led_t);

static struct button_t buttons[] = { 
	{12, 0, NULL}, {13, 0, NULL}, {14, 0, NULL}, {15, 0, NULL}, 
	{16, 0, NULL}, {17, 0, NULL}, {18, 0, NULL}, {19, 0, NULL},
	{20, 0, NULL}, {21, 0, NULL}, {22, 0, NULL}, {23, 0, NULL}
};
unsigned const int button_len = sizeof(buttons) / sizeof(button_t);

static long get_time_millis()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (long) (ts.tv_sec*1e3 + ts.tv_nsec/1e6);
}

static void set_line(led_t* l, int value)
{
	if(gpiod_line_set_value(l->line_fd, value))
	{
		fprintf (stderr, "Couldn't set LED line %d - %s\n", l->line, strerror(errno));
		exit (EXIT_FAILURE);
	}
	l->val = value;
}

static void toggle_line(led_t* l)
{
	l->val = 1 - l->val;
	if(gpiod_line_set_value(l->line_fd, l->val))
	{
		fprintf (stderr, "Couldn't toggle LED line %d - %s\n", l->line, strerror(errno));
		exit (EXIT_FAILURE);
	}
}

static void toggle_my_leds(int id)
{	
	fprintf(stderr, "%d ", id);
	for(int i=0;i<button_len;++i)
	{
		button_t *btn = &buttons[i];
		if(btn->num == id)
		{
			for(int j=0; j < btn->led_len; ++j) {
				toggle_line(btn->my_leds[j]);
			}
			break;
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
		int random_led = rand() % leds_len;
		if (is_led_unique(btn->my_leds, &leds[random_led], i))
	       	{
			btn->my_leds[i] = &leds[random_led];
			++i;
		}
	}
}

void cycle_all_lines(int value)
{
	for(int i=0; i<leds_len;++i) {
		set_line(&leds[i], value);
	}
}

static void init_leds()
{
	for(int i=0; i<leds_len;++i) {
		leds[i].line_fd = gpiod_chip_get_line(g_gpio_chip, leds[i].line);
		gpiod_line_request_output(leds[i].line_fd, g_consumer, 0);
	}
}

static void init()
{
	g_gpio_chip = gpiod_chip_open_by_name(g_controller);
	init_leds();

	srand(time(0));

	// cycle all leds
	//
	cycle_all_lines(0);

	// assign leds to buttons
	for (int i = 0; i < button_len; ++i) {
		init_button(&buttons[i]);
	}

	// random button presses
	fprintf(stderr, "Cipher: ");
	for (int i = 0; i < (rand() % 3)+4; ++i) {
		int button_index = (rand() % 6)+12;
		toggle_my_leds(button_index);
	}
	fprintf(stderr, "\n");
}

static void on_edge_event(int event, unsigned int offset)
{
	const long tm_ms = get_time_millis();
	if ((tm_ms - g_previous_event_time_ms) < g_ignore_duplicate_event_ms && g_previous_button == offset)
	{
		fprintf(stderr, ".");
	}
	else
       	{
		g_previous_event_time_ms = tm_ms;
		g_previous_button = offset;
		toggle_my_leds(offset);
	}
}

// Edge detection event callback
// 
static int falling_edge_event_multiple_offsets(int event, unsigned int offset, const struct timespec *timestamp, void *unused)
{
	switch (event)
	{
		case GPIOD_CTXLESS_EVENT_CB_FALLING_EDGE:
			on_edge_event(event, offset);
			break;

		case GPIOD_CTXLESS_EVENT_CB_RISING_EDGE:
			on_edge_event(event, offset);
			break;

		case GPIOD_CTXLESS_EVENT_CB_TIMEOUT:
			break;
	}
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
