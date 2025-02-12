#include "joystick.h"

using namespace ESC;

void cJoystick::connect(const char *dev_path, const char *ev_path)
{
    m_active = false;
    m_joystick_fd = 0;
    m_joystick_ev = new js_event();
    m_joystick_st = new joystick_state();
    m_joystick_fd = open(dev_path, O_RDONLY | O_NONBLOCK);
    if(m_joystick_fd > 0)
    {

        ioctl(m_joystick_fd, JSIOCGNAME(256), m_name);
        ioctl(m_joystick_fd, JSIOCGVERSION, &m_version);
        ioctl(m_joystick_fd, JSIOCGAXES, &m_axes);
        ioctl(m_joystick_fd, JSIOCGBUTTONS, &m_buttons);
        logln(fstr("Name:", {UNDERLINE}) + fstr(m_name, {BOLD}), true);
        logln(fstr("Path:", {UNDERLINE}) + fstr(dev_path, {BOLD}));
        logln(fstr("Version:", {UNDERLINE}) + fstr_n(m_version, {BOLD}));
        logln(fstr("Axes:", {UNDERLINE}).c_str() + fstr_n((int)m_axes, {BOLD}));
        logln(fstr("Buttons:", {UNDERLINE}) + fstr_n((int)m_buttons, {BOLD}));
        m_joystick_st->axis.reserve(m_axes);
        m_joystick_st->button.reserve(m_buttons);
        m_joystick_st->btn_b = 0;
        m_active = true;
        pthread_create(&m_thread, 0, &cJoystick::loop, this);
    }
    else
        throw log_error(std::string("No joystick found at ") + dev_path);

    std::string event_path(ev_path);
    if(event_path == std::string("")) //search for associated joystick event
    {
        for(int i = 0; i < 40; i++)
        {
            event_path = "/dev/input/event" + std::to_string(i);
            m_event_fd = open(event_path.c_str(), O_RDWR);
            if(m_event_fd > 0)
            {
                char test[256];
                ioctl(m_event_fd, EVIOCGNAME(sizeof(test)), test);
                if(std::string(m_name) == std::string(test))
                    break;
                close(m_event_fd);
            }
        }
    }

    m_event_fd = open(event_path.c_str(), O_RDWR);
    if(m_event_fd > 0)
    {
        char test[256];
        ioctl(m_event_fd, EVIOCGNAME(sizeof(test)), test);
        logln(fstr("Force feedback:", {UNDERLINE}) + fstr(event_path, {BOLD}));

        /* Set master gain to 100% if supported */
        memset(&m_gain, 0, sizeof(m_gain));
        m_gain.type = EV_FF;
        m_gain.code = FF_GAIN;
        m_gain.value = 0xFFFF; /* [0, 0xFFFF]) */
        ssize_t n = write(m_event_fd, &m_gain, sizeof(m_gain));
        (void)n;

        /* pulse Left rumbling effect */
        m_effects[0].type = FF_RUMBLE;
        m_effects[0].id = -1;
        m_effects[0].u.rumble.strong_magnitude = 0xffff;
        m_effects[0].u.rumble.weak_magnitude = 0;
        m_effects[0].replay.length = 200;
        m_effects[0].replay.delay = 0;
        ioctl(m_event_fd, EVIOCSFF, &m_effects[0]);

        /* pulse right rumbling effect */
        m_effects[1].type = FF_RUMBLE;
        m_effects[1].id = -1;
        m_effects[1].u.rumble.strong_magnitude = 0;
        m_effects[1].u.rumble.weak_magnitude = 0xffff;
        m_effects[1].replay.length = 200;
        m_effects[1].replay.delay = 0;
        ioctl(m_event_fd, EVIOCSFF, &m_effects[1]);

        /* long Left rumbling effect */
        m_effects[2].type = FF_RUMBLE;
        m_effects[2].id = -1;
        m_effects[2].u.rumble.strong_magnitude = 0xffff;
        m_effects[2].u.rumble.weak_magnitude = 0;
        m_effects[2].replay.length = 60000;
        m_effects[2].replay.delay = 0;
        ioctl(m_event_fd, EVIOCSFF, &m_effects[2]);

        /* long right rumbling effect */
        m_effects[3].type = FF_RUMBLE;
        m_effects[3].id = -1;
        m_effects[3].u.rumble.strong_magnitude = 0;
        m_effects[3].u.rumble.weak_magnitude = 0xffff;
        m_effects[3].replay.length = 60000;
        m_effects[3].replay.delay = 0;
        ioctl(m_event_fd, EVIOCSFF, &m_effects[3]);
    }
    else
        throw log_error(std::string("No joystick event found at ") + ev_path);
}

