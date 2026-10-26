#pragma once

#include <stdio.h>

/**
  Represents the type of a log statement.
*/
typedef enum vl_type
{
	/**
	  Used for normal log messages.
	*/
	VL_INFO,
	/**
	  Used to signal that an operation
	  was successful.
	*/
	VL_SUCCESS,
	/**
	  Used to debug a value or expression.
	*/
	VL_DEBUG,
	/**
	  Used to send a warning message.
	*/
	VL_WARNING,
	/**
	  Used to send an error message.
	*/
	VL_ERROR
} vl_type;

/**
  Represents a color using values of 0 to 255.
*/
typedef struct vl_color
{
	/**
	  The red value.
	*/
	unsigned char r;
	/**
	  The green value.
	*/
	unsigned char g;
	/**
	  The blue value.
	*/
	unsigned char b;
} vl_color;

/**
  White.
*/
#define VL_WHITE (vl_color) { 255, 255, 255 }
/**
  Black.
*/
#define VL_BLACK (vl_color) { 0, 0, 0 }
/**
  Gray.
*/
#define VL_GRAY (vl_color) { 125, 125, 125 }
/**
  Light gray.
*/
#define VL_LIGHT_GRAY (vl_color) { 175, 175, 175 }
/**
  Light blue.
*/
#define VL_LIGHT_BLUE (vl_color) { 0, 125, 215 }
/**
  Blue.
*/
#define VL_BLUE (vl_color) { 0, 255, 255 }
/**
  Light green.
*/
#define VL_LIGHT_GREEN (vl_color) { 0, 175, 0 }
/**
  Green.
*/
#define VL_GREEN (vl_color) { 0, 255, 0 }
/**
  Light orange.
*/
#define VL_LIGHT_ORANGE (vl_color) { 200, 125, 0 }
/**
  Orange.
*/
#define VL_ORANGE (vl_color) { 255, 150, 0 }
/**
  Light yellow.
*/
#define VL_LIGHT_YELLOW (vl_color) { 200, 200, 0 }
/**
  Yellow.
*/
#define VL_YELLOW (vl_color) { 255, 255, 0 }
/**
  Light red.
*/
#define VL_LIGHT_RED (vl_color) { 200, 0, 0 }
/**
  Red.
*/
#define VL_RED (vl_color) { 255, 0, 0 }
/**
  Light pink.
*/
#define VL_LIGHT_PINK (vl_color) { 220, 22, 120 }
/**
  Pink.
*/
#define VL_PINK (vl_color) { 255, 0, 130 }

/**
  Represents the color scheme of VibrantLogs.

  @see VL_COLOR_SCHEME_DEFAULT
*/
typedef struct vl_color_scheme
{
	/**
	  The color of the time (if printed).

	  @see vl_config.
	  @see vl_use_config(vl_config).
	*/
	vl_color time_color;
	/**
	  The color of the date (if printed).

	  @see vl_config.
	  @see vl_use_config(vl_config).
	*/
	vl_color date_color;
	/**
	  The color of the prefix of info messages.
	*/
	vl_color info_prefix_color;
	/**
	  The main color of info messages.
	*/
	vl_color info_color;
	/**
	  The color of the prefix of successful operation messages.
	*/
	vl_color success_prefix_color;
	/**
	  The main color of successful operation messages.
	*/
	vl_color success_color;
	/**
	  The color of the prefix of debug messages.
	*/
	vl_color debug_prefix_color;
	/**
	  The main color of debug messages.
	*/
	vl_color debug_color;
	/**
	  The color of the prefix of warning messages.
	*/
	vl_color warning_prefix_color;
	/**
	  The main color of warning messages.
	*/
	vl_color warning_color;
	/**
	  The color of the prefix of error messages.
	*/
	vl_color error_prefix_color;
	/**
	  The main color of error messages.
	*/
	vl_color error_color;
} vl_color_scheme;

/**
  The default color scheme of VibrantLogs.
*/
#define VL_COLOR_SCHEME_DEFAULT (vl_color_scheme) { \
		.time_color = VL_WHITE, \
		.date_color = (vl_color) { 190, 190, 190 }, \
		.info_prefix_color = VL_LIGHT_BLUE, \
		.info_color = VL_BLUE, \
		.success_prefix_color = VL_LIGHT_GREEN, \
		.success_color = VL_GREEN, \
		.debug_prefix_color = VL_LIGHT_ORANGE, \
		.debug_color = VL_ORANGE, \
		.warning_prefix_color = VL_LIGHT_YELLOW, \
		.warning_color = VL_YELLOW, \
		.error_prefix_color = VL_LIGHT_RED, \
		.error_color = VL_RED \
	};
