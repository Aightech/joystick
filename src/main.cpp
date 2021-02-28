#include "joystick.h"
#include <iostream>
int main()
{
  
  cJoystick js;
  for(;;)
    std::cout << js.joystickValue(7) << "            \xd" << std::flush;
}
