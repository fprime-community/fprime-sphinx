#include "FPGAGPIODefATB.hpp"
#include <Fw/Types/Assert.hpp>
#include <App/Include/config.hpp>

const NATIVE_UINT_TYPE FPGA_GPIO_TOTAL_AVAIL_PINS = 13;
const NATIVE_UINT_TYPE FPGA_GPIO_LENGTH = 9;
NATIVE_UINT_TYPE FPGA_GPIO_PINS_ENABLE[] = {
        PIN_NUM_0, PIN_NUM_1, PIN_NUM_2, PIN_NUM_4,
        PIN_NUM_6, PIN_NUM_7, PIN_NUM_8, PIN_NUM_9,
        PIN_NUM_10,
};
FW_STATIC_ASSERT(FPGA_GPIO_LENGTH == FW_NUM_ARRAY_ELEMENTS(FPGA_GPIO_PINS_ENABLE));
