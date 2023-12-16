#if !defined(LED_DRIVER_HPP)
#define LED_DRIVER_HPP

#include <cstdint>

class LED_Driver
{
public:
    LED_Driver(int pin_number);

    void put_pixel(uint32_t value);
    void set_rgb_mode();
    void set_wrgb_mode();

private:
    int m_pin;
    uint16_t m_wrap;

    [[nodiscard]] uint16_t to_pwm_level(uint8_t level);
};

#endif