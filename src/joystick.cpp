#include "joystick.h"
#include <string.h>

cJoystick::cJoystick() {
	active = false;
	joystick_fd = 0;
	joystick_ev = new js_event();
	joystick_st = new joystick_state();
	joystick_fd = open(JOYSTICK_DEV, O_RDONLY | O_NONBLOCK);
	if (joystick_fd > 0) {
		ioctl(joystick_fd, JSIOCGNAME(256), name);
		ioctl(joystick_fd, JSIOCGVERSION, &version);
		ioctl(joystick_fd, JSIOCGAXES, &axes);
		ioctl(joystick_fd, JSIOCGBUTTONS, &buttons);
		std::cout << "   Name: " << name << std::endl;
		std::cout << "Version: " << version << std::endl;
		std::cout << "   Axes: " << (int)axes << std::endl;
		std::cout << "Buttons: " << (int)buttons << std::endl;
		joystick_st->axis.reserve(axes);
		joystick_st->button.reserve(buttons);
		joystick_st->btn_b = 0;
		active = true;
		pthread_create(&thread, 0, &cJoystick::loop, this);
	}

    event_fd = open(EVENT_DEV, O_RDWR);
    if (event_fd > 0)
    {
        ioctl(event_fd, EVIOCGNAME(sizeof(name)), name);
        std::cout << "Force feedback Device." << std::endl;
        std::cout << "Device " << name << " opened\n";

        /* Set master gain to 100% if supported */
        memset(&gain, 0, sizeof(gain));
        gain.type = EV_FF;
        gain.code = FF_GAIN;
        gain.value = 0xFFFF; /* [0, 0xFFFF]) */
        write(event_fd, &gain, sizeof(gain));

        /* pulse Left rumbling effect */
        effects[0].type = FF_RUMBLE;
        effects[0].id = -1;
        effects[0].u.rumble.strong_magnitude = 0xffff;
        effects[0].u.rumble.weak_magnitude = 0;
        effects[0].replay.length = 200;
        effects[0].replay.delay = 0;
        ioctl(event_fd, EVIOCSFF, &effects[0]);

        /* pulse right rumbling effect */
        effects[1].type = FF_RUMBLE;
        effects[1].id = -1;
        effects[1].u.rumble.strong_magnitude = 0;
        effects[1].u.rumble.weak_magnitude = 0xffff;
        effects[1].replay.length = 200;
        effects[1].replay.delay = 0;
        ioctl(event_fd, EVIOCSFF, &effects[1]);

        /* long Left rumbling effect */
        effects[2].type = FF_RUMBLE;
        effects[2].id = -1;
        effects[2].u.rumble.strong_magnitude = 0xffff;
        effects[2].u.rumble.weak_magnitude = 0;
        effects[2].replay.length = 60000;
        effects[2].replay.delay = 0;
        ioctl(event_fd, EVIOCSFF, &effects[2]);

        /* long right rumbling effect */
        effects[3].type = FF_RUMBLE;
        effects[3].id = -1;
        effects[3].u.rumble.strong_magnitude = 0;
        effects[3].u.rumble.weak_magnitude = 0xffff;
        effects[3].replay.length = 60000;
        effects[3].replay.delay = 0;
        ioctl(event_fd, EVIOCSFF, &effects[3]);

    }

}

cJoystick::~cJoystick() {
	if (joystick_fd > 0) {
		active = false;
		pthread_join(thread, 0);
		close(joystick_fd);
	}
	delete joystick_st;
	delete joystick_ev;
	joystick_fd = 0;
}

void* cJoystick::loop(void *obj) {
	while (reinterpret_cast<cJoystick *>(obj)->active) reinterpret_cast<cJoystick *>(obj)->readEv();
	return nullptr;
}

void cJoystick::readEv() {
	int bytes = read(joystick_fd, joystick_ev, sizeof(*joystick_ev));
	if (bytes > 0) {
		joystick_ev->type &= ~JS_EVENT_INIT;
		if (joystick_ev->type & JS_EVENT_BUTTON) {
			joystick_st->button[joystick_ev->number] = joystick_ev->value;
			if(joystick_ev->value)
			  joystick_st->btn_b |=  1<<joystick_ev->number;
			else
			  joystick_st->btn_b &=  ~(1<<joystick_ev->number);
		       //std::cout << "Buttons n° " << (int)joystick_ev->number << " :" << joystick_st->button[joystick_ev->number] << std::endl;
		}
		if (joystick_ev->type & JS_EVENT_AXIS) {
			joystick_st->axis[joystick_ev->number] = joystick_ev->value;
		       //std::cout << "Axis n° " << (int)joystick_ev->number << " :" << joystick_st->axis[joystick_ev->number] << std::endl;
		}
	}
}

joystick_position cJoystick::joystickPosition(int n) {
	joystick_position pos;

	if (n > -1 && n < axes) {
		int i0 = n*2, i1 = n*2+1;
		float x0 = joystick_st->axis[i0]/32767.0f, y0 = -joystick_st->axis[i1]/32767.0f;
		float x  = x0 * sqrt(1 - pow(y0, 2)/2.0f), y  = y0 * sqrt(1 - pow(x0, 2)/2.0f);

		pos.x = x0;
		pos.y = y0;
		
		pos.theta = atan2(y, x);
		pos.r = sqrt(pow(y, 2) + pow(x, 2));
	} else {
		pos.theta = pos.r = pos.x = pos.y = 0.0f;
	}
	return pos;
}

int cJoystick::joystickValue(int n) {

	return n > -1 && n < axes ? joystick_st->axis[n] : 0;
}

bool cJoystick::buttonPressed(int n) {
	return n > -1 && n < buttons ? joystick_st->button[n] : 0;
}

void cJoystick::pulse()
{
    leftPulse();
    rightPulse();
}

void cJoystick::rumbleON()
{
    leftON();
    rightON();
}

void cJoystick::rumbleOFF()
{
    leftOFF();
    rightOFF();
}

void cJoystick::leftPulse()
{
    play_f(effects[0].id,1);
}

void cJoystick::rightPulse()
{
    play_f(effects[1].id,1);
}

void cJoystick::leftON()
{
    play_f(effects[2].id,1);
}

void cJoystick::leftOFF()
{
    play_f(effects[2].id,0);
}

void cJoystick::rightON()
{
    play_f(effects[3].id,1);
}

void cJoystick::rightOFF()
{
    play_f(effects[3].id,0);
}

void cJoystick::play_f(__u16 code, __s32 value)
{
    memset(&play,0,sizeof(play));
    play.type = EV_FF;
    play.code = code;
    play.value = value;
    write(event_fd, (const void*) &play, sizeof(play));
}
