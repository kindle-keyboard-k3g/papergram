#include "test_framework.h"

#include "../src/domain/value_objects.h"
#include "../src/graphics/canvas.h"

#include <cstdio>
#include <fstream>
#include <string>

TEST(canvas_clears_and_writes_pixels) {
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    canvas.setPixel(ScreenCoordinate(12, 24), GrayscaleColor::BLACK);

    ASSERT_EQ(GrayscaleColor::BLACK,
              canvas.pixelAt(ScreenCoordinate(12, 24)));
    ASSERT_EQ(GrayscaleColor::WHITE,
              canvas.pixelAt(ScreenCoordinate(13, 24)));
}

TEST(canvas_draws_lines_and_rectangles) {
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    canvas.drawLine(ScreenCoordinate(2, 2), ScreenCoordinate(6, 2),
                    GrayscaleColor::BLACK);
    canvas.drawRect(BoundingBox(3, 3, 5, 5), GrayscaleColor::BLACK);
    canvas.fillRect(BoundingBox(10, 10, 11, 11), GrayscaleColor::DARK_GRAY);

    ASSERT_EQ(GrayscaleColor::BLACK, canvas.pixelAt(ScreenCoordinate(4, 2)));
    ASSERT_EQ(GrayscaleColor::BLACK, canvas.pixelAt(ScreenCoordinate(3, 4)));
    ASSERT_EQ(GrayscaleColor::DARK_GRAY,
              canvas.pixelAt(ScreenCoordinate(10, 10)));
}

TEST(canvas_inverts_only_the_requested_rectangle) {
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    canvas.setPixel(ScreenCoordinate(4, 4), GrayscaleColor::BLACK);
    canvas.setPixel(ScreenCoordinate(6, 6), GrayscaleColor::DARK_GRAY);
    canvas.invertRect(BoundingBox(4, 4, 5, 5));

    ASSERT_EQ(GrayscaleColor::WHITE,
              canvas.pixelAt(ScreenCoordinate(4, 4)));
    ASSERT_EQ(GrayscaleColor::BLACK,
              canvas.pixelAt(ScreenCoordinate(5, 5)));
    ASSERT_EQ(GrayscaleColor::DARK_GRAY,
              canvas.pixelAt(ScreenCoordinate(6, 6)));
}

TEST(canvas_clips_rectangles_at_display_bounds) {
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    canvas.fillRect(-4, -3, 1, 1, GrayscaleColor::BLACK);

    ASSERT_EQ(GrayscaleColor::BLACK,
              canvas.pixelAt(ScreenCoordinate(0, 0)));
    ASSERT_EQ(GrayscaleColor::BLACK,
              canvas.pixelAt(ScreenCoordinate(1, 1)));
    ASSERT_EQ(GrayscaleColor::WHITE,
              canvas.pixelAt(ScreenCoordinate(2, 2)));
}

TEST(canvas_blits_bitmap_text) {
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    canvas.blitChar(ScreenCoordinate(8, 16), 'A', GrayscaleColor::BLACK);
    canvas.blitText(ScreenCoordinate(16, 16), "Hi", GrayscaleColor::BLACK);

    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(8, 16, 15, 31)));
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(16, 16, 31, 31)));
}

TEST(canvas_blits_punctuation_glyphs) {
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    canvas.blitText(ScreenCoordinate(8, 16), "+[]()/%", GrayscaleColor::BLACK);

    // Each character is 8x16, total width 7 * 8 = 56
    ASSERT_TRUE(canvas.hasNonWhitePixel(BoundingBox(8, 16, 63, 31)));
}

TEST(canvas_exports_a_valid_ppm_frame) {
    const std::string filename = "/tmp/kindle-canvas-test.ppm";
    Canvas canvas;
    canvas.clear(GrayscaleColor::WHITE);
    canvas.setPixel(ScreenCoordinate(0, 0), GrayscaleColor::BLACK);
    canvas.saveToPpm(filename);

    std::ifstream input(filename, std::ios::binary);
    std::string header;
    std::getline(input, header);
    ASSERT_STR_EQ("P6", header);
    std::getline(input, header);
    ASSERT_STR_EQ("600 800", header);
    std::getline(input, header);
    ASSERT_STR_EQ("255", header);
    ASSERT_TRUE(input.good());
    input.close();
    std::remove(filename.c_str());
}
