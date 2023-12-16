#include "led_driver.hpp"

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <limits>

LED_Driver::LED_Driver(int pin_number) : m_pin{pin_number}, m_wrap{0xFFFFU}
{
    gpio_set_function(m_pin, GPIO_FUNC_PWM);

    pwm_config cfg{pwm_get_default_config()};
    pwm_config_set_wrap(&cfg, m_wrap);

    const auto slice{pwm_gpio_to_slice_num(m_pin)};
    const auto channel{pwm_gpio_to_channel(m_pin)};
    pwm_set_chan_level(slice, channel, 0);
    pwm_init(slice, &cfg, true);
}
void LED_Driver::put_pixel(uint32_t value)
{
    const auto slice{pwm_gpio_to_slice_num(m_pin)};
    const auto channel{pwm_gpio_to_channel(m_pin)};
    pwm_set_chan_level(slice, channel, to_pwm_level(value & 0xFFU));
}
void LED_Driver::set_rgb_mode()
{ /* do nothing*/
}
void LED_Driver::set_wrgb_mode()
{ /* do nothing*/
}

[[nodiscard]] uint16_t LED_Driver::to_pwm_level(uint8_t level)
{
    return level * m_wrap / std::numeric_limits<uint8_t>::max();
}