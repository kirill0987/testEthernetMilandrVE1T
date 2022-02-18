#ifndef MTR_MODBUS_DEFINES_H_
#define MTR_MODBUS_DEFINES_H_

//Maximum timeout for waiting response from slave
#ifndef CONFIG_MODBUS_RTU_TIMEOUT_MS
#define CONFIG_MODBUS_RTU_TIMEOUT_MS 50
#endif

//Max polling device amount. Need to overdefine this value with more num if need
#ifndef CONFIG_MODBUS_CONNECTED_DEVICE_NUM
#define CONFIG_MODBUS_CONNECTED_DEVICE_NUM 8
#endif

//Max polling holding registers amount for polling device. Need to overdefine this value with more num if need
#ifndef CONFIG_MODBUS_MAX_PULLING_HOLDING_REGISTER_NUM_PER_CONNECTION
#define CONFIG_MODBUS_MAX_PULLING_HOLDING_REGISTER_NUM_PER_CONNECTION 16
#endif

//Max polling input register amount for polling device. Need to overdefine this value with more num if need
#ifndef CONFIG_MODBUS_MAX_PULLING_INPUT_REGISTER_NUM_PER_CONNECTION
#define CONFIG_MODBUS_MAX_PULLING_INPUT_REGISTER_NUM_PER_CONNECTION 16
#endif

//Two next values specify online calc behaviour
#ifndef CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE
#define CONFIG_MODBUS_RTU_MASTER_DECREASE_VALUE_FOR_ONLINE 2
#endif

#ifndef CONFIG_MODBUS_RTU_MASTER_RECENTY_GOOD_MESSAGES_AS_ONLINE
#define CONFIG_MODBUS_RTU_MASTER_RECENTY_GOOD_MESSAGES_AS_ONLINE 5
#endif

#endif /* MTR_MODBUS_DEFINES_H_ */
