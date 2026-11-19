#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
	#include <windows.h>
	#include <io.h>
#else
	#include <unistd.h>
#endif

#include "timey.h"
#include "vibrant_logs.h"

// the max number of delayed log messages
#define MAX_DELAY_MSGS 64
// enough chars to fit the time string in VibrantLogs ('[HH:MM:SS]') (includes null terminator)
#define TIME_STR_LEN 11
//enough chars to fit the longest possible date string in VibrantLogs ('[MM-DD-YYYY]' or '[MONTH-DD-YYYY]') (includes null terminator)
#define DATE_STR_LEN 30
// the starting size of initial buffers used when logging and printing messages
#define INIT_MSG_LEN 64

typedef struct delayed_msg
{
	// the message to print
	char *msg;
	// the amount of seconds before the message prints
	double remaining_time_sec;
	// the log type
	vl_type type;
	// should the message label be printed?
	bool print_label;
} delayed_msg;

typedef struct scheduled_msg
{
	// date-time
	timey_datetime dt;
	// time stamp
	timey_timestamp ts;
	// the message to print
	char *msg;
	// log type
	vl_type type;
	// if this msg was scheduled using a timestamp only
	bool scheduled_ts;
	// should the message label be printed?
	bool print_label;
} scheduled_msg;

// storage of all delayed logs:
static delayed_msg delayed_msgs[MAX_DELAY_MSGS];
static size_t delayed_msg_count = 0;

// storage of all scheduled logs:
static scheduled_msg scheduled_msgs[MAX_DELAY_MSGS];
static size_t scheduled_msg_count = 0;

static bool init = false;
static vl_config config = {0};

// macro for quickly obtaining the RGB value of a color (used when printing to shorten lines)
#define rgb(color) color.r, color.g, color.b

// determine if config.output_destination is a file/terminal (use colors in terminal only)
static bool can_use_colors(void)
{
	#ifdef _WIN32
		DWORD mode;
		HANDLE h = (HANDLE) _get_osfhandle(_fileno(config.output_destination));

		if(h == INVALID_HANDLE_VALUE)
			return false;

		return GetConsoleMode(h, &mode) != 0;
	#else
		return isatty(fileno(config.output_destination));
	#endif
}

static void get_fmt_str_buf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args = NULL;
	va_start(args, fmt);
	if(!args)
		return;
	vsnprintf(buf, size, fmt, args);
	va_end(args);
}
static char *get_fmt_str_auto_realloc_args(char **buf, size_t *size, const char *fmt, va_list args)
{
	if(!args)
		return NULL;

	va_list args_copy = NULL;
	va_copy(args_copy, args);

	int req_len = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	if(req_len < 0)
	{
		va_end(args_copy);
		return NULL;
	}

	if(req_len + 1 >= *size)
	{
		char *new_buf = realloc(*buf, req_len + 1);
		if(!new_buf)
		{
			va_end(args_copy);
			return NULL;
		}
		*buf = new_buf;
		*size = req_len + 1;
	}

	vsnprintf(*buf, *size, fmt, args_copy);
	va_end(args_copy);
	return *buf;
}
static char *get_fmt_str_auto_realloc(char **buf, size_t *size, const char *fmt, ...)
{
	va_list args = NULL;
	va_start(args, fmt);
	return get_fmt_str_auto_realloc_args(buf, size, fmt, args);
}

