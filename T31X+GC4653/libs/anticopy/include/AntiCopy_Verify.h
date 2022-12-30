#ifndef __ANTICOPY_VERIFY_H__
#define __ANTICOPY_VERIFY_H__
/*
 *#define ANTICOPY_PREV_STRING "xxxxxxxxx"
 *#define PREV_LEN              32
 */

#ifndef ANTICOPY_PRIV_STRING
int AntiCopy_Verify(void);
#else
int AntiCopy_Verify_PrivStr(char *priv);
#endif

#endif /* __ANTICOPY_VERIFY_H__ */
