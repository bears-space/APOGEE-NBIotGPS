# I2C Interface

Vigilant Engine can expose an I2C master bus for node-specific sensors and peripherals. When I2C is enabled in
menuconfig, the bus is initialized during `vigilant_init()` and can then be used from the main firmware through
the public `vigilant_i2c_*` helpers.

## Enable I2C

In `idf.py menuconfig`, open `Vigilant Engine Configuration: I2C` and configure:

- `VE_ENABLE_I2C`: Enables the I2C interface in Vigilant Engine
- `VE_I2C_SCL_IO`: GPIO used for SCL
- `VE_I2C_SDA_IO`: GPIO used for SDA
- `VE_I2C_FREQ_HZ`: I2C bus frequency in Hertz

When enabled, Vigilant Engine builds the I2C driver, creates the bus during startup, and logs a scan of detected
7-bit device addresses.

## Runtime flow

- Call `vigilant_init(...)` first
- Define a `VigilantI2CDevice` object for your sensor or peripheral
- Add the device with `vigilant_i2c_add_device(&device)`
- Optionally verify identity with `vigilant_i2c_whoami_check(&device)`
- Read single-byte registers with `vigilant_i2c_read_reg8(&device, reg, &value)`
- Remove the device with `vigilant_i2c_remove_device(&device)` if you no longer need it

If I2C is disabled in menuconfig, the public `vigilant_i2c_*` helpers return `ESP_ERR_NOT_SUPPORTED`.

## `VigilantI2CDevice`

The I2C interface uses a small device object so the caller only has to pass one pointer around.

- `address`: 7-bit I2C device address
- `whoami_reg`: Register used by the optional WHOAMI check
- `expected_whoami`: Expected value returned by the WHOAMI register
- `handle`: Runtime device handle managed by Vigilant Engine. Initialize this to `NULL`

## Example

```c
VigilantI2CDevice accel = {
    .address = 0x18,
    .whoami_reg = 0x0F,
    .expected_whoami = 0x33,
    .handle = NULL,
};

ESP_ERROR_CHECK(vigilant_i2c_add_device(&accel));
ESP_ERROR_CHECK(vigilant_i2c_whoami_check(&accel));

uint8_t ctrl_reg = 0;
ESP_ERROR_CHECK(vigilant_i2c_read_reg8(&accel, 0x20, &ctrl_reg));
```

Use the device-specific address, register map, and expected WHOAMI value from your peripheral datasheet.

## Notes

- The current helper `vigilant_i2c_read_reg8(...)` reads one 8-bit register at a time
- The I2C bus is shared, so multiple devices can be added as separate `VigilantI2CDevice` objects
- `vigilant_i2c_add_device(...)` must be called before any read or WHOAMI check