// print a message:
static int print_msg(vl_type type, const char *msg, bool print_label, bool print_time, bool print_date, bool print_full_day_name, bool print_abbrev_day_name, bool print_full_month_name, bool print_abbrev_month_name)
{
	// see if config.output_destination is NULL, indicating vl_init() was never called
	if(!config.output_destination)
	{
		// no check for 'use_colors' here since printf uses stdout by default
		if(print_label)
			printf("[ ERROR ] Output destination is NULL! Cannot log a message!\n");
		else
			printf("Output destination is NULL! Cannot log a message!\n");
		return 0;
	}

	// check for log level and the type given
	if(type == VL_INFO && (config.log_level == VL_WARNING || config.log_level == VL_ERROR))
		return 1;
	if(type == VL_WARNING && config.log_level == VL_ERROR)
		return 1;

	bool use_colors = can_use_colors();

	// if 'print_time' is true, obtain and format time string:
	if(print_time)
	{
		// obtain the time string
		char time_buf[TIME_STR_LEN];
		timey_query_time(time_buf, sizeof(time_buf));

		// obtain formatted time string
		char time_str[TIME_STR_LEN + 21]; // time string (11 chars) + format string (21 chars) = 32
		if(use_colors)
			get_fmt_str_buf(time_str, sizeof(time_str), "\x1b[38;2;%d;%d;%dm%s ", rgb(config.colors.time_color), time_buf);
		else
			get_fmt_str_buf(time_str, sizeof(time_str), "%s ", time_buf);

		// write time string to output
		fwrite(time_str, sizeof(char), strlen(time_str), config.output_destination);
	}

	// if 'print_date' is true, obtain and format date string:
	if(print_date)
	{
		// obtain the date string
		char date_buf[DATE_STR_LEN];
		timey_query_date(date_buf, sizeof(date_buf));

		// now 'date_buf' holds the 'mm-dd-yyyy' string

		/*
		   but if the month name is supposed to be printed,
		   print it like 'name-dd-yyyy' instead

		   note that the if statement checks for either month name being used
		*/
		if(print_full_month_name || print_abbrev_month_name)
		{
			// the first two chars in 'date_buf' will be equal to the month number
			char month_buf[3];
			month_buf[0] = date_buf[0];
			month_buf[1] = date_buf[1];
			month_buf[2] = '\0';

			// convert month number to an integer
			char *end_ptr = NULL;
			int month_num = strtol(month_buf, &end_ptr, 10);
			// if no numbers parsed, return
			if(end_ptr == month_buf)
				return 0;

			// now get the corresponding month name
			char month_name[10];
			timey_query_month_name(month_name, sizeof(month_name), month_num, print_abbrev_month_name);

			size_t month_name_len = strlen(month_name);

			// insert month name into 'date_buf' where the 'mm' string already is:

			// first, shift everything to the right to make room for the month name
			memmove(date_buf + month_name_len, date_buf + 2, strlen(date_buf + 2) + 1);
			// then copy the month name into the string
			memcpy(date_buf, month_name, month_name_len);
		}

		// now see if the day name should be inserted at the beginning of the date
		if(print_full_day_name || print_abbrev_day_name)
		{
			/*
			   because the date string could be in 'name-dd-yyyy' format now,
			   to parse the 'dd' part, first move to the first '-' character in the string
			*/
			const char *day_num_start = date_buf;

			while(*day_num_start && *day_num_start != '-')
				day_num_start++;

			// skip the '-'
			day_num_start++;

			// now parse the 'dd' part of the string
			char day_buf[3];
			day_buf[0] = *day_num_start;
			day_buf[1] = *(day_num_start + 1);
			day_buf[2] = '\0';

			// convert day number to an integer
			char *end_ptr = NULL;
			int day_num = strtol(day_buf, &end_ptr, 10);
			if(end_ptr == day_buf)
				return 0;

			// now get the corresponding day name
			char day_name[11]; // buffer will be 11 bytes to account for the ' ' character between the day and date
			timey_query_day_name(day_name, sizeof(day_name), day_num, false, print_abbrev_day_name);
			// insert space into buffer
			size_t end_of_day_name = strlen(day_name);
			day_name[end_of_day_name] = ' ';
			day_name[end_of_day_name + 1] = '\0';

			size_t day_name_len = strlen(day_name);

			// insert day name into the start of 'date_buf':

			// first shift everything to the right to make room for the day name
			memmove(date_buf + day_name_len, date_buf, strlen(date_buf) + 1);
			// then copy the day name into the string
			memcpy(date_buf, day_name, day_name_len);
		}

		// obtain formatted date string
		char date_str[DATE_STR_LEN + 21]; // date string (20 chars) + format string (21 chars) = 41
		if(use_colors)
			get_fmt_str_buf(date_str, sizeof(date_str), "\x1b[38;2;%d;%d;%dm%s ", rgb(config.colors.date_color), date_buf);
		else
			get_fmt_str_buf(date_str, sizeof(date_str), "%s ", date_buf);

		// write date string to output
		fwrite(date_str, sizeof(char), strlen(date_str), config.output_destination);
	}

	// buffer holding final log message (start out with 56 bytes to fit color codes, prefixes, and null terminator)
	size_t final_str_size = 56;
	char *final_str = calloc(final_str_size, sizeof(char));
	if(!final_str)
	{
		// no check for 'use_colors' here since printf uses stdout by default
		if(print_label)
			printf("\x1b[38;2;%d;%d;%dm[ ERROR ] \x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.error_prefix_color), rgb(config.colors.error_color), "Failed to allocate memory for VibrantLogs message!\n");
		else
			printf("\x1b[38;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.error_prefix_color), rgb(config.colors.error_color), "Failed to allocate memory for VibrantLogs message!\n");
		return 0;
	}

	switch(type)
	{
		case VL_INFO:
			if(print_label)
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm[ INFO ] \x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.info_prefix_color), rgb(config.colors.info_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "[ INFO ] %s", msg);
			}
			else
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.info_prefix_color), rgb(config.colors.info_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "%s", msg);
			}
			break;
		case VL_SUCCESS:
			if(print_label)
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm[ SUCCESS ] \x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.success_prefix_color), rgb(config.colors.success_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "[ SUCCESS ] %s", msg);
			}
			else
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.success_prefix_color), rgb(config.colors.success_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "%s", msg);
			}
			break;
		case VL_DEBUG:
			if(print_label)
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm[ DEBUG ] \x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.debug_prefix_color), rgb(config.colors.debug_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "[ DEBUG ] %s", msg);
			}
			else
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.debug_prefix_color), rgb(config.colors.debug_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "%s", msg);
			}
			break;
		case VL_WARNING:
			if(print_label)
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm[ WARNING ] \x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.warning_prefix_color), rgb(config.colors.warning_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "[ WARNING ] %s", msg);
			}
			else
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.warning_prefix_color), rgb(config.colors.warning_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "%s", msg);
			}
			break;
		case VL_ERROR:
		default:
			if(print_label)
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm[ ERROR ] \x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.error_prefix_color), rgb(config.colors.error_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "[ ERROR ] %s", msg);
			}
			else
			{
				if(use_colors)
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "\x1b[38;2;%d;%d;%dm\x1b[38;2;%d;%d;%dm%s\x1b[0m", rgb(config.colors.error_prefix_color), rgb(config.colors.error_color), msg);
				else
					get_fmt_str_auto_realloc(&final_str, &final_str_size, "%s", msg);
			}
			break;
	}

	// write final string to output
	fwrite(final_str, sizeof(char), strlen(final_str), config.output_destination);

	free(final_str);

	return 1;
}

