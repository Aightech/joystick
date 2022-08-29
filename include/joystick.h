#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <linux/joystick.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>

#include <strANSIseq.hpp>
#include <string.h>

#define JOYSTICK_DEV "/dev/input/js0"
#define EVENT_DEV ""

struct joystick_position
{
    float theta, r, x, y;
};

struct joystick_state
{
    std::vector<int16_t> button;
    uint32_t btn_b;
    std::vector<int16_t> axis;
};

class cJoystick : public ESC::CLI
{

    public:
    cJoystick(int verbose = -1)
        : m_verbose(verbose), CLI(verbose, "Joystick"){};
    ~cJoystick();

    void connect(const char *dev_path = JOYSTICK_DEV,
                 const char *ev_path = EVENT_DEV);

    static void *loop(void *obj);

    void readEv();
    joystick_position joystickPosition(int n);
    int16_t joystickValue(int n);

    bool buttonPressed(int n);
    uint32_t button_bytes();

    void leftPulse();
    void leftON();
    void leftOFF();
    void rightPulse();
    void rightON();
    void rightOFF();
    void pulse();
    void rumbleON();
    void rumbleOFF();

    uint8_t getNbAxis() { return m_axes; };
    uint8_t getNbButtons() { return m_buttons; };

    private:
    pthread_t m_thread;
    bool m_active;
    int m_joystick_fd;
    int m_verbose;
    std::string m_verbose_id;
    std::string m_verbose_indent;

    int m_event_fd;
    struct ff_effect m_effects[4];
    struct input_event m_play, m_gain;

    js_event *m_joystick_ev;
    joystick_state *m_joystick_st;
    uint32_t m_version;
    uint8_t m_axes;
    uint8_t m_buttons;

    char m_name[256];

    void play_f(__u16 code, __s32 value);
};

#endif
