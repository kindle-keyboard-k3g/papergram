#ifndef KINDLE_UI_STATUS_HEADER_H
#define KINDLE_UI_STATUS_HEADER_H

#include "../graphics/canvas.h"
#include <string>

/**
 * @brief Renders the title, network state, battery level, and clock header.
 */
class StatusHeader {
public:
    /**
     * @brief Creates a header with an optional title.
     * @param title Text displayed at the left side of the header.
     */
    explicit StatusHeader(const std::string& title = "Telegram");

    /**
     * @brief Replaces the displayed title.
     * @param title New header title.
     */
    void setTitle(const std::string& title);

    /**
     * @brief Replaces the displayed network status text.
     * @param status Network status, such as `[Wi-Fi]` or `[3G]`.
     */
    void setNetworkStatus(const std::string& status);

    /**
     * @brief Updates the displayed battery percentage.
     * @param percentage Battery level to display.
     */
    void setBatteryLevel(int percentage);

    /**
     * @brief Draws the status header into a canvas.
     * @param canvas Destination canvas for the header.
     */
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
