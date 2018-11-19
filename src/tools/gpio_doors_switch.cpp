/** Copyright &copy; 2017, rostislav.vel@gmail.com.
 * \brief  Тестовое приложение для отладки работы дверей.
 * \author Величко Ростислав
 * \date   07.06.2017
 */
 
#include <iostream>
#include <memory>

#include <boost/program_options.hpp>

#include "Log.hpp"
#include "GpioController.hpp"
 
namespace bpo = boost::program_options;

typedef robocooler::driver::GpioController GpioController;
typedef std::shared_ptr<GpioController> PGpioController;
typedef robocooler::driver::Timer Timer;


int main(int argc, char **argv) {
    LOG_TO_STDOUT;
    try {
        size_t timeout;
        bool open_l = false;
        bool close_l = false;
        bool open_r = false;
        bool close_r = false;
        bpo::options_description desc("Тестовое приложение для проверки работы приводов дверей.");
        desc.add_options()
          ("help,h", "Показать список параметров")
          ("timeout,t", bpo::value<size_t>(&timeout)->default_value(3000), "Таймаут после запуска каждого процесса в милисекундах.")
          ("open_l,L",  bpo::bool_switch(&open_l)->default_value(false),  "Открыть левую дверь.")
          ("close_l,l", bpo::bool_switch(&close_l)->default_value(false), "Закрыть левую дверь.")
          ("open_r,R",  bpo::bool_switch(&open_r)->default_value(false),  "Открыть правую дверь")
          ("close_r,r", bpo::bool_switch(&close_r)->default_value(false), "Закрыть правую дверь")
          ; //NOLINT
        bpo::variables_map vm;
        bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
        bpo::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }
        PGpioController gpioc = std::make_shared<GpioController>(nullptr, true);
        if (gpioc->isInited()) {
            if (open_l) {
                gpioc->openLeftDoor();
            }
            if (close_l) {
                gpioc->closeLeftDoor();
            }
            if (open_r) {
                gpioc->openRightDoor();
            }
            if (close_r) {
                gpioc->closeRightDoor();
            }
        }
        gpioc.reset();
        LOG(ERROR) << "Stop application.";
    } catch (std::exception &e) {
        LOG(ERROR) << e.what() << "\n";
    }
    return 0;
}
