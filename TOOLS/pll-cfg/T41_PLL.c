#include <stdio.h>
#include <stdlib.h>

static unsigned int pll_want = 1300;

int main(int argc, char *argv[])
{
	int get_ok = 0;
	unsigned int m = 0, n = 0, od1 = 0, od0 = 1, pllrg = 0, pllrg_n = 0;
	unsigned int fref = 24, pllout = 0, fvco = 0, reg = 0, pllrg_n_re = 0;

/*	PLLM = m;//乘法器
	PLLN = n;//除法器
	PLLOD = od0;//分频器
	PLLOD1 = od1;//分频器1
	FVCO = 24 * PLLM * 2/PLLN;
	PLLOUT = FVCO / PLLOD * PLLOD1;

	PLLRG  //频率过滤器
		000: bypass 	100: 30-50M 
		001: 7-11M 	101: 50-80M 
		010: 11-18M 	110: 80M-130M 
		011: 18-30M 	111: 130M-200M
*/
	if(argc > 2)
	{
		printf("such as: ./pll_get 1500\n");
		return 0;
	}else if(argc == 2)
	{
		pll_want = (unsigned int)atoi(argv[1]);
		printf("want get %dMHz\n", pll_want);
	}else{
		printf("want get %dMHz\n", pll_want);
	}
	for (m = 0; m < 512; m++) {
		for (n = 0; n <= 23; n++) {
			for (od0 = 1; od0 <= 7; od0++) {
				for (od1 = 0; od1 <= 15; od1++) {
					fvco = fref * (m + 1) * 2 / (n + 1);
					pllout = fvco / (1 << od0) / (od1 + 1);
					pllrg_n = 24 / (n + 1);
					pllrg_n_re = 24 % (n + 1);
					if((pllrg_n >= 0) && (pllrg_n < 7))
						pllrg = 0;
					else if((pllrg_n >= 7) && (pllrg_n < 11))
						pllrg = 1;
					else if((pllrg_n >= 11) && (pllrg_n < 18))
						pllrg = 2;
					else
						pllrg = 3;
					if((pll_want == pllout) && (fvco <= 6000) && (fvco >= 3000) && (pllrg_n_re == 0)) {
						printf("---M= %d N=%d OD0=%d OD1=%d pllrg=%d pllout = %d\n", m, n, od0, od1, pllrg, pllout);
						reg = m << 20 | n << 14 | od0 << 11 | od1 << 7 | pllrg << 4 | 0xd; 
						printf("reg 0x10000010 data is 0x%08x\n", reg);
						get_ok = 1;
						break;
					} else {
						continue;
					}
				}
			}
		}
	}
	if(!get_ok)
		printf("The frequency cannot be set!\n");
	return 0;
}
