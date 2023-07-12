#ifndef EMAIL_H
#define EMAIL_H

#include <ESP_Mail_Client.h>

class Email {
    private:
        ESP_Mail_Session session;
        SMTPSession smtp;
        SMTP_Message message;
    public:
        explicit Email();
        void send(const int reset, int boot_count, int tries);
        void add_attachment(String& base64_string, int img_count);
};

#endif