cJoystick::~cJoystick()
{
    if(m_joystick_fd > 0)
    {
        m_active = false;
        pthread_join(m_thread, 0);
        close(m_joystick_fd);
    }
    delete m_joystick_st;
    delete m_joystick_ev;
    m_joystick_fd = 0;
}

void *cJoystick::loop(void *obj)
{
    while(reinterpret_cast<cJoystick *>(obj)->m_active)
        reinterpret_cast<cJoystick *>(obj)->readEv();
    return nullptr;
}

void cJoystick::readEv()
{
    int bytes = read(m_joystick_fd, m_joystick_ev, sizeof(*m_joystick_ev));
    if(bytes > 0)
    {
        m_joystick_ev->type &= ~JS_EVENT_INIT;
        if(m_joystick_ev->type & JS_EVENT_BUTTON)
        {
            m_joystick_st->button[m_joystick_ev->number] = m_joystick_ev->value;
            if(m_joystick_ev->value)
                m_joystick_st->btn_b |= 1 << m_joystick_ev->number;
            else
                m_joystick_st->btn_b &= ~(1 << m_joystick_ev->number);
            //std::cout << "Buttons n° " << joystick_st->btn_b << " :" << joystick_st->button[joystick_ev->number] << std::endl;
        }
        if(m_joystick_ev->type & JS_EVENT_AXIS)
        {
            m_joystick_st->axis[m_joystick_ev->number] = m_joystick_ev->value;
            //std::cout << "Axis n° " << (int)joystick_ev->number << " :" << joystick_st->axis[joystick_ev->number] << std::endl;
        }
    }
}

joystick_position cJoystick::joystickPosition(int n)
{
    joystick_position pos;

    if(n > -1 && n < m_axes)
    {
        int i0 = n * 2, i1 = n * 2 + 1;
        float x0 = m_joystick_st->axis[i0] / 32767.0f,
              y0 = -m_joystick_st->axis[i1] / 32767.0f;
        float x = x0 * sqrt(1 - pow(y0, 2) / 2.0f),
              y = y0 * sqrt(1 - pow(x0, 2) / 2.0f);

        pos.x = x0;
        pos.y = y0;

        pos.theta = atan2(y, x);
        pos.r = sqrt(pow(y, 2) + pow(x, 2));
    }
    else
    {
        pos.theta = pos.r = pos.x = pos.y = 0.0f;
    }
    return pos;
}

int16_t cJoystick::joystickValue(int n)
{

    return n > -1 && n < m_axes ? m_joystick_st->axis[n] : 0;
}

bool cJoystick::buttonPressed(int n)
{
    return n > -1 && n < m_buttons ? m_joystick_st->button[n] : 0;
}

uint32_t cJoystick::button_bytes() { return m_joystick_st->btn_b; }

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

void cJoystick::leftPulse() { play_f(m_effects[0].id, 1); }

void cJoystick::rightPulse() { play_f(m_effects[1].id, 1); }

void cJoystick::leftON() { play_f(m_effects[2].id, 1); }

void cJoystick::leftOFF() { play_f(m_effects[2].id, 0); }

void cJoystick::rightON() { play_f(m_effects[3].id, 1); }

void cJoystick::rightOFF() { play_f(m_effects[3].id, 0); }

void cJoystick::play_f(__u16 code, __s32 value)
{
    memset(&m_play, 0, sizeof(m_play));
    m_play.type = EV_FF;
    m_play.code = code;
    m_play.value = value;
    ssize_t n = write(m_event_fd, (const void *)&m_play, sizeof(m_play));
    (void)n;
}
