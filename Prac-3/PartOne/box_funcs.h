/* box_funcs.c
   server part of our rpc demo
   
   compile with other rpcgen-ed files
   cc box_funcs.c box_svc.c box_xdr.c -o box_svc
*/ 

#include "box.h"

/* some worker procedures */
float surface(float l, float w, float h)
{
	return 2*(l*w + l*h + w*h);
}

float volume(float l, float w, float h)
{
	return l*w*h;
}

/* the remote procedure */
box_results * box_calc_1_svc( dimensions *dims, struct svc_req *rqstp)
{
	static box_results res;
	/* this res structure needs to be static as we return a pointer to it*/
	
	/* fill in the bits */
	res.surface=surface(dims->length, dims->width, dims->height);
	res.volume=volume(dims->length, dims->width, dims->height);
	
	/* ship off the result */
	return &res;
}