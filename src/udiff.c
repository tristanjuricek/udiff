// Utility method
int udiff_max(int a, int b) {
struct udiff_chunk {
    int start_idx;  // Header line
    int end_idx;    // Last line of chunk content

    int from_start;
    int from_length;

    int to_start;
    int to_length;
};


    int chunk_count;
    int chunk_capacity;
    struct udiff_chunk **chunks;
struct udiff_file * alloc_udiff_file() {
struct udiff_chunk * alloc_udiff_chunk() {
    struct udiff_chunk *new_chunk = malloc(sizeof(struct udiff_chunk));
    new_chunk->start_idx = -1;
    new_chunk->end_idx = -1;
    new_chunk->from_start = -1;
    new_chunk->from_length = -1;
    new_chunk->to_start = -1;
    new_chunk->to_length = -1;
    return new_chunk;
}

void append_udiff_file(struct udiff *diff, struct udiff_file *file) {
        int new_cap = udiff_max(2 * old_cap, 10);
        diff->files = malloc(new_cap * sizeof(struct udiff_file *));
        diff->file_capacity = new_cap;
            memcpy(diff->files, old, old_cap * sizeof(struct udiff_file *));
void append_udiff_chunk(struct udiff_file *file, struct udiff_chunk *chunk) {
    if (file->chunk_count >= file->chunk_capacity){
        struct udiff_chunk **old = file->chunks;
        int old_cap = file->chunk_capacity;

        int new_cap = udiff_max(2 * old_cap, 10);
        file->chunks = malloc(new_cap * sizeof(struct udiff_chunk *));
        file->chunk_capacity = new_cap;

        if (old != NULL) {
            memcpy(file->chunks, old, old_cap * sizeof(struct udiff_chunk *));
            free(old);
        }
    }

    file->chunks[file->chunk_count] = chunk;
    file->chunk_count++;
}

const char *const UDIFF_GIT_HEADERS[] = {
const int UDIFF_GIT_HEADERS_LEN = sizeof(UDIFF_GIT_HEADERS) / sizeof(UDIFF_GIT_HEADERS[0]);
const char *const UDIFF_FROM_HEADER_PREFIX = "---";
const char *const UDIFF_TO_HEADER_PREFIX = "+++";
const char *const UDIFF_CHUNK_HEADER_PREFIX = "@@";
bool udiff_line_starts_with(const char *str, const char *prefix) {
bool is_udiff_from_header(const char *line) {
    return udiff_line_starts_with(line, UDIFF_FROM_HEADER_PREFIX);
bool is_udiff_to_header(const char *line) {
    return udiff_line_starts_with(line, UDIFF_TO_HEADER_PREFIX);
bool is_udiff_chunk_header(const char *line) {
    return udiff_line_starts_with(line, UDIFF_CHUNK_HEADER_PREFIX);
bool is_udiff_git_header(const char *line) {
    for (int ii = 0; ii < UDIFF_GIT_HEADERS_LEN; ++ii) {
        if (udiff_line_starts_with(line, UDIFF_GIT_HEADERS[ii])) {
bool is_udiff_file_header(const char *line) {
    return is_udiff_from_header(line) ||
            is_udiff_to_header(line) ||
            is_udiff_git_header(line);
char *alloc_udiff_filename_from_header(const char *line, bool is_git) {
void set_udiff_chunk_range(struct udiff_chunk *chunk, const char *line, char start_ch, bool is_from) {
    char *start = index(line, start_ch);
    char *end = index(start, ' ');

    char *comma = NULL;
    for (char* ch = start; ch < end; ++ch) {
        if (*ch == ',') {
            comma = ch;
        }
    }

    if (comma != NULL) {
        if (is_from) {
            chunk->from_start = atoi(start + 1);
            chunk->from_length = atoi(comma + 1);
        } else {
            chunk->to_start = atoi(start + 1);
            chunk->to_length = atoi(comma + 1);
        }
    } else {
        if (is_from) {
            chunk->from_start = atoi(start + 1);
            chunk->from_length = 1;
        } else {
            chunk->to_start = atoi(start + 1);
            chunk->to_length = 1;
        }
    }
}

void set_udiff_chunk_ranges(struct udiff_chunk *chunk, const char *line) {
    set_udiff_chunk_range(chunk, line, '-', true);
    set_udiff_chunk_range(chunk, line, '+', false);
}

    struct udiff_chunk *cur_chunk = NULL;

    int idx;
    for (idx = 0; idx < diff->num_lines; ++idx) {
        if (is_udiff_file_header(line)) {
                if (cur_chunk != NULL) {
                    cur_chunk->end_idx = idx - 1;
                    append_udiff_chunk(cur_file, cur_chunk);
                }
                    append_udiff_file(diff, cur_file);
                cur_file = alloc_udiff_file();
            if (is_udiff_git_header(line)) {
            if (is_udiff_from_header(line)) {
                        alloc_udiff_filename_from_header(line, cur_file->is_git);
            if (is_udiff_to_header(line)) {
                        alloc_udiff_filename_from_header(line, cur_file->is_git);
            }
        } else if (is_udiff_chunk_header(line)) {
            // Start new chunk
            if (cur_chunk != NULL) {
                cur_chunk->end_idx = idx - 1;
                append_udiff_chunk(cur_file, cur_chunk);
            cur_chunk = alloc_udiff_chunk();
            cur_chunk->start_idx = idx;
            set_udiff_chunk_ranges(cur_chunk, line);
    if (cur_chunk != NULL) {
        cur_chunk->end_idx = idx - 1;
        append_udiff_chunk(cur_file, cur_chunk);
    }
        append_udiff_file(diff, cur_file);
    for (int ii = 0; ii < file->chunk_count; ++ii) {
        free(file->chunks[ii]);
    }
    free(file->chunks);

    struct udiff *diff = (struct udiff *) handle;
    struct udiff *diff = (struct udiff *)udiff;
    return diff->files[file_idx]->chunk_count;
    struct udiff *diff = (struct udiff *)udiff;
    return diff->files[file_idx]->chunks[chunk_idx]->from_start;
    struct udiff *diff = (struct udiff *)udiff;
    return diff->files[file_idx]->chunks[chunk_idx]->from_length;
    struct udiff *diff = (struct udiff *)udiff;
    return diff->files[file_idx]->chunks[chunk_idx]->to_start;
    struct udiff *diff = (struct udiff *)udiff;
    return diff->files[file_idx]->chunks[chunk_idx]->to_length;
    struct udiff *diff = (struct udiff *)udiff;
    struct udiff_chunk *chunk = diff->files[file_idx]->chunks[chunk_idx];
    return chunk->end_idx - chunk->start_idx;
const char *udiff_chunk_line(udiff_handle handle, int file_idx, int chunk_idx,
        int line_idx) {
    struct udiff *diff = (struct udiff *)handle;
    struct udiff_chunk *chunk = diff->files[file_idx]->chunks[chunk_idx];
    int idx = chunk->start_idx + 1 + line_idx;
    return diff->lines[idx] + 1;
        int line_idx) {
    struct udiff *diff = (struct udiff *)handle;
    struct udiff_chunk *chunk = diff->files[file_idx]->chunks[chunk_idx];
    int idx = chunk->start_idx + 1 + line_idx;
    const char *line = diff->lines[idx];

    switch(line[0]) {
        case ' ':
            return udiff_same;

        case '-':
            return udiff_delete;

        case '+':
            return udiff_add;
    }
