# Task 5 Brief: ScreenNavigator & ConversationScreen Integration

## Goal
Wire active conversation tracking into `ScreenNavigator` (`src/ui/screen_navigator.h` and `src/ui/screen_navigator.cpp`) and ensure `openConversation(chat_id, chat_title)` passes the selected conversation to `ConversationScreen` (`src/ui/conversation_screen.h` and `src/ui/conversation_screen.cpp`), exposing `activeConversationId() const` so `PopupManager::notifyIncomingMessage` can detect messages for active vs inactive chats.

## Files
- Modify: `src/ui/screen_navigator.h`
- Modify: `src/ui/screen_navigator.cpp`
- Modify: `src/ui/conversation_screen.h`
- Modify: `src/ui/conversation_screen.cpp`
- Test: add tests in `tests/test_conversation.cpp` or `tests/test_popup_manager.cpp`

## Requirements
1. `ConversationScreen`:
   - Add/verify `void setChat(const ChatId& chat_id, const std::string& title);`
   - Add `const ChatId& chatId() const;`
2. `ScreenNavigator`:
   - Add `std::optional<ChatId> active_conversation_id_{std::nullopt};` to navigator state.
   - In `openConversation(const ChatId& chat_id, const std::string& chat_title)`:
     - Set `active_conversation_id_ = chat_id;`
     - Call `conversation_screen_->setChat(chat_id, chat_title);`
     - Transition to conversation screen.
   - In `showLogin()` and `showChatList()`:
     - Clear `active_conversation_id_ = std::nullopt;`
   - Add public getter: `std::optional<ChatId> activeConversationId() const;`
3. Object Calisthenics:
   - 0 `else` keywords.
   - Max 1 indent level per method.
   - Methods <= 15 lines, classes <= 100 lines.
   - Max 2 instance variables per class.

## Steps
1. Write failing tests.
2. Implement methods.
3. Verify with `make test` and `make test-asan`.
4. Commit:
   `git add src/ui/screen_navigator.* src/ui/conversation_screen.* tests/...`
   `git commit -m "feat(ui): wire activeConversationId in ScreenNavigator and setChat in ConversationScreen"`
