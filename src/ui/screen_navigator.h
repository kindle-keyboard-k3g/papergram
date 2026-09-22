#ifndef KINDLE_UI_SCREEN_NAVIGATOR_H
#define KINDLE_UI_SCREEN_NAVIGATOR_H

#include "confirmation_dialog.h"
#include "kindle_menu.h"
#include "screen.h"
#include "../domain/value_objects.h"
#include <functional>
#include <memory>
#include <optional>
#include <vector>

/**
 * @brief Coordinates screen transitions, menus, locking, and confirmation dialogs.
 */
class ScreenNavigator {
public:
    /** @brief Creates an uninitialized screen navigator. */
    ScreenNavigator();

    /**
     * @brief Takes ownership of the primary application screens.
     * @param login Login screen shown before authorization.
     * @param chat_list Screen displaying available conversations.
     * @param conversation Screen displaying the active conversation.
     */
    void setScreens(std::unique_ptr<IScreen> login,
                    std::unique_ptr<IScreen> chat_list,
                    std::unique_ptr<IScreen> conversation);

    /**
     * @brief Takes ownership of the screen shown while the device is locked.
     * @param screensaver Screen used for the locked state.
     */
    void setScreensaver(std::unique_ptr<IScreen> screensaver);

    /** @brief Switches to the login screen unless the device is locked. */
    void showLogin();

    /** @brief Switches to the chat-list screen unless the device is locked. */
    void showChatList();

    /**
     * @brief Selects and prepares a conversation screen.
     * @param chat_id Identifier of the conversation to open.
     * @param chat_title Title displayed by the conversation screen.
     */
    void openConversation(const ChatId& chat_id, const std::string& chat_title);

    /** @brief Locks the device and preserves the previously active screen. */
    void lockScreen();

    /** @brief Unlocks the device and restores the previously active screen. */
    void unlockScreen();

    /**
     * @brief Reports whether the navigator is showing the locked state.
     * @return True when the device is locked.
     */
    bool isLocked() const;

    /**
     * @brief Returns the active conversation identifier, when one is selected.
     * @return Active chat identifier or std::nullopt outside a conversation.
     */
    std::optional<ChatId> activeConversationId() const;

    /**
     * @brief Reports whether the contextual menu is open.
     * @return True when menu input is being handled.
     */
    bool isMenuOpen() const;

    /**
     * @brief Reports whether the exit confirmation dialog is open.
     * @return True when dialog input is being handled.
     */
    bool isConfirmDialogOpen() const;

    /** @brief Opens the contextual menu for the active screen. */
    void openMenu();

    /** @brief Closes the contextual menu if it is open. */
    void closeMenu();

    /**
     * @brief Registers a callback for the menu's refresh action.
     * @param on_refresh Callback invoked when refresh is selected.
     */
    void setRefreshCallback(std::function<void()> on_refresh);

    /**
     * @brief Registers a callback for confirmed application exit.
     * @param on_exit Callback invoked after exit is confirmed.
     */
    void setExitCallback(std::function<void()> on_exit);

    /**
     * @brief Renders the active screen and any open overlays.
     * @param canvas Destination canvas.
     */
    void render(Canvas& canvas);

    /**
     * @brief Routes an input event to the active overlay or screen.
     * @param event Input event to dispatch.
     */
    void handleInput(const InputEvent& event);

    /**
     * @brief Returns the currently active screen.
     * @return Active screen pointer, or null when none is configured.
     */
    IScreen* currentScreen() const;

private:
    struct NavigationState {
        std::unique_ptr<IScreen> login;
        std::unique_ptr<IScreen> chat_list;
        std::unique_ptr<IScreen> conversation;
        std::unique_ptr<IScreen> screensaver;
        IScreen* active_screen = nullptr;
        IScreen* previous_active_screen = nullptr;
        std::optional<ChatId> active_conversation_id{std::nullopt};
        bool locked = false;
        std::function<void()> refresh_cb;
        std::function<void()> exit_cb;
        ui::ConfirmationDialog confirm_dialog;
    };

    NavigationState state_;
    ui::KindleMenu menu_;

    void populateMenu();
};

#endif
