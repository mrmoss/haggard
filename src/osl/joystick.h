#ifndef __OSL_JOYSTICK_H
#define __OSL_JOYSTICK_H

/* Call this to begin getting joystick reports. */
extern void Joystick_begin(void);

enum {max_buttons=32, max_axes=16};

/* This array is indexed by the joystick axis number.
  It's -1 for the left, 0 for neutral, and +1 for the right. */
extern int joy_buttons[max_buttons];

/* This array is indexed by the joystick button number.
  It's 1 for a pressed button, 0 for a non-pressed button. */
extern float joy_axes[max_axes];

#endif