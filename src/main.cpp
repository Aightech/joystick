#include "joystick.h"
#include <iostream>
int main()
{
  
  cJoystick js;
  for(;;)
    std::cout << js.joystickValue(1) << "            \xd" << std::flush;
}
