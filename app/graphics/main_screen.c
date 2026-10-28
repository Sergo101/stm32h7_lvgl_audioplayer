#include "main_screen.h"

#include <stdint.h>

#include "lvgl.h"

#define SCREEN_COUNT        3
#define SWITCH_ANIM_MS      200
#define THEME_BG_RGB        0xF4F6F9U        /* also the tint of the cover backdrops */
#define COLOR_BG            lv_color_hex(THEME_BG_RGB)
#define COLOR_BTN2          lv_color_hex(0x808080)

lv_obj_t *s_screens[SCREEN_COUNT];

static lv_obj_t *make_screen(void)
{
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_scrollable(scr, false);
  return scr;
}

static void btn_event_doubleclick(lv_event_t * e)
{
    lv_screen_load_anim(s_screens[0], LV_SCREEN_LOAD_ANIM_FADE_IN, SWITCH_ANIM_MS, 0, false);

}

static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target_obj(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
        lv_screen_load_anim(s_screens[1], LV_SCREEN_LOAD_ANIM_FADE_IN, SWITCH_ANIM_MS, 0, false);
    }
}

/**
 * @title Button with click counter
 * @brief Increment a label on a button each time it is clicked.
 *
 * A button sized 120x50 is placed at position (10, 10) on the active screen
 * with a centered label reading `Button`. The button subscribes to
 * `LV_EVENT_ALL` and on `LV_EVENT_CLICKED` the callback updates its child
 * label with `lv_label_set_text_fmt` to show an incrementing counter.
 */
void lv_example_get_started_button(void)
{
    lv_obj_t * btn = lv_button_create(lv_screen_active());     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/
    
    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);
    
    s_screens[0] = lv_screen_active();
    
    s_screens[1] = make_screen();
    btn = lv_button_create(s_screens[1]);
    lv_obj_set_pos(btn, 100, 100);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_doubleclick, LV_EVENT_LONG_PRESSED, NULL);           /*Assign a callback to the button*/
    lv_obj_set_style_bg_color(btn, COLOR_BTN2, 0);

}
