#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "udiff.h"

// Errors
//
const char *const ERRORS[] = {
        // MALFORMED_HEADER
        "Unknown header line type: not from git and doesn't start with '---' "
                "or '+++' before a chunk section"
};

const int ERRORS_LEN = sizeof(ERRORS) / sizeof(ERRORS[0]);

// Start error codes at 1001
const int MALFORMED_HEADER = 1000 + 1;


int max(int a, int b) {
    return a > b ? a : b;
}

// The internal data.
//
// We split the original string into an array of strings, one per line,
// basically split on the '\n' character.
//
// From there, we can start figuring out how the files tend to be grouped.
//
struct udiff_file {
    int start_idx;

    // We flag certain files as "git"
    bool is_git;

    // Line indexes for the standard from and to headers. Everything ahead of
    // these indexes probably is some kind of git reference.
    int from_header_idx;
    int to_header_idx;

    // String copies of filenames
    char *from_filename;
    char *to_filename;
};


struct udiff {
    int num_lines;
    char **lines;

    int file_count;
    int file_capacity;
    struct udiff_file **files;
};

struct udiff * alloc_udiff() {
    struct udiff *new_diff = malloc(sizeof(struct udiff));
    new_diff->num_lines = 0;
    new_diff->lines = NULL;
    new_diff->file_count = 0;
    new_diff->file_capacity = 0;
    new_diff->files = NULL;
    return new_diff;
}

struct udiff_file *alloc_file() {
    struct udiff_file *new_file = malloc(sizeof(struct udiff_file));
    new_file->start_idx = -1;
    new_file->is_git = false;
    new_file->from_header_idx = -1;
    new_file->to_header_idx = -1;
    new_file->from_filename = NULL;
    new_file->to_filename = NULL;
    return new_file;
}

//
void append_file(struct udiff *diff, struct udiff_file *file) {
    if (diff->file_count >= diff->file_capacity) {
        struct udiff_file **old = diff->files;
        int old_cap = diff->file_capacity;

        int new_cap = max(2 * old_cap, 10);
        diff->files = malloc(new_cap * sizeof(struct udiff_file*));

        if (old != NULL) {
            memcpy(diff->files, old, old_cap * sizeof(struct udiff_file*));
            free(old);
        }
    }

    diff->files[diff->file_count] = file;
    diff->file_count++;
}

// Stores the character buffer contents as an array of strings in diff.
//
// We need to start by determining the overall number of lines, which should
// end up being the total number of newlines, plus one if there's any additional
// content after the last newline.
void create_udiff_lines(const char *contents, struct udiff *diff) {
    int content_len = strlen(contents);

    int nl_count = 0;       // The total number of newlines
    int last_nl_idx = -1;   // The last newline index
    for (int ii = 0; ii < content_len; ++ii) {
        if (contents[ii] == '\n') {
            nl_count++;
            last_nl_idx = ii;
        }
    }

    diff->num_lines = nl_count;
    if (last_nl_idx < (content_len - 1)) {
        diff->num_lines++;
    }

    diff->lines = malloc(diff->num_lines * sizeof(char *));

    int last_idx = -1;
    int line_idx = 0;
    for (int ii = 0; ii < content_len; ++ii) {
        if (last_idx == -1) {
            last_idx = ii;
        }
        if (contents[ii] == '\n') {
            // We're at a newline, include it in the copied line
            int line_len = ii - last_idx + 1;

            // Make room for a null byte, we're copying a fragment of the
            // string using memcpy since there's no null byte.
            diff->lines[line_idx] = malloc((line_len + 1) * sizeof(char));
            memcpy(diff->lines[line_idx], &contents[last_idx], line_len);
            diff->lines[line_idx][line_len] = '\0';

            line_idx++;
            last_idx = -1;
        }
    }
}

const char *const GIT_HEADERS[] = {
        "diff --git",
        "old mode",
        "new mode",
        "deleted file mode",
        "new file mode",
        "copy from",
        "copy to",
        "rename from",
        "rename to",
        "similarity index",
        "index"
};

const int GIT_HEADERS_LEN = sizeof(GIT_HEADERS) / sizeof(GIT_HEADERS[0]);

const char *const FROM_HEADER_PREFIX = "---";

const char *const TO_HEADER_PREFIX = "+++";

const char *const CHUNK_HEADER_PREFIX = "@@";

bool starts_with(const char *str, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    size_t str_len = strlen(str);
    return str_len >= prefix_len && strncmp(str, prefix, prefix_len) == 0;
}

bool is_from_header(const char *line) {
    return starts_with(line, FROM_HEADER_PREFIX);
}

bool is_to_header(const char *line) {
    return starts_with(line, TO_HEADER_PREFIX);
}

bool is_chunk_header(const char *line) {
    return starts_with(line, CHUNK_HEADER_PREFIX);
}

bool is_git_header(const char *line) {
    for (int ii = 0; ii < GIT_HEADERS_LEN; ++ii) {
        if (starts_with(line, GIT_HEADERS[ii])) {
            return true;
        }
    }
    return false;
}

bool is_file_header(const char *line) {
    return is_from_header(line) ||
            is_to_header(line) ||
            is_git_header(line);
}

