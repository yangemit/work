#include <stdio.h>

#include "../include/AntiCopy_Verify.h"

int main(void)
{
	int ret = -1;

#ifdef ANTICOPY_PREV_STRING
	ret = AntiCopy_Verify_PrevStr(ANTICOPY_PREV_STRING);
#else
	ret = AntiCopy_Verify();
#endif

	/* TODO */
	if (ret < 0) {
		printf("ERROR(%s): Anticopy_verify failed! \n", __func__);
		return -1;
	}
	printf("INFO(%s): Anticopy_verify OK! \n", __func__);

	return 0;
}
