#ifndef DDX_JOYSTICK_H
#define DDX_JOYSTICK_H

#ifdef __cplusplus
extern "C" {
#endif
/*********************************************************************
* \brief        Joystick data structure for the store
* \author       Elliot Duff & Fabrice Chinjoie
*********************************************************************/

#include <ddx.h>

DDX_STORE_TYPE( DDX_JOYSTICK, 
  struct {
    int nb;	/*!< Number of Buttons */
    int na;	/*!< Number of Axes */
    int axes[12];
    int buttons[12];
} );

#ifdef __cplusplus
}
#endif
#endif /* DDX_JOYSTICK_H */