char *alloc_filename_from_header(const char *line, bool is_git) {
    char *start = index(line, ' ') + 1;

    if (is_git) {
        start = start + 2;
    }

    char *end = rindex(line, '\t');
    if (end == NULL) {
        end = rindex(line, '\n');
    }
    if (end == NULL) {
        end = rindex(line, '\0');
    }

    size_t name_len = end - start;

    char *filename = malloc(sizeof(char) * (name_len + 1));
    memcpy(filename, start, name_len);
    filename[name_len] = '\0';

    return filename;
}

// Make a single pass through all lines, and create a files structure for each
// new header section you find.
//
// We make a note of each header section, and, each chunk section.
int initialize_udiff_files(struct udiff *diff) {
    // When we start reading chunks, we know we've hit the next file when we
    // hit a valid header line. This is our toggle.
    bool in_header = false;

    // The current line reference
    const char *line = NULL;

    // The handles to the current file
    struct udiff_file *cur_file = NULL;

    diff->file_count = 0;
    diff->files = NULL;

    for (int idx = 0; idx < diff->num_lines; ++idx) {
        line = diff->lines[idx];

        if (is_file_header(line)) {
            if (!in_header) {
                // start new file
                if (cur_file != NULL) {
                    append_file(diff, cur_file);
                }
                cur_file = alloc_file();
                in_header = true;
            }
            if (is_git_header(line)) {
                cur_file->is_git = true;
            }
            if (is_from_header(line)) {
                cur_file->from_header_idx = idx;
                cur_file->from_filename =
                        alloc_filename_from_header(line, cur_file->is_git);
            }
            if (is_to_header(line)) {
                cur_file->to_header_idx = idx;
                cur_file->to_filename =
                        alloc_filename_from_header(line, cur_file->is_git);
            }
        } else if (is_chunk_header(line)) {
            // TODO: start new chunk
            in_header = false;
        } else if (in_header) {
            return MALFORMED_HEADER;
        } else if (cur_file == NULL && !in_header) {
            // This is kind of a special case where if the first line is crap
            // we don't otherwise detect it
            return MALFORMED_HEADER;
        }
    }

    if (cur_file != NULL) {
        append_file(diff, cur_file);
    }

    return 0;
}


// Allocate a new udiff_handle reference based on a UTF-8 string buffer.
//
// If there is any issue, the err will be allocated, but the return value
// should be NULL. Free the error after dealing with it.
int udiff_parse(const char *contents, udiff_handle *handle) {
    struct udiff *diff = alloc_udiff();

    create_udiff_lines(contents, diff);

    int err;

    if ((err = initialize_udiff_files(diff)) != 0) {
        udiff_free(diff);
        return err;
    }

    *handle = (udiff_handle) diff;
    return 0;
}

void udiff_file_free(struct udiff_file *file) {
    free(file->from_filename);
    free(file->to_filename);
    free(file);
}

// Free the udiff structure
void udiff_free(udiff_handle handle) {
    struct udiff *udiff_ptr = (struct udiff *) handle;

    for (int ii = 0; ii < udiff_ptr->num_lines; ++ii) {
        free(udiff_ptr->lines[ii]);
    }
    free(udiff_ptr->lines);

    for (int jj = 0; jj < udiff_ptr->file_count; ++jj) {
        udiff_file_free(udiff_ptr->files[jj]);
    }
    free(udiff_ptr->files);

    free(udiff_ptr);
}

const char *udiff_error_description(int code) {
    int idx = code - 1001;
    if (0 <= idx && idx < ERRORS_LEN) {
        return ERRORS[idx];
    }
    return NULL;
}

int udiff_file_count(udiff_handle handle) {
    struct udiff* diff = (struct udiff*)handle;
    return diff->file_count;
}

// Get reference to the 'from filename' for the diff.
const char *udiff_from_filename(udiff_handle handle, int file_idx) {
    struct udiff *diff = (struct udiff *) handle;
    return diff->files[file_idx]->from_filename;
}

// Get reference to the 'to filename' for the diff.
const char *udiff_to_filename(udiff_handle handle, int file_idx) {
    struct udiff *diff = (struct udiff *) handle;
    return diff->files[file_idx]->to_filename;
}

int udiff_chunk_count(udiff_handle udiff, int file_idx) {
    return -1;
}

int udiff_chunk_from_start(udiff_handle udiff, int file_idx, int chunk_idx) {
    return -1;
}

int udiff_chunk_from_length(udiff_handle udiff, int file_idx, int chunk_idx) {
    return -1;
}

int udiff_chunk_to_start(udiff_handle udiff, int file_idx, int chunk_idx) {
    return -1;
}

int udiff_chunk_to_length(udiff_handle udiff, int file_idx, int chunk_idx) {
    return -1;
}

int udiff_chunk_line_count(udiff_handle udiff, int file_idx, int chunk_idx) {
    return -1;
}

const char * udiff_chunk_line(udiff_handle handle, int file_idx, int chunk_idx,
                            int lineno) {
    return NULL;
}

enum udiff_line_type udiff_chunk_line_type(udiff_handle handle, int file_idx, int chunk_idx,
        int lineno) {
    // Look at the first column of the line
    return -1;
}
