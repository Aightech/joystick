#include "joystick.h"
#include <iostream>

int main()
{
  
  cJoystick js;
  std::cout <<"|";
  for(int i =0; i<js.getNbButtons(); i++)
    std::cout << ((i>=10)?"":" ") << i << "|";

  
  std::cout <<"| ";
  for(int i =0; i<js.getNbAxis(); i++)
    std::cout << i << "\t| ";
  std::cout << std::endl;
 
  for(;;)
    {
      std::cout <<"|";
      for(int i =0; i<3*js.getNbButtons()+8*js.getNbAxis(); i++)
	std::cout << " ";
      std::cout <<"\xd| ";
      for(int i =0; i<js.getNbButtons(); i++)
	std::cout << js.buttonPressed(i)  << "| ";
      
      std::cout <<"\b|";;
      for(int i =0; i<js.getNbAxis(); i++)
  	std::cout << ((js.joystickValue(i)<0)?"":" ") << js.joystickValue(i) << "\t|";
      std::cout << "\xd" << std::flush;
      
    }
}