/**
  A dimmer version of the default color scheme.
*/
#define VL_COLOR_SCHEME_DEFAULT_DIM (vl_color_scheme) { \
		.time_color = VL_LIGHT_GRAY, \
		.date_color = (vl_color) { 150, 150, 150 }, \
		.info_prefix_color = (vl_color) { 25, 115, 180 }, \
		.info_color = (vl_color) { 25, 200, 200 }, \
		.success_prefix_color = (vl_color) { 15, 150, 15 }, \
		.success_color = (vl_color) { 30, 200, 30 }, \
		.debug_prefix_color = (vl_color) { 175, 115, 15 }, \
		.debug_color = (vl_color) { 190, 135, 30 }, \
		.warning_prefix_color = (vl_color) { 170, 170, 20 }, \
		.warning_color = (vl_color) { 200, 200, 30 }, \
		.error_prefix_color = (vl_color) { 175, 15, 15 }, \
		.error_color = (vl_color) { 200, 25, 25 } \
	};

/**
  The Candy Land color scheme.
*/
#define VL_COLOR_SCHEME_CANDY_LAND (vl_color_scheme) { \
		.time_color = (vl_color) { 255, 0, 100 }, \
		.date_color = VL_PINK, \
		.info_prefix_color = (vl_color) { 75, 150, 225 }, \
		.info_color = (vl_color) { 0, 125, 255 }, \
		.success_prefix_color = (vl_color) { 35, 210, 100 }, \
		.success_color = (vl_color) { 0, 255, 100 }, \
		.debug_prefix_color = (vl_color) { 200, 170, 50 }, \
		.debug_color = (vl_color) { 225, 180, 15 }, \
		.warning_prefix_color = (vl_color) { 225, 145, 75 }, \
		.warning_color = (vl_color) { 255, 130, 0 }, \
		.error_prefix_color = (vl_color) { 100, 45, 150 }, \
		.error_color = (vl_color) { 140, 0, 255 } \
	};
/**
  The Cloudy color scheme.
*/
#define VL_COLOR_SCHEME_CLOUDY (vl_color_scheme) { \
		.time_color = (vl_color) { 190, 190, 190 }, \
		.date_color = (vl_color) { 210, 210, 245 }, \
		.info_prefix_color = (vl_color) { 60, 100, 150 }, \
		.info_color = (vl_color) { 35, 115, 190 }, \
		.success_prefix_color = (vl_color) { 75, 190, 85 }, \
		.success_color = (vl_color) { 60, 255, 75 }, \
		.debug_prefix_color = (vl_color) { 60, 160, 175 }, \
		.debug_color = (vl_color) { 20, 190, 210 }, \
		.warning_prefix_color = VL_LIGHT_YELLOW, \
		.warning_color = (vl_color) VL_YELLOW, \
		.error_prefix_color = (vl_color) { 185, 125, 55 }, \
		.error_color = (vl_color) { 210, 125, 25 } \
	};
/**
  The Monotone color scheme.
*/
#define VL_COLOR_SCHEME_MONOTONE (vl_color_scheme) { \
		.time_color = VL_LIGHT_GRAY, \
		.date_color = (vl_color) { 190, 190, 190 }, \
		.info_prefix_color = VL_GRAY, \
		.info_color = VL_WHITE, \
		.success_prefix_color = VL_GRAY, \
		.success_color = VL_WHITE, \
		.debug_prefix_color = VL_GRAY, \
		.debug_color = VL_WHITE, \
		.warning_prefix_color = VL_GRAY, \
		.warning_color = VL_WHITE, \
		.error_prefix_color = VL_GRAY, \
		.error_color = VL_WHITE \
	};
/**
  The Wizard color scheme.
*/
#define VL_COLOR_SCHEME_WIZARD (vl_color_scheme) { \
		.time_color = (vl_color) { 110, 145, 210 }, \
		.date_color = (vl_color) { 100, 145, 255 }, \
		.info_prefix_color = (vl_color) { 130, 100, 30 }, \
		.info_color = (vl_color) { 170, 115, 10 }, \
		.success_prefix_color = (vl_color) { 170, 170, 50 }, \
		.success_color = (vl_color) { 200, 200, 15 }, \
		.debug_prefix_color = (vl_color) { 40, 160, 170 }, \
		.debug_color = (vl_color) { 20, 190, 200 }, \
		.warning_prefix_color = (vl_color) { 100, 150, 50 }, \
		.warning_color = (vl_color) { 110, 180, 30 }, \
		.error_prefix_color = (vl_color) { 160, 60, 60 }, \
		.error_color = (vl_color) { 200, 30, 30 } \
	};

