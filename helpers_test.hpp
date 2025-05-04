#pragma once

// is_space_char
static_assert(is_space_char(' '));
static_assert(is_space_char('\t'));
static_assert(is_space_char('\n'));
static_assert(is_space_char('\v'));
static_assert(is_space_char('\f'));
static_assert(is_space_char('\r'));
static_assert(!is_space_char('a'));
static_assert(!is_space_char('1'));

static_assert(contains_only_whitespaces("   "));
static_assert(contains_only_whitespaces(" \t\n"));
static_assert(contains_only_whitespaces(" \t\n\r"));
static_assert(contains_only_whitespaces(""));
static_assert(!contains_only_whitespaces("a"));
static_assert(!contains_only_whitespaces("a "));

static_assert(!is_comment_line("a #$"));
static_assert(!is_comment_line("1as"));
static_assert(is_comment_line("#some text"));
static_assert(is_comment_line("# some text"));
static_assert(!is_comment_line("2354 # some text"));