/*
 * main.c
 *
 *  Created on: Dec 5, 2014
 *      Author: baoke
 */

#include "h3c.h"

int success_handler()
{
	printf("succeed\n");
	daemon(0, 0);
}

int failure_hander()
{
	printf("failed\n");
}

int main(int argc, char **argv)
{
	/*
	 * Check privilege
	 */
	if (0 != geteuid())
		exit(-1);

	/*
	 * Check arguments
	 */
	if (4 != argc)
		exit(-1);

	h3c_init(argv[1], argv[2], argv[3]);

	h3c_start();

	for(;;)
	{
		h3c_response(success_handler, failure_hander);
	}

	return 0;
}
