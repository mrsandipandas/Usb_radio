#ifndef _gprs_INCLUDED_
#define _gprs_INCLUDED_


//GPRS
extern unsigned char       G2num;                //Point to next GPRS buffer.
extern unsigned char       GPRSALARMACK;       //Acknowledge if alarm is sent by GPRS. Will be set to 0xFF when ack is received.

int gsm_int_gprs(void);
void gsm_gprs_process(void);
void gsm_gprs_process_ack(void);
void gsm_gprs_clear_buf(void);
void gprs_send_id(void);
int gsm_sem_sendgprs(char *gprsbuf, unsigned char resp);
void gprs_info(char nr);
char gprs_alarm(char *mdata);
void gprs_zsend(unsigned char d);

#endif
