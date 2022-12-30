#ifndef __DDR2_CONFIG_H
#define __DDR2_CONFIG_H

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * DDR2 info
 */
/* DDR2 paramters */
#define DDR_ROW 13  /* ROW : 12 to 14 row address */
#define DDR_COL 10  /* COL :  8 to 10 column address */
#define DDR_ROW1 13  /* ROW : 12 to 14 row address */
#define DDR_COL1 10  /* COL :  8 to 10 column address */

#define DDR_BANK8 1   /* Banks each chip: 0-4bank, 1-8bank */
#define DDR_CL 5   /* CAS latency: 1 to 7 */

/*
 * DDR2 controller timing1 register
 */
#define DDR_tRAS DDR__ns(45)  /* tRAS: ACTIVE to PRECHARGE command period to the same bank. */
#define DDR_tRTP DDR__ps(7500)   /* 7.5ns READ to PRECHARGE command period. */
#define DDR_tRP  DDR__ps(12500)  /* tRP: PRECHARGE command period to the same bank */
#define DDR_tRCD DDR__ps(12500)  /* ACTIVE to READ or WRITE command period to the same bank. */
#define DDR_tRC  DDR__ps(57500)  /* ACTIVE to ACTIVE command period to the same bank.*/
#define DDR_tRRD DDR__ns(10)  /* ACTIVE bank A to ACTIVE bank B command period. */
#define DDR_tWR  DDR__ns(15)  /* WRITE Recovery Time defined by register MR of DDR2 memory */
#define DDR_tWTR DDR__ps(7500)  /* WRITE to READ command delay. */
/*
 * DDR2 controller timing2 register
 */
#define DDR_tRFC DDR__ps(127500) /* ps,  AUTO-REFRESH command period. */
#define DDR_tXP DDR__tck(2)    /* tCK EXIT-POWER-DOWN to next valid command period: 1 to 8 tCK. */
#define DDR_tMRD DDR__tck(2)   /* unit: tCK. Load-Mode-Register to next valid command period: 1 to 4 tCK */

/* new add */
#define DDR_BL   8   /* Burst length: 3 - 8 burst, 2 - 4 burst , 1 - 2 burst*/

#define DDR_RL    DDR__tck(5)
#define DDR_WL    DDR__tck(4)

#define DDR_tCCD DDR__tck(2)      /* CAS# to CAS# command delay , tCK*/

#define DDR_tCKE DDR__tck(3)      /* CKE minimum pulse width, tCK */

#define DDR_tXARD DDR__tck(2)     /* DDR2 only: Exit active power down to read command , tCK*/
#define DDR_tXARDS DDR__tck(8)    /* DDR2 only: Exit active power down to read command (slow exit/low power mode), tCK */

#define DDR_tCKESR  DDR__tck(3)
#define DDR_tXSNR   (DDR_tRFC + DDR__ns(10))
#define DDR_tXARD   DDR__tck(2)
#define DDR_tXARDS  DDR__tck(8)
#define DDR_tXSRD   DDR__tck(200)
#define DDR_tFAW    DDR__ns(45)     /////////debug

/*
 * DDR2 controller refcnt register
 */
#define DDR_tREFI	DDR__ns(7800)	/* Refresh period: ns */

#define DDR_CLK_DIV 1    /* Clock Divider. auto refresh
						  *	cnt_clk = memclk/(16*(2^DDR_CLK_DIV))*/

#endif /* __DDR2_CONFIG_H */