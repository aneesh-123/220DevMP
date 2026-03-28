#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board_loader.h"
#include "cJSON.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

static int validate_gravity(int board[ROWS][COLS]) {
    int col, row;
    for (col = 0; col < COLS; col++) {
        int found_piece = 0;
        for (row = 0; row < ROWS; row++) {
            if (board[row][col] != CELL_EMPTY)
                found_piece = 1;
            else if (found_piece)
                return 0; /* floating piece */
        }
    }
    return 1;
}

static int load_one_state(const char *filepath, GameState *state, char *name_out) {
    FILE *fp;
    long fsize;
    char *json_str;
    cJSON *root, *board_arr, *name_val, *player_val, *removals_val;
    int r, c;

    fp = fopen(filepath, "rb");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    json_str = (char *)malloc(fsize + 1);
    if (!json_str) { fclose(fp); return 0; }
    fread(json_str, 1, fsize, fp);
    json_str[fsize] = '\0';
    fclose(fp);

    root = cJSON_Parse(json_str);
    free(json_str);
    if (!root) return 0;

    /* Parse board */
    board_arr = cJSON_GetObjectItem(root, "board");
    if (!board_arr || !cJSON_IsArray(board_arr) || cJSON_GetArraySize(board_arr) != ROWS) {
        cJSON_Delete(root);
        return 0;
    }

    gamestate_init(state);
    for (r = 0; r < ROWS; r++) {
        cJSON *row_arr = cJSON_GetArrayItem(board_arr, r);
        if (!row_arr || !cJSON_IsArray(row_arr) || cJSON_GetArraySize(row_arr) != COLS) {
            cJSON_Delete(root);
            return 0;
        }
        for (c = 0; c < COLS; c++) {
            cJSON *cell = cJSON_GetArrayItem(row_arr, c);
            if (cJSON_IsNull(cell))
                state->board[r][c] = CELL_EMPTY;
            else if (cJSON_IsNumber(cell))
                state->board[r][c] = cell->valueint;
            else
                state->board[r][c] = CELL_EMPTY;
        }
    }

    /* Validate gravity */
    if (!validate_gravity(state->board)) {
        cJSON_Delete(root);
        return 0;
    }

    /* Parse current_player */
    player_val = cJSON_GetObjectItem(root, "current_player");
    if (player_val && cJSON_IsNumber(player_val))
        state->current_player = player_val->valueint;

    /* Parse removals_remaining */
    removals_val = cJSON_GetObjectItem(root, "removals_remaining");
    if (removals_val && cJSON_IsArray(removals_val) && cJSON_GetArraySize(removals_val) == 2) {
        state->removals_remaining[0] = cJSON_GetArrayItem(removals_val, 0)->valueint;
        state->removals_remaining[1] = cJSON_GetArrayItem(removals_val, 1)->valueint;
    }

    /* Parse name */
    name_val = cJSON_GetObjectItem(root, "name");
    if (name_val && cJSON_IsString(name_val))
        strncpy(name_out, name_val->valuestring, MAX_STATE_NAME - 1);
    else
        strncpy(name_out, "Unnamed", MAX_STATE_NAME - 1);
    name_out[MAX_STATE_NAME - 1] = '\0';

    cJSON_Delete(root);
    return 1;
}

#ifdef _WIN32

int load_board_states(const char *dir,
                      GameState *states_out,
                      char names_out[][MAX_STATE_NAME],
                      int max_states) {
    WIN32_FIND_DATAA fd;
    HANDLE hFind;
    char pattern[512];
    int count = 0;

    snprintf(pattern, sizeof(pattern), "%s\\*.json", dir);
    hFind = FindFirstFileA(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        /* Try forward slashes */
        snprintf(pattern, sizeof(pattern), "%s/*.json", dir);
        hFind = FindFirstFileA(pattern, &fd);
        if (hFind == INVALID_HANDLE_VALUE)
            return 0;
    }

    do {
        if (count >= max_states) break;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir, fd.cFileName);

        if (load_one_state(filepath, &states_out[count], names_out[count]))
            count++;
        else
            printf("Warning: Could not load board state from %s\n", fd.cFileName);
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    return count;
}

#else

int load_board_states(const char *dir,
                      GameState *states_out,
                      char names_out[][MAX_STATE_NAME],
                      int max_states) {
    DIR *d;
    struct dirent *entry;
    int count = 0;

    d = opendir(dir);
    if (!d) return 0;

    while ((entry = readdir(d)) != NULL && count < max_states) {
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len < 6 || strcmp(name + len - 5, ".json") != 0)
            continue;

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir, name);

        if (load_one_state(filepath, &states_out[count], names_out[count]))
            count++;
        else
            printf("Warning: Could not load board state from %s\n", name);
    }

    closedir(d);
    return count;
}

#endif
