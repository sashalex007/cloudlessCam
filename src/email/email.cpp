
#include <ESP_Mail_Client.h>
#include "../read_battery/read_battery.h"
#include "private_variables.h"
#include "email.h"

Email::Email() {
    session.server.host_name = host_name;
    session.server.port = host_port;
    session.login.email = host_email;
    session.login.password = host_password;  // set to empty for no SMTP Authentication
    session.login.user_domain = host_domain;

    message.sender.name = sender_name;
    message.sender.email = sender_email;
    message.addRecipient(recipient_name1, recipient_email1);
    // message.addRecipient(recipient_name2, recipient_email2);
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
    message.html.charSet = "utf-8";
    message.html.content += "<br><br>";
}

void Email::add_attachment(String& base64_string, int img_count) {
    SMTP_Attachment att;
    message.html.content += "<div style='height:800; width:600'>";
    message.html.content += "<img style='transform:rotate(-90deg); filter: brightness(150%);' src=\"cid:image-" + toStringPtr(img_count) + "\" alt=\"cam image\"><br/><br/>";
    message.html.content += "</div><br>";
    att.descr.filename = toStringPtr(img_count) + ".jpeg";
    att.descr.mime = "image/jpeg";
    att.blob.size = base64_string.length() + 1000;                         // correct for jpeg corruption
    att.blob.data = reinterpret_cast<const uint8_t *>(&base64_string[0]);  // convert to uint8 array
    att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
    att.descr.content_encoding = Content_Transfer_Encoding::enc_base64;
    att.descr.content_id = "image-" + toStringPtr(img_count);
    message.addInlineImage(att);
    Serial.println("Attached");
}

void Email::send(const int reset, int boot_count, int tries) {
    if (tries < 3) {
        static char outstr[15];
        dtostrf(read_battery(), 3, 2, outstr);
        if (!reset) {
            message.subject = "Detection #" + toStringPtr(boot_count) + "  Batt: " + outstr;
        } else {
            message.subject = "Detection #" + toStringPtr(boot_count) + "  Batt: " + outstr + " RST";
        }
        smtp.connect(&session);
        // send or try again
        if (!MailClient.sendMail(&smtp, &message)) {
            Serial.println("Error sending Email, trying again" + smtp.errorReason());
            send(reset, boot_count, tries + 1);
        }
    } else {
        Serial.println("Sending email failed " + smtp.errorReason());
    }
}


