#include "test_framework.h"

#include "../src/domain/message.h"
#include "../src/graphics/canvas.h"
#include "../src/ui/popup_manager.h"

#include <chrono>
#include <string>
#include <vector>

namespace {

ui::PopupTimestamp timestamp(int milliseconds) {
    return ui::PopupTimestamp(std::chrono::milliseconds(milliseconds));
}

Message message(std::int64_t chat_id, bool outgoing = false) {
    return Message(MessageId(1), ChatId(chat_id), "Alice", "Hello", 10,
                   outgoing);
}

} // namespace

TEST(popup_manager_starts_with_default_state) {
    const ui::PopupManager manager;

    ASSERT_EQ(PopupPosition::BOTTOM, manager.position());
    ASSERT_EQ(PopupMode::VISUAL_STACK, manager.mode());
    ASSERT_EQ(static_cast<std::size_t>(0), manager.visibleCount());
    ASSERT_EQ(static_cast<std::size_t>(0), manager.pendingCount());
    ASSERT_FALSE(manager.needsRedraw());
    ASSERT_FALSE(manager.damageArea().has_value());
}

TEST(popup_manager_show_marks_redraw_and_computes_damage) {
    ui::PopupManager manager;
    const std::vector<std::string> lines = {"Line"};

    ASSERT_TRUE(manager.show("Title", lines,
                             ui::PopupDuration(std::chrono::milliseconds(100))));
    ASSERT_EQ(static_cast<std::size_t>(1), manager.visibleCount());
    ASSERT_EQ(static_cast<std::size_t>(0), manager.pendingCount());
    ASSERT_TRUE(manager.needsRedraw());
    ASSERT_TRUE(manager.damageArea().has_value());
    ASSERT_EQ(BoundingBox(30, 692, 570, 760), *manager.damageArea());

    manager.acknowledgeRedraw();
    ASSERT_FALSE(manager.needsRedraw());
}

TEST(popup_manager_renders_visible_popup_without_allocating_entries) {
    ui::PopupManager manager;
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    ASSERT_TRUE(manager.show("Title", {"Line"}));

    manager.render(canvas);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(30, 692, 570, 760)));
}

TEST(popup_manager_update_marks_damage_when_popup_expires) {
    ui::PopupManager manager;
    ASSERT_TRUE(manager.show(
        "Title", {"Line"}, ui::PopupDuration(std::chrono::milliseconds(100))));
    manager.acknowledgeRedraw();

    ASSERT_FALSE(manager.update(timestamp(99)));
    ASSERT_FALSE(manager.needsRedraw());
    ASSERT_TRUE(manager.update(timestamp(100)));
    ASSERT_TRUE(manager.needsRedraw());
    ASSERT_EQ(static_cast<std::size_t>(0), manager.visibleCount());
    ASSERT_EQ(BoundingBox(0, 0, 599, 799), *manager.damageArea());
}

TEST(popup_manager_filters_incoming_messages) {
    ui::PopupManager manager;
    const ChatId active_chat(7);

    ASSERT_FALSE(manager.notifyIncomingMessage(message(8, true), std::nullopt));
    ASSERT_FALSE(manager.notifyIncomingMessage(message(7), active_chat));
    ASSERT_FALSE(manager.notifyIncomingMessage(message(8), std::nullopt, true));
    ASSERT_TRUE(manager.notifyIncomingMessage(message(8), active_chat));
    ASSERT_EQ(static_cast<std::size_t>(1), manager.visibleCount());
}

TEST(popup_manager_uses_position_and_mode_settings) {
    ui::PopupManager manager;
    manager.setPosition(PopupPosition::TOP);
    manager.setMode(PopupMode::SEQUENTIAL_QUEUE);

    ASSERT_EQ(PopupPosition::TOP, manager.position());
    ASSERT_EQ(PopupMode::SEQUENTIAL_QUEUE, manager.mode());
    ASSERT_TRUE(manager.show("First", {"Line"}));
    ASSERT_TRUE(manager.show("Second", {"Line"}));
    ASSERT_EQ(static_cast<std::size_t>(1), manager.visibleCount());
    ASSERT_EQ(static_cast<std::size_t>(1), manager.pendingCount());
    ASSERT_EQ(BoundingBox(30, 36, 570, 104), *manager.damageArea());
}

TEST(popup_manager_render_does_nothing_when_no_popup_is_visible) {
    ui::PopupManager manager;
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);

    manager.render(canvas);

    ASSERT_FALSE(canvas.hasNonWhitePixel(BoundingBox(0, 0, 599, 799)));
}
