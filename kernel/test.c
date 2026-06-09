#include <kernel/test.h>
#include <kernel/vga.h>
#include <stddef.h>

static void reset_color(void) {
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

static void print_section(const char* title) {
  terminal_writestring("\n");

  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
  terminal_writestring("== ");
  terminal_writestring(title);
  terminal_writestring(" ==\n");

  reset_color();
}

static void print_expected(const char* text) {
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
  terminal_writestring("Expected: ");
  terminal_writestring(text);
  terminal_writestring("\n");

  reset_color();
}

static void test_terminal_writestring(void) {
  print_section("terminal_writestring");

  terminal_writestring("This line was printed with terminal_writestring().\n");
  print_expected("A full sentence should appear above.");
}

static void test_terminal_putchar(void) {
  print_section("terminal_putchar");

  terminal_putchar('O');
  terminal_putchar('K');
  terminal_putchar('\n');

  print_expected("OK");
}

static void test_terminal_write(void) {
  print_section("terminal_write with explicit size");

  terminal_write("ABCDE-THIS-SHOULD-NOT-PRINT", 5);
  terminal_putchar('\n');

  print_expected("ABCDE");
}

static void test_newline(void) {
  print_section("newline: \\n");

  terminal_writestring("Line 1\n");
  terminal_writestring("Line 2\n");
  terminal_writestring("Line 3\n");

  print_expected("Three separate lines.");
}

static void test_tab(void) {
  print_section("tab: \\t");

  terminal_writestring("A\tB\tC\n");

  print_expected("A, B, and C should be spaced apart by tab stops.");
}

static void test_backspace_same_line(void) {
  print_section("backspace on same line: \\b");

  terminal_writestring("ABC\bD\n");

  print_expected("ABD");
}

static void test_backspace_multiple_chars(void) {
  print_section("multiple backspaces");

  terminal_writestring("HELLO\b\b\bP!\n");

  print_expected("HEP!");
}

static void test_backspace_at_line_start(void) {
  print_section("backspace at start of line");

  terminal_writestring("ABC\n\bZ\n");

  print_expected("ABZ if backspace moves to the previous line.");
}

static void test_carriage_return(void) {
  print_section("carriage return: \\r");

  terminal_writestring("ABC\rZ\n");

  print_expected("ZBC");
}

static void test_color_output(void) {
  print_section("VGA color output");

  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
  terminal_writestring("This should be light green on black.\n");

  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
  terminal_writestring("This should be light red on black.\n");

  terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE));
  terminal_writestring("This should be white text on a blue background.\n");

  reset_color();

  print_expected("Three different color styles.");
}

static void test_line_wrapping(void) {
  print_section("line wrapping");

  for (size_t i = 0; i < 85; i++) {
    terminal_putchar('X');
  }

  terminal_putchar('\n');

  print_expected("80 X characters should fill one row, then wrap to the next row.");
}

static void test_tab_near_end_of_line(void) {
  print_section("tab near end of line");

  for (size_t i = 0; i < 77; i++) {
    terminal_putchar('.');
  }

  terminal_writestring("\tT\n");

  print_expected("The T should not corrupt memory. It may wrap to the next line.");
}

static void test_terminal_clear_function(void) {
  print_section("terminal_clear");

  terminal_writestring("This text will disappear after terminal_clear().\n");
  terminal_clear();

  terminal_writestring("terminal_clear() worked if this is near the top-left.\n");
}

static void test_form_feed(void) {
  print_section("form feed: \\f");

  terminal_writestring("This text will disappear after form feed.\n");
  terminal_writestring("\f");

  terminal_writestring("Form feed worked if the screen was cleared.\n");
}

void run_vga_tests(void) {
  terminal_initialize();

  terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE));
  terminal_writestring("wigOSX Stage 2 VGA Test Suite\n");
  reset_color();

  terminal_writestring("These are visual tests. Read the Expected lines.\n");

  test_terminal_writestring();
  test_terminal_putchar();
  test_terminal_write();

  test_newline();
  test_tab();
  test_backspace_same_line();
  test_backspace_multiple_chars();
  test_backspace_at_line_start();
  test_carriage_return();

  test_color_output();
  test_line_wrapping();
  test_tab_near_end_of_line();

  /*
   * These clear the screen, so keep them near the end.
   */
  test_terminal_clear_function();
  test_form_feed();

  terminal_writestring("\nAll VGA visual tests finished.\n");
}