// SPDX-License-Identifier: BSD-Source-Code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <generated/csr.h>

#include "../command.h"
#include "../helpers.h"

static const char *link_state_mapping[] = {
	"ErrorReset", "ErrorWait", "Ready", "Started", "Connecting", "Run"
};

/**
 * Command ""
 *
 * ...
 *
 */
static void spw_init_handler(int nb_params, char **params)
{
	if (nb_params != 0) {
		printf("spw_init");
		return;
	}

    spw_node_control_link_error_clear_write(1);
    spw_node_control_link_autorecover_write(1);
    spw_node_control_link_start_write(1);
}

define_command(spw_init, spw_init_handler, "Initialize SpaceWire Nodes", SPW_CMDS);

static void spw_send_handler(int nb_params, char **params)
{
	char *c;
	unsigned int value;
	unsigned int i;

	if (nb_params < 1) {
		printf("spw_send <value> [value2] ...");
		return;
	}

	for (i = 1; i < nb_params; i++) {
		value = strtoul(params[i], &c, 0);
		if (*c != 0) {
			printf("Incorrect value (%u)", i - 1);
			return;
		}

    	spw_node_fifo_w_write(value);
	}
}

define_command(spw_send, spw_send_handler, "Send bytes from node", SPW_CMDS);

static void spw_read_handler(int nb_params, char **params)
{
	uint32_t value;

	if (nb_params != 0) {
		printf("spw_read");
		return;
	}

	while (spw_node_status_data_available_read()) {
    	value = spw_node_fifo_r_read();
		printf("%#lx ", value);
	}
}

define_command(spw_read, spw_read_handler, "Read all bytes from node fifo", SPW_CMDS);

static void spw_status_handler(int nb_params, char **params)
{
	uint32_t value;

	if (nb_params != 0) {
		printf("spw_status");
		return;
	}

	printf("Status:\n");
    value = spw_node_status_link_error_read();
	printf("\tLink error:\t\t%s\n", value ? "yes" : "no");
    value = spw_node_status_link_state_read();
	printf("\tLink state:\t\t%s\n", link_state_mapping[value]);
	value = spw_node_status_data_available_read();
	printf("\tData available:\t\t%s", value ? "yes" : "no");
}

define_command(spw_status, spw_status_handler, "Read status register from node", SPW_CMDS);

enum dump_format {
	DUMP_FORMAT_HEX,
	DUMP_FORMAT_ASCII
};

static void spw_dump_handler(int nb_params, char **params)
{
	uint32_t value;
	enum dump_format format = DUMP_FORMAT_HEX;

	if (nb_params > 1) {
		printf("spw_dump <format:ascii|hex>");
		return;
	}

	if (nb_params != 0) {
		if (strcmp("hex", params[0]) == 0) {
			format = DUMP_FORMAT_HEX;
		} else if (strcmp("ascii", params[0]) == 0) {
			format = DUMP_FORMAT_ASCII;
		} else {
			printf("Wrong format specifier");
		}
	}

	for (;;) {
		if (spw_node_status_data_available_read()) {
			value = spw_node_fifo_r_read();
			switch (format) {
				case DUMP_FORMAT_HEX:
					printf("%#lx ", value);
					break;
				case DUMP_FORMAT_ASCII:
					printf("%c", (char)value);
					break;
				default:
					break;
			}
		}
	}
}

define_command(spw_dump, spw_dump_handler, "Dump received input while it arrives. Does not return.", SPW_CMDS);
