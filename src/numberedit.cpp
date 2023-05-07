/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "numberedit.h"
#include "widgets/field_edit.h"
#include "keyboard_number.h"

static void numberedit_cb(lv_event_t* e)
{
  NumberEdit* numEdit = (NumberEdit*)lv_event_get_user_data(e);
  if (!numEdit || numEdit->deleted()) return;

  uint32_t key = lv_event_get_key(e);
  switch (key) {
  case LV_KEY_LEFT:
    numEdit->onEvent(EVT_ROTARY_LEFT);
    break;
  case LV_KEY_RIGHT:
    numEdit->onEvent(EVT_ROTARY_RIGHT);
    break;
  }
}

NumberEdit::NumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
                       std::function<int()> getValue,
                       std::function<void(int)> setValue,
                       WindowFlags windowFlags, LcdFlags textFlags) :
    BaseNumberEdit(parent, rect, vmin, vmax, std::move(getValue),
                   std::move(setValue), windowFlags, textFlags,
                   field_edit_create)
{
  // Allow encoder acceleration
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_ENCODER_ACCEL);

  // properties
  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_OFF);
  lv_textarea_set_password_mode(lvobj, false);
  lv_textarea_set_one_line(lvobj, true);
  lv_obj_add_event_cb(lvobj, numberedit_cb, LV_EVENT_KEY, this);

  setHeight(33);
  padTop(5);
  padRight(4);
  lv_obj_set_style_text_align(lvobj, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_style_radius(lvobj, 4, 0);

  update();
}

void NumberEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (editMode) {
    switch (event) {
#if defined(HARDWARE_KEYS)
      case EVT_ROTARY_RIGHT: {
        int value = getValue();
        auto step = getStep();
        step += (rotaryEncoderGetAccel() * getAccelFactor()) / 8;
        do {
          value += step;
        } while (isValueAvailable && !isValueAvailable(value) && value <= vmax);
        if (value <= vmax) {
          setValue(value);
        }
        else {
          setValue(vmax);
          onKeyError();
        }
        return;
      }

      case EVT_ROTARY_LEFT: {
        int value = getValue();
        auto step = getStep();
        step += (rotaryEncoderGetAccel() * getAccelFactor()) / 8;
        do {
          value -= step;
        } while (isValueAvailable && !isValueAvailable(value) && value >= vmin);
        if (value >= vmin) {
          setValue(value);
        }
        else {
          setValue(vmin);
          onKeyError();
        }
        return;
      }
#endif

      case EVT_VIRTUAL_KEY_PLUS:
        setValue(getValue() + getStep());
        break;

      case EVT_VIRTUAL_KEY_MINUS:
        setValue(getValue() - getStep());
        break;

      case EVT_VIRTUAL_KEY_FORWARD:
        setValue(getValue() + getFastStep() * getStep());
        break;

      case EVT_VIRTUAL_KEY_BACKWARD:
        setValue(getValue() - getFastStep() * getStep());
        break;

      case EVT_VIRTUAL_KEY_DEFAULT:
        setValue(getDefault());
        break;

      case EVT_VIRTUAL_KEY_MAX:
        setValue(getMax());
        break;

      case EVT_VIRTUAL_KEY_MIN:
        setValue(getMin());
        break;

      case EVT_VIRTUAL_KEY_SIGN:
        setValue(-getValue());
        break;
    }
  }

  FormField::onEvent(event);
}

void NumberEdit::onClicked()
{
  lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
  if(indev_type == LV_INDEV_TYPE_POINTER) {
    NumberKeyboard::show(this);
    return;
  }

  FormField::onClicked();
}
