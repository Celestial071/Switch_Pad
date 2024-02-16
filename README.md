# Switch_Pad
Switch Pad is a ardruino based gaming controller wannabe. It is implemented by mapping keyboard keys as controller's buttons. While also adding a toggling mechanism by which you can toggle to use your controller as a Mouse.

The arduino I used happens to be a arduino Nano.

Switch_Pad currently can only be ran using Windows as I have used Win32 api to control mouse, emulate keyboard presses and initiate (read and write to and also identify the serial port).

![SGCAM_20240216_213514099](https://github.com/Celestial071/Switch_Pad/assets/157342628/03a64469-0cb4-455a-bd7b-7af00b3b2b80)


Features:
1. Can change between controller or mouse mode by a switch
2. Can automatically select the correct port arduino is connected to and connect through a simple handshaking.
3. If everything is implemented as (switch_pad from keyboard) the program is reliable and offers a smooth experience.


Current Problem I am facing:-
1. Arduino Starts from a garbage value and will only stabilize after a while.
2. using state change to register button presses has resulted in no new data being register.
3. Currently arduino initializes the handshaking so if by any chance the program closes you will have to rerun the program and also reconnect the arduino to reconnect.


I will provide the circuit diagram and component after the switchpad is fully functional. Thank you.