int vl_init()
{
	if(init)
		return 0;

	// default config:
	config.colors = VL_COLOR_SCHEME_DEFAULT;
	config.output_destination = stdout;
	config.log_level = VL_INFO;
	config.print_time = true;
	config.print_date = false;
	config.print_full_day_name = false;
	config.print_abbrev_day_name = false;
	config.print_full_month_name = false;
	config.print_abbrev_month_name = false;

	init = true;

	return 1;
}
bool vl_is_init()
{
	return init;
}
int vl_shutdown()
{
	if(!init)
		return 0;

	init = false;

	// free the rest of the messages that haven't been freed yet in vl_update(...)
	for(size_t i = 0; i < delayed_msg_count; ++i)
	{
		delayed_msg *msg = &delayed_msgs[i];
		if(msg->msg)
		{
			free(msg->msg);
			msg->msg = NULL;
		}
	}
	for(size_t i = 0; i < scheduled_msg_count; ++i)
	{
		scheduled_msg *msg = &scheduled_msgs[i];
		if(msg->msg)
		{
			free(msg->msg);
			msg->msg = NULL;
		}
	}

	delayed_msg_count = 0;
	scheduled_msg_count = 0;

	vl_log(VL_SUCCESS, "VibrantLogs successfully shutdown!\n");

	return 1;
}

void vl_use_config(vl_config cfg)
{
	config = cfg;
}
vl_config *vl_curr_config()
{
	return &config;
}

int vl_log(vl_type log_type, const char *fmt, ...)
{
	if(!fmt)
		return 0;

	size_t buf_size = INIT_MSG_LEN;
	char *buf = calloc(buf_size, sizeof(char));
	if(!buf)
		return 0;

	va_list args = NULL;
	va_start(args, fmt);
	if(!get_fmt_str_auto_realloc_args(&buf, &buf_size, fmt, args))
		return 0;

	int success = print_msg(log_type, buf, true, config.print_time, config.print_date, config.print_full_day_name, config.print_abbrev_day_name, config.print_full_month_name, config.print_abbrev_month_name);

	free(buf);

	return success;
}
int vl_print(vl_type log_type, const char *fmt, ...)
{
	if(!fmt)
		return 0;

	size_t buf_size = INIT_MSG_LEN;
	char *buf = calloc(buf_size, sizeof(char));
	if(!buf)
		return 0;

	va_list args = NULL;
	va_start(args, fmt);
	if(!get_fmt_str_auto_realloc_args(&buf, &buf_size, fmt, args))
		return 0;

	int success = print_msg(log_type, buf, false, false, false, false, false, false, false);

	free(buf);

	return success;
}

