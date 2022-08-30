/* box_main.c
   does calculations about boxes using remote procedure calls

   this is the client code
   compile it with the rpcgen-ed code

   cc box_main.c box_clnt.c box_xdr.c -o box_client
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "box_2.h"

/* print out the box details */
void print_box(float l, float w, float h, float sa, float v, float m)
{
	printf("A box of dimensions %f x %f x %f\n", l, w, h);
	printf("Has a surface area of %f\n", sa);
	printf("a mass of %f\n", m);
	printf("and a volume of %f\n", v);

	return;
}

void print_mail(float c)
{
	printf("The cost of postage is %f\n", c);

	return;
}

int main(int ac, char **av)
{
	CLIENT *c_handle;
	dimensions dims;
	box_results *res;
	float* mailRes;
	mail_dims mail_dims;
	float mass;

	/* everything comes off the command line so check its there */
	if (ac != 6)
	{
		fprintf(stderr, "Usage: %s remote_host length width height mass\n",
				av[0]);
		/* Changed to accept mass.*/
		exit(EXIT_FAILURE);
	}

	/* grab the dims and shove them in the struct */
	dims.length = atof(av[2]);
	dims.width = atof(av[3]);
	dims.height = atof(av[4]);
	mass = atof(av[5]);

	/* create a connection handle to make rpc’s */
	c_handle = clnt_create(av[1], RPC_BOX, BOXVERSION2, "udp");
	if (c_handle == NULL)
	{
		clnt_pcreateerror(av[1]);
		exit(EXIT_FAILURE);
	}

	/* make the rpc */
	res = box_calc_2(&dims, c_handle);
	if (res == NULL)
	{
		clnt_perror(c_handle, av[1]);
		exit(EXIT_FAILURE);
	}
	mail_dims.volume = res->volume;
	mail_dims.mass = mass;
	mailRes = mail_calc_2(&mail_dims, c_handle);
	if (mailRes == NULL)
	{
		/*change av? might be right.*/
		clnt_perror(c_handle, av[1]);
		exit(EXIT_FAILURE);
	}

	/* print out the results */
	print_box(dims.length, dims.width, dims.height, res->surface, res->volume, mass);

	print_mail(*mailRes);

	/* free up the RPC handle */
	clnt_destroy(c_handle);

	return 0;
}