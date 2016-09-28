/*
 * mcp3021.c - Support for Microchip MCP3021
 *
 * Copyright (C) 2014 Krzysztof Chojnowski <krzysiek@embedev.pl>
 *
 * Based on mcp320x by Oskar Andero <oskar.andero@gmail.com> 
 * and mcp4725 by Peter Meerwald <pmeerw@pmeerw.net>
 *
 * This file is subject to the terms and conditions of version 2 of
 * the GNU General Public License.  See the file COPYING in the main
 * directory of this archive for more details.
 *
 * driver for the Microchip I2C 10-bit analog-to-digital converter (ADC)
 * (7-bit I2C slave address 0x4D, the three LSBs can be configured in
 * hardware)
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/iio/iio.h>

#define MCP3021_DRV_NAME "mcp3021"

struct mcp3021_data{
	struct i2c_client *client;
};

static const struct iio_chan_spec mcp3021_channel = {
	.type		= IIO_VOLTAGE,
	.indexed	= 1,
	.channel	= 0,
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
};

static int mcp3021_read_i2c_value(const struct i2c_client *client) {
	int result;
	char buf[2];

	i2c_master_recv(client, buf, 2);
	result = buf[1]>>2;
	result += ((int)buf[0])<<6;

	return result;
}

static int mcp3021_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val, int *val2, long mask)
{
	struct mcp3021_data *data = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		*val = mcp3021_read_i2c_value(data->client);
		return IIO_VAL_INT;
	}
	return -EINVAL;
}

static const struct iio_info mcp3021_info = {
	.read_raw = mcp3021_read_raw,
	.driver_module = THIS_MODULE,
};

static int mcp3021_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct mcp3021_data *data;
	struct iio_dev *indio_dev;
	int err;

	indio_dev = iio_device_alloc(sizeof(*data));
	if (indio_dev == NULL)
		return -ENOMEM;
	data = iio_priv(indio_dev);
	i2c_set_clientdata(client, indio_dev);
	data->client = client;

	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &mcp3021_info;
	indio_dev->channels = &mcp3021_channel;
	indio_dev->num_channels = 1;
	indio_dev->modes = INDIO_DIRECT_MODE;

	err = iio_device_register(indio_dev);
	if (err) 
		return err;

	dev_info(&client->dev, "MCP3021 ADC registered\n");

	return 0;
}

static int mcp3021_remove(struct i2c_client *client)
{
	struct iio_dev *iio = i2c_get_clientdata(client);
	iio_device_unregister(iio);

	return 0;
}

static const struct i2c_device_id mcp3021_id[] = {
	{ "mcp3021", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mcp3021_id);

static struct i2c_driver mcp3021_driver = {
	.driver = {
		.name	= MCP3021_DRV_NAME,
	},
	.probe		= mcp3021_probe,
	.remove		= mcp3021_remove,
	.id_table	= mcp3021_id,
};
module_i2c_driver(mcp3021_driver);

MODULE_AUTHOR("Krzysztof Chojnowski <krzysiek@embedev.pl>");
MODULE_DESCRIPTION("MCP3021 10-bit ADC");
MODULE_LICENSE("GPL");
