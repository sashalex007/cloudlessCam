#ifndef PRIVATE_VARIABLES_H
#define PRIVATE_VARIABLES_H

//wifi settings
extern const char *ssid;
extern const char *password;

//email info
extern const char *sender_name;
extern const char *sender_email;
extern const char *recipient_name1;
extern const char *recipient_email1;
extern const char *recipient_name2; //optional
extern const char *recipient_email2; //optional

//mailgun settings
extern const int host_port;
extern const char *host_name;
extern const char *host_email;
extern const char *host_password;
extern const char *host_domain;

#endif