/**
  Represents a custom config for VibrantLogs.

  When VibrantLogs logs messages, you can set
  how VibrantLogs prints the time and date.
*/
typedef struct vl_config
{
	/**
	  The color scheme of VibrantLogs.

	  By default, it is set to VL_COLOR_SCHEME_DEFAULT.
	*/
	vl_color_scheme colors;
	/**
	  The output destination of VibrantLogs
	  messages.

	  The default destination is stdout, but
	  you can set this to another file, and VibrantLogs
	  will write to that file instead.
	*/
	FILE *output_destination;
	/**
	  The log level of VibrantLogs.

	  This acts as a filter for which log messages are
	  allowed through. For example, if the log level
	  is VL_ERROR, only error messages are printed,
	  but if the log level is VL_WARNING, then both
	  warning and error messages are shown. The VL_DEBUG
	  and VL_SUCCESS log types are exceptions to this rule.
	*/
	vl_type log_level;
	/**
	  Indicates whether or not VibrantLogs will
	  print the current time. This is true by default.
	*/
	bool print_time;
	/**
	  Indicates whether or not VibrantLogs will
	  print the current date. This includes
	  the day, month, and year. This is false
	  by default.
	*/
	bool print_date;
	/**
	  Indicates whether or not VibrantLogs should
	  print the day's name. This is false by default.

	  @note If this is true, this will result in the
	  day's full name being used. Additionally,
	  if this and 'print_abbrev_day_name' are both
	  true, 'print_full_day_name' takes precedence.

	  @important For day names to print, 'print_date'
	  also needs to be true.
	*/
	bool print_full_day_name;
	/**
	  Indicates whether or not VibrantLogs should
	  print the day's name. This is false by default.

	  @note If this is true, this will result in the
	  day's abbreviated name being used. Additionally,
	  if this and 'print_full_day_name' are both true,
	  'print_full_day_name' takes precedence.

	  @important For day names to print, 'print_date'
	  also needs to be true.
	*/
	bool print_abbrev_day_name;
	/**
	  Indicates whether or not VibrantLogs should
	  print the month's name instead of a number value.
	  This is false by default.

	  @note If this is true, this will result in the
	  month's full name being used. Additionally,
	  if this and 'print_abbrev_month_name' are both
	  true, 'print_full_month_name' takes precedence.

	  @important For month names to print, 'print_date'
	  also needs to be true.
	*/
	bool print_full_month_name;
	/**
	  Indicates whether or not VibrantLogs should
	  print the month's name instead of a number value.
	  This is false by default.

	  @note If this is true, this will result in the
	  month's abbreviated name being used. Additionally,
	  if this and 'print_full_month_name' are both true,
	  'print_full_month_name' takes precedence.

	  @important For month names to print, 'print_date'
	  also needs to be true.
	*/
	bool print_abbrev_month_name;
} vl_config;

#ifdef _WIN32
	#ifdef VL_DLL
		#ifdef VL_EXPORTS
			#define VL_API __declspec(dllexport)
		#else
			#define VL_API __declspec(dllimport)
		#endif
	#else
		#define VL_API
	#endif
#else
	#define VL_API
#endif

/**
  Initializes the VibrantLogs library.

  @note This must be called for VibrantLogs
  to work properly.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_init(void);

/**
  Sets the config of VibrantLogs.

  @see vl_config.
*/
VL_API void vl_use_config(vl_config cfg);
/**
  Obtains the current config of VibrantLogs.
*/
VL_API vl_config *vl_curr_config(void);

/**
  Prints a message.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next
  line when printing messages.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_log(vl_type log_type, const char *fmt, ...);
/**
  Continues printing characters to the last message printed.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_print(vl_type log_type, const char *fmt, ...);
/**
  Prints a delayed message.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_delay_log(vl_type log_type, double seconds, const char *fmt, ...);
/**
  Prints a delayed message.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_delay_print(vl_type log_type, double seconds, const char *fmt, ...);

typedef struct timey_timestamp timey_timestamp;
typedef struct timey_datetime timey_datetime;

/**
  Schedules a message using a timestamp.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_schedule_log_ts(vl_type log_type, struct timey_timestamp *ts, const char *fmt, ...);
/**
  Schedules a message using a timestamp.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_schedule_print_ts(vl_type log_type, struct timey_timestamp *ts, const char *fmt, ...);
/**
  Schedules a message using a datetime.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_schedule_log_dt(vl_type log_type, struct timey_datetime *dt, const char *fmt, ...);
/**
  Schedules a message using a datetime.

  @note The max length of a VibrantLogs message
  is 256 characters.

  @note VibrantLogs does not automatically go to the next line
  when printing messages.

  @note This function only works when vl_update(double)
  is called during the program.

  @return 1 on success, 0 on failure.
*/
VL_API int vl_schedule_print_dt(vl_type log_type, timey_datetime *dt, const char *fmt, ...);

/**
  Updates the VibrantLogs library.

  This function is used to update delayed logs.
  It must be called in the program's event loop.
  The function also needs the delta time value
  of the program. Delta time is the time between
  the last frame and the current frame.
*/
VL_API void vl_update(double delta_time);
