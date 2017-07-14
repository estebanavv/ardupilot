
#include "gpio_hal.h"
#include <boards.h>

#include <exti.h>

#include "GPIO.h"
#include "Scheduler.h"



using namespace REVOMINI;

REVOMINIGPIO::REVOMINIGPIO()
{
}


void REVOMINIGPIO::init() 
{
    gpio_init_all();

    afio_init(); // empty
}


void REVOMINIGPIO::_pinMode(uint8_t pin, uint8_t output)
{
    gpio_pin_mode outputMode;
    bool pwm = false;

    switch(output) {
    case OUTPUT:
        outputMode = GPIO_OUTPUT_PP;
        break;
    case OUTPUT_OPEN_DRAIN:
        outputMode = GPIO_OUTPUT_OD;
        break;
    case INPUT:
    case INPUT_FLOATING:
        outputMode = GPIO_INPUT_FLOATING;
        break;
    case INPUT_ANALOG:
        outputMode = GPIO_INPUT_ANALOG;
        break;
    case INPUT_PULLUP:
        outputMode = GPIO_INPUT_PU;
        break;
    case INPUT_PULLDOWN:
        outputMode = GPIO_INPUT_PD;
        break;

    case PWM:
        outputMode = GPIO_AF_OUTPUT_PP;
        pwm = true;
        break;

    case PWM_OPEN_DRAIN:
        outputMode = GPIO_AF_OUTPUT_OD;
        pwm = true;
        break;

    default:
        assert_param(0);
        return;
    }


    const stm32_pin_info &p = PIN_MAP[pin];

    const gpio_dev* dev =     p.gpio_device;
    uint8_t bit =             p.gpio_bit;
    const timer_dev * timer = p.timer_device;

    gpio_set_mode(dev, bit, outputMode);

    if (pwm && timer != NULL) {    
        gpio_set_speed(dev, bit, GPIO_Speed_25MHz);  // cleanflight sets 2MHz
	GPIO_PinAFConfig(dev->GPIOx, bit, timer->af);
	timer_set_mode(timer, p.timer_channel, TIMER_PWM); // init in setupTimers()
    }
}


void REVOMINIGPIO::pinMode(uint8_t pin, uint8_t output){

    if ((pin >= BOARD_NR_GPIO_PINS))   return;

    _pinMode(pin, output);
}


uint8_t REVOMINIGPIO::read(uint8_t pin) {
    if (pin >= BOARD_NR_GPIO_PINS)     return 0;

    return _read(pin);
}


void REVOMINIGPIO::write(uint8_t pin, uint8_t value) {
    if ((pin >= BOARD_NR_GPIO_PINS))   return;

    _write(pin, value);
}


void REVOMINIGPIO::toggle(uint8_t pin)
{
    if ((pin >= BOARD_NR_GPIO_PINS))  return;
    
    const stm32_pin_info &p = PIN_MAP[pin];
    
    gpio_toggle_bit(p.gpio_device, p.gpio_bit);
}


/* Interrupt interface: */
// TODO - add ability to attach not only C functions but class methods too
bool REVOMINIGPIO::_attach_interrupt(uint8_t pin, uint64_t p, uint8_t mode, uint8_t priority)
{
    if ( (pin >= BOARD_NR_GPIO_PINS) || !p) return false;

    const stm32_pin_info &pp = PIN_MAP[pin];
    
    exti_attach_interrupt_pri((afio_exti_num)(pp.gpio_bit),
                           gpio_exti_port(pp.gpio_device),
                           p, exti_out_mode((ExtIntTriggerMode)mode),
                           priority);

    return true;
}

void REVOMINIGPIO::detach_interrupt(uint8_t pin)
{
    if ( pin >= BOARD_NR_GPIO_PINS) return;

    exti_detach_interrupt((afio_exti_num)(PIN_MAP[pin].gpio_bit));
}



/* Alternative interface: */
AP_HAL::DigitalSource* REVOMINIGPIO::channel(uint16_t pin) {

    if ((pin >= BOARD_NR_GPIO_PINS)) return NULL;

    return  get_channel(pin); 
}


void REVOMINIDigitalSource::mode(uint8_t output)
{
    gpio_pin_mode outputMode;

    switch(output) {
    case OUTPUT:
        outputMode = GPIO_OUTPUT_PP;
        break;
    case OUTPUT_OPEN_DRAIN:
        outputMode = GPIO_OUTPUT_OD;
        break;
    case INPUT:
    case INPUT_FLOATING:
        outputMode = GPIO_INPUT_FLOATING;
        break;
    case INPUT_ANALOG:
        outputMode = GPIO_INPUT_ANALOG;
        break;
    case INPUT_PULLUP:
        outputMode = GPIO_INPUT_PU;
        break;
    case INPUT_PULLDOWN:
        outputMode = GPIO_INPUT_PD;
        break;
    // no PWM via this interface!
    default:
        assert_param(0);
        return;
    }

    gpio_set_mode(_device, _bit, outputMode);
}