static int delay_log(vl_type log_type, double seconds, const char *fmt, va_list args, delayed_msg **out_msg)
{
	if(!init || seconds < 0.0 || delayed_msg_count >= MAX_DELAY_MSGS)
		return 0;

	size_t buf_size = INIT_MSG_LEN;
	char *buf = calloc(buf_size, sizeof(char));
	if(!buf)
		return 0;

	if(!get_fmt_str_auto_realloc_args(&buf, &buf_size, fmt, args))
		return 0;

	// search for duplicate message first
	for(size_t i = 0; i < delayed_msg_count; ++i)
	{
		delayed_msg *existing = &delayed_msgs[i];
		if(existing->type == log_type && strcmp(existing->msg, buf) == 0)
			return 0;
	}

	delayed_msg *msg = &delayed_msgs[delayed_msg_count];
	msg->type = log_type;
	msg->remaining_time_sec = seconds;
	msg->print_label = true;
	msg->msg = buf;

	// TODO does the '\0' have to be inserted at end of msg here?

	if(out_msg)
		*out_msg = msg;

	delayed_msg_count++;

	return 1;
}
int vl_delay_log(vl_type log_type, double seconds, const char *fmt, ...)
{
	if(!fmt)
		return 0;

	va_list args = NULL;
	va_start(args, fmt);

	int success = delay_log(log_type, seconds, fmt, args, NULL);
	va_end(args);

	return success;
}
int vl_delay_print(vl_type log_type, double seconds, const char *fmt, ...)
{
	if(!fmt)
		return 0;

	va_list args = NULL;
	va_start(args, fmt);

	delayed_msg *msg = NULL;
	int success = delay_log(log_type, seconds, fmt, args, &msg);
	va_end(args);

	if(success)
		msg->print_label = true;

	return 1;
}

static int schedule_log_dt(vl_type log_type, timey_datetime *dt, const char *fmt, va_list args, scheduled_msg **out_msg)
{
	if(!init || scheduled_msg_count >= MAX_DELAY_MSGS)
		return 0;

	size_t buf_size = INIT_MSG_LEN;
	char *buf = calloc(buf_size, sizeof(char));
	if(!buf)
		return 0;

	if(!get_fmt_str_auto_realloc_args(&buf, &buf_size, fmt, args))
		return 0;

	// search for duplicate messages first
	for(size_t i = 0; i < scheduled_msg_count; ++i)
	{
		scheduled_msg *existing = &scheduled_msgs[i];
		if(existing->type == log_type && strcmp(existing->msg, buf) == 0)
			return 0;
	}

	scheduled_msg *msg = &scheduled_msgs[scheduled_msg_count];
	msg->type = log_type;
	if(dt)
		msg->dt = *dt;
	msg->ts = (timey_timestamp) {0};
	msg->scheduled_ts = false;
	msg->print_label = true;
	msg->msg = buf;

	// TODO does the '\0' have to be inserted at end of msg here?

	if(out_msg)
		*out_msg = msg;

	scheduled_msg_count++;

	return 1;
}
static int schedule_log_ts(vl_type log_type, timey_timestamp *ts, const char *fmt, va_list args, scheduled_msg **out_msg)
{
	if(!ts)
		return 0;

	scheduled_msg *msg = NULL;
	int success = schedule_log_dt(log_type, NULL, fmt, args, &msg);

	if(success)
	{
		msg->dt = (timey_datetime) {0};
		msg->ts = *ts;
		msg->scheduled_ts = true;
	}

	return success;
}

