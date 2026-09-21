#ifndef KINDLE_UI_STATUS_HEADER_H
#define KINDLE_UI_STATUS_HEADER_H

#include "../graphics/canvas.h"
#include <string>

class StatusHeader {
public:
    explicit StatusHeader(const std::string& title = "Telegram");

    void setTitle(const std::string& title);
    void setNetworkStatus(const std::string& status);
    void setBatteryLevel(int percentage);
    void render(Canvas& canvas) const;

private:
    struct HeaderData {
        std::string title;
        std::string network_status = "[Wi-Fi]";
        std::string battery_text = "[98%]";
    };

    HeaderData data_;
};

#endif
