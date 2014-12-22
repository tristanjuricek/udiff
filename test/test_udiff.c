#include <check.h>
#include <stdlib.h>
#include <udiff.h>

// This is a basic minimial change just to see if the basic plumbing works
const char *const SIMPLE_CHANGE =
    "--- README\t2014-10-23 12:23:34.123456789 +0100\n"
    "+++ READTHIS\t2014-10-23 12:23:34.123456789 +0100\n"
    "@@ -1,2 +1,3 @@\n"
    " This is a common line\n"
    "-Line 1\n"
    "+Line 1 was boring\n"
    "+Line 2 has more!\n";

START_TEST(parse_simple_change)
{
    udiff_handle handle = NULL;
    int err = 0;

    err = udiff_parse(SIMPLE_CHANGE, &handle);

    ck_assert_int_eq(err, 0);
    ck_assert_ptr_ne(handle, NULL);

    ck_assert_str_eq(udiff_from_filename(handle, 0), "README");
    ck_assert_str_eq(udiff_to_filename(handle, 0), "READTHIS");

    udiff_free(handle);
}
END_TEST

// This is a single git style change for a single file
const char *const GIT_CHANGE =
    "diff --git a/yaggapp/src/main/java/com/tristanjuricek/yaggapp/component/InlineFileDiff.java b/yaggapp/src/main/java/com/tristanjuricek/yaggapp/component/InlineFileDiff.java\n"
    "index 2647e45..bd34ad6 100644\n"
    "--- a/yaggapp/src/main/java/com/tristanjuricek/yaggapp/component/InlineFileDiff.java\n"
    "+++ b/yaggapp/src/main/java/com/tristanjuricek/yaggapp/component/InlineFileDiff.java\n"
    "@@ -1,16 +1,28 @@\n"
    " package com.tristanjuricek.yaggapp.component;\n"
    " \n"
    "+import com.sun.javafx.css.converters.FontConverter;\n"
    " import com.tristanjuricek.yaggapp.model.DiffHunk;\n"
    "+import javafx.css.CssMetaData;\n"
    "+import javafx.css.StyleableObjectProperty;\n"
    "+import javafx.css.StyleableProperty;\n"
    " import javafx.scene.control.Control;\n"
    " import javafx.scene.control.Skin;\n"
    "+import javafx.scene.control.SkinBase;\n"
    "+import javafx.scene.text.Font;\n"
    " \n"
    " /**\n"
    "  * Renders the diff of a single file in the page.\n"
    "+ *\n"
    "+ * TODO is there a way to fetch defaults in a more global way?\n"
    "  */\n"
    " public class InlineFileDiff extends Control {\n"
    " \n"
    "     private DiffHunk diffHunk;\n"
    " \n"
    "+    // The font of the diff content, typically a monospaced font, can be\n"
    "+    // customized\n"
    "+    private StyleableObjectProperty<Font> contentFont;\n"
    "+\n"
    "     @Override\n"
    "     protected Skin<?> createDefaultSkin() {\n"
    "         return new InlineFileDiffSkin(this);\n"
    "@@ -26,5 +38,19 @@ public class InlineFileDiff extends Control {\n"
    " \n"
    "     private static class StyleableProperties {\n"
    " \n"
    "+        private static final CssMetaData<InlineFileDiff, Font> CONTENT_FONT =\n"
    "+                new CssMetaData<InlineFileDiff, Font>(\"-fx-inline-file-diff-content-font\",\n"
    "+                        FontConverter.getFontConverter(), Font.font(\"Menlo\", 12)) {\n"
    "+                    @Override\n"
    "+                    public boolean isSettable(InlineFileDiff styleable) {\n"
    "+\n"
    "+                        return false;\n"
    "+                    }\n"
    "+\n"
    "+                    @Override\n"
    "+                    public StyleableProperty<Font> getStyleableProperty(InlineFileDiff styleable) {\n"
    "+                        return null;\n"
    "+                    }\n"
    "+                };\n"
    "     }\n"
    " }\n";

START_TEST(single_git_style)
{
    udiff_handle handle = NULL;
    int err = udiff_parse(GIT_CHANGE, &handle);

    ck_assert_int_eq(err, 0);

    const char *from_filename = udiff_from_filename(handle, 0);
    ck_assert_str_eq(from_filename, "yaggapp/src/main/java/com/tristanjuricek/yaggapp/component/InlineFileDiff.java");

    const char *to_filename = udiff_to_filename(handle, 0);
    ck_assert_str_eq(to_filename, "yaggapp/src/main/java/com/tristanjuricek/yaggapp/component/InlineFileDiff.java");

    udiff_free(handle);
}
END_TEST

// If we don't think the line stars with a known prefix, then punt
const char *const BAD_HEADER =
    "what the hell is this\n"
    "--- README\t2014-10-23 12:23:34.123456789 +0100\n"
    "+++ README\t2014-10-23 12:23:34.123456789 +0100\n"
    "@@ -1,2 +1,3 @@\n"
    " This is a common line\n"
    "-Line 1\n"
    "+Line 1 was boring\n"
    "+Line 2 has more!\n";

START_TEST(illegal_header_error)
{
    udiff_handle handle = NULL;
    int err = 0;

    err = udiff_parse(BAD_HEADER, &handle);

    ck_assert_int_ne(err, 0);
    ck_assert_ptr_eq(handle, NULL);

    const char * msg = udiff_error_description(err);

    ck_assert_ptr_ne(msg, NULL);
}
END_TEST

Suite * udiff_suite(void)
{
    Suite *s;
    TCase *tc_main;

    s = suite_create("udiff");

    tc_main = tcase_create("main");

    tcase_add_test(tc_main, parse_simple_change);
    tcase_add_test(tc_main, single_git_style);
    tcase_add_test(tc_main, illegal_header_error);
    suite_add_tcase(s, tc_main);

    return s;
}


int main(void)
{
    int n_failed = -1;
    Suite *s;
    SRunner *runner;

    s = udiff_suite();
    runner = srunner_create(s);

    srunner_run_all(runner, CK_NORMAL);
    n_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (n_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}