int vl_schedule_log_ts(vl_type log_type, timey_timestamp *ts, const char *fmt, ...)
{
	if(!fmt)
		return 0;

	va_list args = NULL;
	va_start(args, fmt);

	int success = schedule_log_ts(log_type, ts, fmt, args, NULL);
	va_end(args);

	return success;
}
int vl_schedule_print_ts(vl_type log_type, struct timey_timestamp *ts, const char *fmt, ...)
{
	if(!fmt)
		return 0;

	va_list args = NULL;
	va_start(args, fmt);

	scheduled_msg *msg = NULL;
	int success = schedule_log_ts(log_type, ts, fmt, args, &msg);
	va_end(args);

	if(success)
		msg->print_label = false;

	return success;
}
int vl_schedule_log_dt(vl_type log_type, timey_datetime *dt, const char *fmt, ...)
{
	if(!fmt)
		return 0;

	va_list args = NULL;
	va_start(args, fmt);

	int success = schedule_log_dt(log_type, dt, fmt, args, NULL);
	va_end(args);

	return success;
}
int vl_schedule_print_dt(vl_type log_type, timey_datetime *dt, const char *fmt, ...)
{
	if(!fmt)
		return 0;

	va_list args = NULL;
	va_start(args, fmt);

	scheduled_msg *msg = NULL;
	int success = schedule_log_dt(log_type, dt, fmt, args, &msg);
	va_end(args);

	// do not print label on the message
	if(success)
		msg->print_label = false;

	return success;
}

void vl_update(double delta_time)
{
	// run through delayed messages:
	for(size_t i = 0; i < delayed_msg_count;)
	{
		// get current message to work on
		delayed_msg *msg = &delayed_msgs[i];

		// update timer
		msg->remaining_time_sec -= delta_time;

		// check to see if that message's time has passed
		if(msg->remaining_time_sec <= 0.0)
		{
			print_msg(msg->type, msg->msg, msg->print_label, config.print_time, config.print_date, config.print_full_day_name, config.print_abbrev_day_name, config.print_full_month_name, config.print_abbrev_month_name);

			// because the delay_log functions allocate memory for the message, it must be freed here after it has been printed
			free(msg->msg);
			msg->msg = NULL;

			// remove this msg
			size_t dest_pos = i;
			size_t src_pos = i + 1;
			// see if this msg is the last one in the array
			if(src_pos >= MAX_DELAY_MSGS)
				// if so, just decrement count
				delayed_msg_count--;
			else
			{
				// otherwise, shift everything after this msg to the left
				memmove(&delayed_msgs[i], &delayed_msgs[i + 1], (delayed_msg_count - i - 1) * sizeof(*delayed_msgs));
				delayed_msg_count--;
			}
		}
		else
			i++;
	}

	// run through scheduled messages:
	for(size_t i = 0; i < scheduled_msg_count;)
	{
		// get current timestamp and datetime
		timey_timestamp now_ts = timey_curr_timestamp();
		timey_datetime now_dt = timey_curr_datetime();

		// get current message to work on
		scheduled_msg *msg = &scheduled_msgs[i];

		// check its timestamp or datetime vs. now_ts and now_dt
		if(msg->scheduled_ts)
		{
			// handle timestamp only
			if(timey_timestamps_are_equal(&msg->ts, &now_ts))
			{
				print_msg(msg->type, msg->msg, msg->print_label, config.print_time, config.print_date, config.print_full_day_name, config.print_abbrev_day_name, config.print_full_month_name, config.print_abbrev_month_name);

				// because the schedule_log functions allocate memory for the message, it must be freed here after it has been printed
				free(msg->msg);
				msg->msg = NULL;

				// use same logic from delayed msg removal above:
				size_t dest_pos = i;
				size_t src_pos = i + 1;
				if(src_pos >= MAX_DELAY_MSGS)
					scheduled_msg_count--;
				else
				{
					memmove(&scheduled_msgs[i], &scheduled_msgs[i + 1], (scheduled_msg_count - i - 1) * sizeof(*scheduled_msgs));
					scheduled_msg_count--;
				}
			}
			else
				i++;
		}
		else
		{
			// handle datetime

			if(timey_datetimes_are_equal(&msg->dt, &now_dt))
			{
				print_msg(msg->type, msg->msg, true, config.print_time, config.print_date, config.print_full_day_name, config.print_abbrev_day_name, config.print_full_month_name, config.print_abbrev_month_name);

				// because the schedule_log functions allocate memory for the message, it must be freed here after it has been printed
				free(msg->msg);
				msg->msg = NULL;

				// use same logic from delayed msg removal above:
				size_t dest_pos = i;
				size_t src_pos = i + 1;
				if(src_pos >= MAX_DELAY_MSGS)
					scheduled_msg_count--;
				else
				{
					memmove(&scheduled_msgs[i], &scheduled_msgs[i + 1], (scheduled_msg_count - i - 1) * sizeof(*scheduled_msgs));
					scheduled_msg_count--;
				}
			}
			else
				i++;
		}
	}
}
