#include <pebble.h>
#include <stdlib.h>

static Window *window;
static TextLayer *text_layer;

char *itoa(int num) {
  static char buff[20] = {};
  int i = 0, temp_num = num, length = 0;
  char *string = buff;
  
  if(num >= 0) { //See NOTE
    // count how many characters in the number
    while(temp_num) {
      temp_num /= 10;
      length++;
    }
    // assign the number to the buffer starting at the end of the 
    // number and going to the begining since we are doing the
    // integer to character conversion on the last number in the
    // sequence
    for(i = 0; i < length; i++) {
      buff[(length-1)-i] = '0' + (num % 10);
      num /= 10;
    }
    buff[i] = '\0'; // can't forget the null byte to properly end our string
  }
  else {
    return "Unsupported Number";
  }
  
  return string;
}

// 12:34
char* shortTime() {
  char *outBuffer = calloc(6, sizeof(char));
  char buffer[] = "%H:%M";
  if (clock_is_24h_style() == true) {
    strcpy(buffer, "%I:%M");
  }

  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  strftime(outBuffer, strlen(buffer) * sizeof(char) +1, buffer, tick_time);

  return outBuffer;
}

char* randomShortTime() {
  srand(time(NULL));
  int hour = rand() % 24;
  int minutes = rand() % 60;
  char *timeString = calloc(6, sizeof(char));

  if (hour < 10) {
    strcat(timeString, "0");
  }
  strcat(timeString, itoa(hour));

  strcat(timeString, ":");

  if (minutes < 10) {
    strcat(timeString, "0");
  }
  strcat(timeString, itoa(minutes));

  return timeString;
}

char* stringForTime(bool *extraLong) {

  srand(time(NULL));
  int n = rand() % 10;

  if (n < 4) {
    // Time-related

    srand(time(NULL));
    int n2 = rand() % 3;
    srand(time(NULL));

    if (n2 == 0) {
      // Maybe
      char prefix[] = "Maybe ";
      char *timeString = rand() % 2 == 0 ? shortTime() : randomShortTime();
      char *concat = calloc(strlen(prefix) + strlen(timeString), sizeof(char));
      strcpy(concat, prefix);
      strcat(concat, timeString);
      free(timeString);
      return concat;

    } else if (n2 == 1) {
      // Maybe not
      char prefix[] = "Maybe not ";
      char *timeString = rand() % 2 == 0 ? shortTime() : randomShortTime();
      char *concat = calloc(strlen(prefix) + strlen(timeString), sizeof(char));
      strcpy(concat, prefix);
      strcat(concat, timeString);
      free(timeString);
      return concat;

    } else if (n2 == 2) {
      // probably
      char prefix[] = "Probably ";
      char *timeString = rand() % 2 == 0 ? shortTime() : randomShortTime();
      char *concat = calloc(strlen(prefix) + strlen(timeString), sizeof(char));
      strcpy(concat, prefix);
      strcat(concat, timeString);
      free(timeString);
      return concat;
    }

  } else if (n < 8) {
    // Wrong time
    char *correctTime = shortTime();
    char *randomTime = randomShortTime();
    while (strcmp(correctTime, randomTime) == 0) {
      randomTime = randomShortTime();
    }

    srand(time(NULL));
    int n2 = rand() % 3;

    if (n2 == 0) {
      char *buffer = calloc(10, sizeof(char));
      strcpy(buffer, "Not ");
      strcat(buffer, randomTime);

      free(correctTime);
      free(randomTime);

      return buffer;

    } else if (n2 == 1) {
      char *buffer = calloc(21, sizeof(char));
      strcpy(buffer, "Definitely Not ");
      strcat(buffer, randomTime);

      free(correctTime);
      free(randomTime);
      
      return buffer;

    } else if (n2 == 2) {
      char *buffer = calloc(18, sizeof(char));
      strcpy(buffer, "Must not be ");
      strcat(buffer, randomTime);

      free(correctTime);
      free(randomTime);
      
      return buffer;
    }

  } else if (n < 10) {
    // Rubbish
    srand(time(NULL));
    int n2 = rand() % 6;

    if (n2 == 0) {
      char *result = calloc(6, sizeof(char));
      strcpy(result, "Today");
      return result;

    } else if (n2 == 1) {
      char *result = calloc(4, sizeof(char));
      strcpy(result, "Now");
      return result;
    
    } else if (n2 == 2) {
      char *buffer = calloc(15, sizeof(char));
      strcpy(buffer, "Guess the time");
      return buffer;

    } else if (n2 == 3) {
      char *buffer = calloc(17, sizeof(char));
      strcpy(buffer, "Not yet tomorrow");
      return buffer;
      
    } else if (n2 == 4) {
      char *buffer = calloc(24, sizeof(char));
      strcpy(buffer, "The day after yesterday");
      *extraLong = true;
      return buffer;

    } else if (n2 == 5) {
      char *buffer = calloc(24, sizeof(char));
      strcpy(buffer, "The day before tomorrow");
      *extraLong = true;
      return buffer;
    }
  }
  return shortTime();
}

static GRect normalTextRect() {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  return (GRect) { .origin = { 0, 20 }, .size = { bounds.size.w, bounds.size.h } };
}

static GRect longTextRect() {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  return (GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h } };
}

char *currentText = NULL;
static void update_time() {
  if (currentText != NULL) {
    free(currentText);
  }
  bool extraLong = false;
  currentText = stringForTime(&extraLong);
  text_layer_set_text(text_layer, currentText);

  if (extraLong) {
    layer_set_frame((Layer*)text_layer, longTextRect());
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  } else {
    layer_set_frame((Layer*)text_layer, normalTextRect());
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  }
}

// Refresh every 59 seconds.
// If I reload every minute, the randomness become strange and return 0 more than often,
// I have to work around by this.
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  int second = (*tick_time).tm_sec;
  int minute = (*tick_time).tm_min;
  if ((second + minute * 60) % 59 == 0) {
    update_time();
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorBlack);

  text_layer = text_layer_create(normalTextRect());
  text_layer_set_text(text_layer, "Maybe 12:34");
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  update_time();

  app_event_loop();
  deinit();
}
