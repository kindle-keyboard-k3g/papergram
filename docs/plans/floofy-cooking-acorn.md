# Implementation Plan: Wire Up UI with Real Telegram Client & Async Worker (TDD)

## Context
Papergram currently relies on hardcoded fixture strings in `TelegramClient` (`"Kindle Developer Group"`, `"Welcome to Kindle Telegram!"`, etc.) and mock navigation in `LoginScreen` that skips real authentication. Furthermore, Kindle hardware requires a responsive UI event loop without synchronous blocking network calls freezing e-ink screen refresh.

This plan outlines replacing all hardcoded mocks with real MTProto RPC decoding and wiring `LoginScreen`, `ChatListScreen`, and `ConversationScreen` through a clean, non-blocking `AsyncWorker` using test-driven development (TDD) with strict vertical slices.

---

## Architecture & Seams Under Test

### 1. Seam 1: `mtproto::TelegramClient` (RPC & TL Deserialization)
- **Files**: `src/mtproto/telegram_client.h`, `src/mtproto/telegram_client.cpp`
- **Seam**: `INetworkTransport` boundary.
- **Responsibilities**:
  - Serialize MTProto RPC requests using `TlWriter`.
  - Parse real TL-encoded responses using `TlReader` for:
    - `getDialogs`: Decodes TL vectors of dialogs/chats into domain `ChatList`.
    - `getHistory`: Decodes TL vectors of messages into domain `MessageHistory`.
    - `requestAuthCode` / `sendCode`: Decodes `auth.sentCode` or detects error codes.
    - `signIn`: Decodes `auth.authorization` and saves session, or flags 2FA required.
    - `checkPassword`: Decodes `auth.authorization` with 2FA password.
    - `sendMessage`: Decodes `updates` / `updateShortSentMessage`.
  - Removes all hardcoded fixture appends.

### 2. Seam 2: `hal::AsyncWorker` (Concurrency & UI Callbacks)
- **Files**: `src/hal/async_worker.h`, `src/hal/async_worker.cpp`
- **Seam**: Work item queue & UI completion queue.
- **Responsibilities**:
  - Thread-safe background task queue (`std::thread`, `std::mutex`, `std::condition_variable`).
  - Thread-safe UI callback queue drained by the main event loop via `drainUiCallbacks()`.
  - Ensures no background thread touches `Canvas`, `IScreen`, or e-ink ioctl directly.

### 3. Seam 3: UI Screens & Navigator
- **Files**:
  - `src/ui/login_screen.h`, `src/ui/login_screen.cpp`
  - `src/ui/chat_list_screen.h`, `src/ui/chat_list_screen.cpp`
  - `src/ui/conversation_screen.h`, `src/ui/conversation_screen.cpp`
  - `src/main.cpp`
- **Seam**: `IScreen` methods (`onEnter`, `handleInput`, `render`).
- **Responsibilities**:
  - `LoginScreen`: Real 3-step auth with 2FA, `BUSY` state rendering, input validation with `PhoneNumber`, error toast feedback.
  - `ChatListScreen`: Asynchronous `getDialogs` loading state, empty state rendering (`"No conversations found."`), network error toast.
  - `ConversationScreen`: Asynchronous `getHistory` loading, optimistic message append on send, background `sendMessage` RPC, error handling.
  - `main.cpp`: Integrates `AsyncWorker::drainUiCallbacks()` into the input polling loop.

---

## Vertical Slices & TDD Steps

### Slice 1: AsyncWorker Infrastructure (TDD)
1. **Red**: Write `tests/test_async_worker.cpp`:
   - `async_worker_executes_task_and_drains_ui_callback`
   - `async_worker_handles_multiple_tasks_in_order`
   - `async_worker_shuts_down_cleanly`
2. **Green**: Implement `src/hal/async_worker.h` and `src/hal/async_worker.cpp`.
3. **Verify**: Run `make test` and verify 3 tests pass.

### Slice 2: TelegramClient Real TL Decoding & Removal of Hardcoded Mocks (TDD)
1. **Red**: Write `tests/test_telegram_client_rpc.cpp`:
   - `telegram_client_decodes_dialogs_from_tl_response`
   - `telegram_client_decodes_empty_dialogs`
   - `telegram_client_returns_false_on_network_transport_failure`
   - `telegram_client_decodes_history_messages_from_tl_response`
   - `telegram_client_send_message_serializes_chat_and_text`
   - `telegram_client_auth_flow_parses_responses_and_persists_session`
2. **Green**:
   - Refactor `src/mtproto/telegram_client.cpp` to remove hardcoded `Chat` and `Message` additions.
   - Implement real TL response parsing with `TlReader` for dialogs, messages, and auth.
3. **Verify**: Run `make test` to ensure all existing tests and new RPC tests pass.

### Slice 3: LoginScreen Real 3-Step Auth with 2FA & Error Handling (TDD)
1. **Red**: Expand `tests/test_login_screen.cpp`:
   - `login_screen_submits_phone_and_transitions_to_code_on_success`
   - `login_screen_shows_error_toast_on_network_or_phone_failure`
   - `login_screen_submits_code_and_navigates_to_chat_list_on_success`
   - `login_screen_transitions_to_2fa_step_when_password_needed`
   - `login_screen_submits_password_and_authenticates`
2. **Green**:
   - Update `LoginScreen` in `src/ui/login_screen.h/.cpp` with `AsyncWorker` integration, 3-step auth state machine, `BUSY` / loading indicator, and error notifications.
3. **Verify**: Run `make test`.

### Slice 4: ChatListScreen Dynamic Loading, Empty & Error States (TDD)
1. **Red**: Update `tests/test_chat_list.cpp`:
   - `chat_list_screen_renders_loading_state_during_fetch`
   - `chat_list_screen_populates_real_dialogs_from_client`
   - `chat_list_screen_renders_empty_state_when_no_dialogs`
   - `chat_list_screen_shows_error_banner_on_fetch_failure`
2. **Green**:
   - Update `ChatListScreen` in `src/ui/chat_list_screen.h/.cpp` to trigger async fetch on `onEnter()`, handle `LOADING`, `READY`, `EMPTY`, and `ERROR` states.
3. **Verify**: Run `make test`.

### Slice 5: ConversationScreen Dynamic History & Send Message (TDD)
1. **Red**: Update `tests/test_conversation.cpp`:
   - `conversation_screen_loads_history_on_enter`
   - `conversation_screen_renders_empty_state_when_no_messages`
   - `conversation_screen_sends_message_via_client_optimistically`
   - `conversation_screen_shows_error_toast_when_send_fails`
2. **Green**:
   - Update `ConversationScreen` in `src/ui/conversation_screen.h/.cpp` to fetch history via `client.getHistory` and send messages asynchronously with `client.sendMessage`.
3. **Verify**: Run `make test`.

### Slice 6: Main Event Loop Integration & Verification
1. Update `src/main.cpp`:
   - Instantiate `AsyncWorker worker;`.
   - Pass `AsyncWorker&` to screens or navigator.
   - In `while (g_running)` loop: call `worker.drainUiCallbacks()`.
   - On exit: call `worker.stop()`.
2. Run full test suite (`make test`), ASan test suite (`make test-asan`), and native debug client build (`make client-debug`).
3. Run smoke test on host client (`./bin/kindle-telegram`) with fallback devices to verify frame generation.
