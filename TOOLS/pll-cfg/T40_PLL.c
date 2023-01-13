#include <stdio.h>
#include <stdlib.h>

static unsigned int pll_want = 1300;

int main(int argc, char *argv[])
{
	int get_ok = 0;
	unsigned int m = 0, n = 0, od1 = 2, od0 = 1;
	unsigned int fref = 24, pllout = 0, fvco = 0, reg = 0;

	/*	PLLM = m;16~2500:整数分屏 20~500:小数分屏 //乘法因子
		PLLN = n;1~63 //除法器
		PLL0D1 = od1;2~7//分频器1
		PLL0D0 = od0;1//分频器0
		FVCO = 24 * PLLM / PLLN;1250~5000
		PLLOUT = FVCO / PLL0D1 * PLL0D0;25~5000//输出时钟
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
	for (m = 1; m < 516; m++) {
		for (n = 1; n <= 63; n++) {
				for (od1 = 2; od1 <= 7; od1++) {
					fvco = fref * m  / n ;
					pllout = fvco / od1 ;
					if((pll_want == pllout) && (fvco <= 5000) && (fvco >= 1250)) {
						printf("---M= %d N=%d OD1=%d OD0=%d  pllout = %d\n", m, n, od1, od0, pllout);
						reg = m << 20 | n << 14 | od1 << 11 | od0 << 8; 
						printf("reg 0x10000010 data is 0x%08x\n", reg);
						get_ok = 1;
						break;
					} else {
						continue;
					}
				}
			
		}
	}
	if(!get_ok)
		printf("The frequency cannot be set!\n");
	return 0;
}
