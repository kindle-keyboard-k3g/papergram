# Domain Layer Specification

The domain layer (`src/domain/`) models all core entities and value objects representing user identity, conversations, messages, and layout geometry. It contains **zero platform dependencies** and strictly complies with **Object Calisthenics**:
- All domain primitives are encapsulated in distinct value objects (`PhoneNumber`, `AuthCode`, `ChatId`, `MessageId`, etc.).
- Collections are first-class objects (`ChatList`, `MessageHistory`).
- Entities expose behaviors rather than naked setters.
- Classes are kept under 100 lines and methods under 15 lines with at most one indent level.

---

## 1. Value Objects (`src/domain/value_objects.h`)

### `GrayscaleColor`
Enum class defining the strict 4-tone grayscale palette honoring the Nintendo Game Boy aesthetic and E-Ink Pearl physical response:
- `WHITE = 0xFF`: Background color for screens and text entry boxes.
- `LIGHT_GRAY = 0xAA`: Secondary background, toast card fill, incoming bubble fill.
- `DARK_GRAY = 0x55`: Inner borders, timestamps, muted subtitles, dividers.
- `BLACK = 0x00`: Text glyphs, active selections, outer borders.

### `ScreenCoordinate`
Represents an immutable 2D Cartesian integer coordinate `(x, y)` clamped to the 600x800 display area.

### `BoundingBox`
Represents an axis-aligned rectangular region bounded by `(x1, y1)` to `(x2, y2)`.
- Enforces invariant: `x1 <= x2` and `y1 <= y2`.
- Provides `width()`, `height()`, `contains(ScreenCoordinate)`, and `overlaps(BoundingBox)`.
- Used extensively by the graphics subsystem for partial refresh dirty regions.

### `PhoneNumber`
Encapsulates an international E.164 phone number:
- Enforces format: starts with `+` followed by 7 to 15 decimal digits.
- Throws `std::invalid_argument` upon malformed strings.

### `AuthCode`
Represents a Telegram SMS or app login verification code:
- Enforces invariant: exactly 5 decimal digits `[0-9]{5}`.
- Rejects non-numeric characters or incorrect lengths.

### `ChatId`
Encapsulates a 64-bit Telegram peer identifier (user, chat, or channel ID).

### `MessageId`
Encapsulates a unique 32-bit or 64-bit message identifier within a dialogue.

### `MessageText`
Encapsulates user message body text:
- Validates that text is non-empty and well-formed UTF-8.
- Provides character count and length inspection.

---

## 2. Entities & Collections

### `Message` (`src/domain/message.h`)
Represents a single chat message exchanged via MTProto:
- Attributes: `MessageId id`, `ChatId chatId`, `MessageText text`, `uint32_t timestamp`, `bool isOutgoing`.
- Immutable after construction.
- Exposes formatting helpers for display formatting on e-ink message bubbles.

### `MessageHistory` (`src/domain/message.h`)
First-class collection wrapping `std::vector<Message>`:
- Encapsulates chronological insertion, sorting, and pagination.
- Provides `slice(size_t offset, size_t limit)` for screen pagination.
- Prevents leaking internal collection mutation to UI screens.

### `Chat` (`src/domain/chat.h`)
Represents a Telegram dialog (direct peer, group, or channel):
- Attributes: `ChatId id`, `std::string title`, `std::string lastMessage`, `uint32_t lastMessageTime`, `uint32_t unreadCount`.
- Exposes methods for updating the unread badge and snippet without exposing raw struct fields.

### `ChatList` (`src/domain/chat.h`)
First-class collection wrapping `std::vector<Chat>`:
- Supports cursor navigation: `selectNext()`, `selectPrevious()`, with wrap-around semantics.
- Manages active selection index for D-pad navigation.
- Provides page calculation (`chatsPerPage = 8`) for 600x800 e-ink presentation.
