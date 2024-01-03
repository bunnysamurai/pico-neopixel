#if !defined(PICO_LOGGER_HPP)
#define PICO_LOGGER_HPP

#include "pico/stdlib.h"
#include "pico/printf.h"

#include <string_view>
#include <array>
#include <cstddef>
#include <utility>

namespace pico
{
    namespace logger
    {
        class PicoLogger
        {
        public:
            PicoLogger() noexcept : m_okay{stdio_init_all()}
            {
            }

            template <class... Args>
            void print(std::string_view str, Args &&...vals)
            {
                static constexpr auto MAX_LEN{80};
                std::array<char, MAX_LEN + 1> null_term{};
                if (std::size(str) > MAX_LEN)
                {
                    printf("print_error_fail");
                }
                memcpy(std::data(null_term), std::data(str), std::size(str));
                printf(std::data(null_term), std::forward<Args>(vals)...);
            }

            [[nodiscard]] bool is_okay() const noexcept { return m_okay; }

        private:
            bool m_okay;
        };

        template <class... Args>
        void print(PicoLogger &dev, std::string_view str, Args &&...vals)
        {
            dev.print(str, std::forward<Args>(vals)...);
        }

        template <class... Args>
        void print_error(PicoLogger &dev, std::string_view str, Args &&...vals)
        {
            print(dev, "[!] ");
            print(dev, str, std::forward<Args>(vals)...);
        }

        template <class... Args>
        void print_info(PicoLogger &dev, std::string_view str, Args &&...vals)
        {
            print(dev, "[*] ");
            print(dev, str, std::forward<Args>(vals)...);
        }

        template <class... Args>
        void print_debug(PicoLogger &dev, std::string_view str, Args &&...vals)
        {
            print(dev, "[@] ");
            print(dev, str, std::forward<Args>(vals)...);
        }
    }
}
#endif