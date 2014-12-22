// udiff.h
//
// Object model for unified diffs typically seen in the wild.
//
// A unified diff contains a header and a series of chunks.
//
// This attempts to be as flexible as possible to accept different kinds of
// unified diff formats, which are not really a standard but pretty close.
//
// Here's the best reference I could find:
// http://www.artima.com/forums/flat.jsp?forum=106&thread=164293
//
// The description is going to be reproduced here.
//
//
// The standard header is two lines, each line with the format:
//
//     indicator ' ' filename '\t' date ' ' time ' ' timezone
//
// * `indicator` is '---' for the old file, and '+++' for the new
// * `date` is `YYYY-MM-DD`
// * `time` is `hh:mm:ss.nnnnnnnnn` for a 24-hour clock
// * `timezone` is `'+'|'-'hhmm` where `hhmm` is hours and minutes east or west
//   of GMT/UTC
//
// The git diff is nearly the same thing, but has additional header lines,
// and does not give you the datetime. Review the help for git-diff for more
// details.
//
// Each chunk stars with a line that looks like this:
//
//     '@@ -' range ' +' range ' @@'
//
// Each `range` is either one unsigned decimal number or two separated by a
// comma. The first number is the start line of the chunk in the old or the
// new file. The second number is the chunk size in that file. It and the
// comma are omitted if the chunk size is 1. If the chunk size is 0, the
// first number is one lower than one would expect. It is the line number after
// which the chunk should be inserted or deleted; in all other cases it gives
// the first line number or the replaced range of lines).
//
// A chunk then continues with lines starting with ' ' (common line), '-' (only
// in old file), or '+' (only in new file). If the last line of a file doesn't
// end in a newline character, it is displayed with a newline character, and the
// following line in the chunk has the literal text (starting in the first
// column):
//
//     '\ No newline at end of file'
//
// When a file is deleted (rather than just made empty), the '+++' date is set
// to the epoch. Similary, when a file is created, the '---' date is set to the
// epoch.
//
#ifndef udiff_handle_H
#define udiff_handle_H 1

// The handle to a udiff_handle, used for most of the interface
typedef void* udiff_handle;

// Allocate a new udiff_handle reference based on a UTF-8 string buffer.
//
// Returns a non-zero number if there was an error, and handle should not be
// set (and thus, not freed).
int udiff_parse(const char* contents, udiff_handle *handle);

// Free the udiff structure
void udiff_free(udiff_handle udiff);

// Return a textual description of the error.
const char * udiff_error_description(int code);

int udiff_file_count(udiff_handle udiff);

// Get reference to the 'from filename' for the diff.
const char* udiff_from_filename(udiff_handle udiff, int file_idx);

// Get reference to the 'to filename' for the diff.
const char* udiff_to_filename(udiff_handle udiff, int file_idx);

int udiff_chunk_count(udiff_handle udiff, int file_idx);

int udiff_chunk_from_start(udiff_handle udiff, int file_idx, int chunk_idx);

int udiff_chunk_from_length(udiff_handle udiff, int file_idx, int chunk_idx);

int udiff_chunk_to_start(udiff_handle udiff, int file_idx, int chunk_idx);

int udiff_chunk_to_length(udiff_handle udiff, int file_idx, int chunk_idx);

int udiff_chunk_line_count(udiff_handle udiff, int file_idx, int chunk_idx);

const char * udiff_chunk_line(udiff_handle handle, int file_idx, int chunk_idx,
                              int lineno);

enum udiff_line_type {
    same,
    add,
    delete
};

enum udiff_line_type udiff_chunk_line_type(udiff_handle handle, int file_idx,
                                           int chunk_idx, int lineno);


#endif // udiff_handle_H