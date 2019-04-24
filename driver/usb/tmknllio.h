#ifndef __TMKNLLIO__
#define __TMKNLLIO__

/*#ifndef __EXTERN
#ifdef __cplusplus
#define __EXTERN extern
#else
#define __EXTERN
#endif
#endif*/

#ifdef ASYNCHRONOUS_IO
//__EXTERN char cAsync[MAX_DEVICES];
extern char cAsync[MAX_DEVICES];
#endif

//__EXTERN unsigned int Block_out_in(struct tmk1553busb * dev, u16 *buf_out, u16 *buf_in);
extern unsigned int Block_out_in(struct tmk1553busb * dev, u16 *buf_out, u16 *buf_in);
#endif
