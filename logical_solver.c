#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <time.h>

#define BLACK_0 0
#define BLACK_1 1
#define BLACK_2 2
#define BLACK_3 3
#define BLACK_4 4
#define BLACK 5
#define WHITE 6
#define TRI_UP_LEFT 7
#define TRI_UP_RIGHT 8
#define TRI_DOWN_LEFT 9
#define TRI_DOWN_RIGHT 10

#define CAND_WHITE (1 << 4)
#define CAND_TRI_UP_LEFT (1 << 5)
#define CAND_TRI_UP_RIGHT (1 << 6)
#define CAND_TRI_DOWN_LEFT (1 << 7)
#define CAND_TRI_DOWN_RIGHT (1 << 8)
#define CAND_ALL (CAND_WHITE | CAND_TRI_UP_LEFT | CAND_TRI_UP_RIGHT | CAND_TRI_DOWN_LEFT | CAND_TRI_DOWN_RIGHT)

enum EdgeDirection {
    EDGE_UP = 0,
    EDGE_DOWN = 1,
    EDGE_LEFT = 2,
    EDGE_RIGHT = 3
};

static bool is_candidate_cell(int cell) {
    return (cell & CAND_ALL) != 0;
}

static int resolve_single_candidate(int cell) {
    if (!is_candidate_cell(cell)) {
        return cell;
    }

    if (cell == CAND_WHITE) {
        return WHITE;
    }
    if (cell == CAND_TRI_UP_LEFT) {
        return TRI_UP_LEFT;
    }
    if (cell == CAND_TRI_UP_RIGHT) {
        return TRI_UP_RIGHT;
    }
    if (cell == CAND_TRI_DOWN_LEFT) {
        return TRI_DOWN_LEFT;
    }
    if (cell == CAND_TRI_DOWN_RIGHT) {
        return TRI_DOWN_RIGHT;
    }

    return cell;
}

static bool has_black_edge_toward(int cell, enum EdgeDirection direction) {
    if (cell >= BLACK_0 && cell <= BLACK) {
        return true;
    }

    switch (cell) {
        case TRI_UP_LEFT:
            return direction == EDGE_UP || direction == EDGE_LEFT;
        case TRI_UP_RIGHT:
            return direction == EDGE_UP || direction == EDGE_RIGHT;
        case TRI_DOWN_LEFT:
            return direction == EDGE_DOWN || direction == EDGE_LEFT;
        case TRI_DOWN_RIGHT:
            return direction == EDGE_DOWN || direction == EDGE_RIGHT;
        default:
            return false;
    }
}

static char solution_token_for_cell(int cell) {
    switch (cell) {
        case TRI_DOWN_LEFT:
            return '2';
        case TRI_DOWN_RIGHT:
            return '3';
        case TRI_UP_RIGHT:
            return '4';
        case TRI_UP_LEFT:
            return '5';
        default:
            return '.';
    }
}

static bool build_solution_path(const char *input_path, char *output_path, size_t output_size) {
    const char *last_slash = strrchr(input_path, '/');
    const char *file_name = (last_slash != NULL) ? last_slash + 1 : input_path;
    const char *last_dot = strrchr(file_name, '.');
    size_t prefix_length = (last_dot != NULL) ? (size_t)(last_dot - input_path) : strlen(input_path);

    if (last_dot == NULL) {
        return snprintf(output_path, output_size, "%s_logic_solved.txt", input_path) < (int)output_size;
    }

    return snprintf(output_path, output_size, "%.*s_logic_solved.txt", (int)prefix_length, input_path) < (int)output_size;
}

static bool write_solution_file(const char *input_path, int **board, int height, int width) {
    char output_path[1024];
    FILE *output_fp = NULL;

    if (!build_solution_path(input_path, output_path, sizeof(output_path))) {
        return false;
    }

    output_fp = fopen(output_path, "w");
    if (output_fp == NULL) {
        perror("fopen");
        return false;
    }

    fprintf(output_fp, "pzprv3\n");
    fprintf(output_fp, "shakashaka\n");
    fprintf(output_fp, "%d\n", height);
    fprintf(output_fp, "%d\n", width);

    for (int row = 1; row <= height; row++) {
        for (int col = 1; col <= width; col++) {
            if (board[row][col] >= BLACK_0 && board[row][col] <= BLACK) {
                fprintf(output_fp, "%d", board[row][col]);
            } else {
                fprintf(output_fp, ".");
            }
            if (col < width) {
                fputc(' ', output_fp);
            }
        }
        fputc('\n', output_fp);
    }

    for (int row = 1; row <= height; row++) {
        for (int col = 1; col <= width; col++) {
            fputc(solution_token_for_cell(board[row][col]), output_fp);
            if (col < width) {
                fputc(' ', output_fp);
            }
        }
        fputc('\n', output_fp);
    }

    fclose(output_fp);
    return true;
}

static void trim_newline(char *text) {
    size_t length = strlen(text);
    if (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        text[length - 1] = '\0';
        length--;
    }
    if (length > 0 && text[length - 1] == '\r') {
        text[length - 1] = '\0';
    }
}

static void free_board(int **board, int board_height) {
    if (board == NULL) {
        return;
    }

    for (int row = 0; row < board_height; row++) {
        free(board[row]);
    }
    free(board);
}

static bool load_puzzle_board(const char *input_file, int ***board_out, int *height_out, int *width_out, int *board_height_out, int *board_width_out, int *unsolved_out) {
    char line[1024];
    int height = 0;
    int width = 0;
    int board_height = 0;
    int board_width = 0;
    int unsolved = 0;
    int **board = NULL;
    FILE *fp = fopen(input_file, "r");

    if (fp == NULL) {
        perror("fopen");
        return false;
    }

    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Error: missing first line.\n");
        fclose(fp);
        return false;
    }
    trim_newline(line);
    if (strcmp(line, "pzprv3") != 0) {
        fprintf(stderr, "Error: invalid first line: %s\n", line);
        fclose(fp);
        return false;
    }

    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Error: missing second line.\n");
        fclose(fp);
        return false;
    }
    trim_newline(line);
    if (strcmp(line, "shakashaka") != 0) {
        fprintf(stderr, "Error: invalid second line: %s\n", line);
        fclose(fp);
        return false;
    }

    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Error: missing height.\n");
        fclose(fp);
        return false;
    }
    if (sscanf(line, "%d", &height) != 1 || height <= 0) {
        fprintf(stderr, "Error: invalid height.\n");
        fclose(fp);
        return false;
    }

    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Error: missing width.\n");
        fclose(fp);
        return false;
    }
    if (sscanf(line, "%d", &width) != 1 || width <= 0) {
        fprintf(stderr, "Error: invalid width.\n");
        fclose(fp);
        return false;
    }

    board_height = height + 2;
    board_width = width + 2;

    board = (int **)malloc((size_t)board_height * sizeof(int *));
    if (board == NULL) {
        fprintf(stderr, "Error: failed to allocate board rows.\n");
        fclose(fp);
        return false;
    }

    for (int row = 0; row < board_height; row++) {
        board[row] = (int *)malloc((size_t)board_width * sizeof(int));
        if (board[row] == NULL) {
            fprintf(stderr, "Error: failed to allocate board row %d.\n", row);
            free_board(board, row);
            fclose(fp);
            return false;
        }

        for (int col = 0; col < board_width; col++) {
            board[row][col] = CAND_ALL;
        }
    }

    for (int row = 0; row < board_height; row++) {
        board[row][0] = BLACK;
        board[row][board_width - 1] = BLACK;
    }
    for (int col = 0; col < board_width; col++) {
        board[0][col] = BLACK;
        board[board_height - 1][col] = BLACK;
    }

    for (int row = 0; row < height; row++) {
        if (fgets(line, sizeof(line), fp) == NULL) {
            fprintf(stderr, "Error: missing puzzle row %d.\n", row);
            free_board(board, board_height);
            fclose(fp);
            return false;
        }
        unsolved += width;
        char *token = strtok(line, " \t\r\n");
        for (int col = 0; col < width; col++) {
            if (token == NULL) {
                fprintf(stderr, "Error: not enough cells in puzzle row %d.\n", row);
                free_board(board, board_height);
                fclose(fp);
                return false;
            }

            board[row + 1][col + 1] = CAND_ALL;
            if (token[0] >= '0' && token[0] <= '5') {
                board[row + 1][col + 1] = token[0] - '0';
                unsolved--;
            }

            token = strtok(NULL, " \t\r\n");
        }
    }

    for (int row = 0; row < height; row++) {
        if (fgets(line, sizeof(line), fp) == NULL) {
            fprintf(stderr, "Error: missing solve row %d.\n", row);
            free_board(board, board_height);
            fclose(fp);
            return false;
        }

        char *token = strtok(line, " \t\r\n");
        for (int col = 0; col < width; col++) {
            if (token == NULL) {
                fprintf(stderr, "Error: not enough cells in solve row %d.\n", row);
                free_board(board, board_height);
                fclose(fp);
                return false;
            }

            if (token[0] != '.') {
                fprintf(stderr, "Error: solve state should start with dots, got %c at row %d col %d.\n", token[0], row, col);
                free_board(board, board_height);
                fclose(fp);
                return false;
            }

            token = strtok(NULL, " \t\r\n");
        }
    }

    fclose(fp);
    *board_out = board;
    *height_out = height;
    *width_out = width;
    *board_height_out = board_height;
    *board_width_out = board_width;
    *unsolved_out = unsolved;
    return true;
}

static bool check_triangle_candidate_with_rectangle_completion(int **board, int height, int width, int row, int col, int candidate_type) {
    int target_row = row;
    int target_col = col;
    switch (candidate_type) {
        case CAND_TRI_UP_LEFT:
            if (board[row - 1][col + 1] == TRI_UP_LEFT) {
                int count = 0;
                while (board[row - 1][col + 1] == TRI_UP_LEFT) {
                    count++;
                    row--;
                    col++;
                }
                if (board[row][col + 1] == TRI_UP_RIGHT) {
                    col++;
                    while (board[row + 1][col + 1] == TRI_UP_RIGHT) {
                        row++;
                        col++;
                    }
                    if (board[row + 1][col] == TRI_DOWN_RIGHT) {
                        row++;
                        while (board[row + 1][col - 1] == TRI_DOWN_RIGHT && count > 0) {
                            count--;
                            row++;
                            col--;
                        }
                        if (count == 0) return true;
                    }
                }
            }
            row = target_row;
            col = target_col;
            if (board[row + 1][col - 1] == TRI_UP_LEFT) {
                int count = 0;
                while (board[row + 1][col - 1] == TRI_UP_LEFT) {
                    count++;
                    row++;
                    col--;
                }
                if (board[row + 1][col] == TRI_DOWN_LEFT) {
                    row++;
                    while (board[row + 1][col + 1] == TRI_DOWN_LEFT) {
                        row++;
                        col++;
                    }
                    if (board[row][col + 1] == TRI_DOWN_RIGHT) {
                        col++;
                        while (board[row - 1][col + 1] == TRI_DOWN_RIGHT && count > 0) {
                            count--;
                            row--;
                            col++;
                        }
                        if (count == 0) return true;
                    }
                }
            }
            return false;
        case CAND_TRI_UP_RIGHT:
            if (board[row - 1][col - 1] == TRI_UP_RIGHT) {
                int count = 0;
                while (board[row - 1][col - 1] == TRI_UP_RIGHT) {
                    count++;
                    row--;
                    col--;
                }
                if (board[row][col - 1] == TRI_UP_LEFT) {
                    col--;
                    while (board[row + 1][col - 1] == TRI_UP_LEFT) {
                        row++;
                        col--;
                    }
                    if (board[row + 1][col] == TRI_DOWN_LEFT) {
                        row++;
                        while (board[row + 1][col + 1] == TRI_DOWN_LEFT && count > 0) {
                            count--;
                            row++;
                            col++;
                        }
                        if (count == 0) return true;
                    }
                }
            }
            row = target_row;
            col = target_col;
            if (board[row + 1][col + 1] == TRI_UP_RIGHT) {
                int count = 0;
                while (board[row + 1][col + 1] == TRI_UP_RIGHT) {
                    count++;
                    row++;
                    col++;
                }
                if (board[row + 1][col] == TRI_DOWN_RIGHT) {
                    row++;
                    while (board[row + 1][col - 1] == TRI_DOWN_RIGHT) {
                        row++;
                        col--;
                    }
                    if (board[row][col - 1] == TRI_DOWN_LEFT) {
                        col--;
                        while (board[row - 1][col - 1] == TRI_DOWN_LEFT && count > 0) {
                            count--;
                            row--;
                            col--;
                        }
                        if (count == 0) return true;
                    }
                }
            }
            return false;
        case CAND_TRI_DOWN_LEFT:
            if (board[row - 1][col - 1] == TRI_DOWN_LEFT) {
                int count = 0;
                while (board[row - 1][col - 1] == TRI_DOWN_LEFT) {
                    count++;
                    row--;
                    col--;
                }
                if (board[row - 1][col] == TRI_UP_LEFT) {
                    row--;
                    while (board[row - 1][col + 1] == TRI_UP_LEFT) {
                        row--;
                        col++;
                    }
                    if (board[row][col + 1] == TRI_UP_RIGHT) {
                        col++;
                        while (board[row + 1][col + 1] == TRI_UP_RIGHT && count > 0) {
                            count--;
                            row++;
                            col++;
                        }
                        if (count == 0) return true;
                    }
                }
            }
            row = target_row;
            col = target_col;
            if (board[row + 1][col + 1] == TRI_DOWN_LEFT) {
                int count = 0;
                while (board[row + 1][col + 1] == TRI_DOWN_LEFT) {
                    count++;
                    row++;
                    col++;
                }
                if (board[row][col + 1] == TRI_DOWN_RIGHT) {
                    col++;
                    while (board[row - 1][col + 1] == TRI_DOWN_RIGHT) {
                        row--;
                        col++;
                    }
                    if (board[row - 1][col] == TRI_UP_RIGHT) {
                        row--;
                        while (board[row - 1][col - 1] == TRI_UP_RIGHT && count > 0) {
                            count--;
                            row--;
                            col--;
                        }
                        if (count == 0) return true;
                    }
                }
            }
            return false;
        case CAND_TRI_DOWN_RIGHT:
            if (board[row - 1][col + 1] == TRI_DOWN_RIGHT) {
                int count = 0;
                while (board[row - 1][col + 1] == TRI_DOWN_RIGHT) {
                    count++;
                    row--;
                    col++;
                }
                if (board[row - 1][col] == TRI_UP_RIGHT) {
                    row--;
                    while (board[row - 1][col - 1] == TRI_UP_RIGHT) {
                        row--;
                        col--;
                    }
                    if (board[row][col - 1] == TRI_UP_LEFT) {
                        col--;
                        while (board[row + 1][col - 1] == TRI_UP_LEFT && count > 0) {
                            count--;
                            row++;
                            col--;
                        }
                        if (count == 0) return true;
                    }
                }
            }
            row = target_row;
            col = target_col;
            if (board[row + 1][col - 1] == TRI_DOWN_RIGHT) {
                int count = 0;
                while (board[row + 1][col - 1] == TRI_DOWN_RIGHT) {
                    count++;
                    row++;
                    col--;
                }
                if (board[row][col - 1] == TRI_DOWN_LEFT) {
                    col--;
                    while (board[row - 1][col - 1] == TRI_DOWN_LEFT) {
                        row--;
                        col--;
                    }
                    if (board[row - 1][col] == TRI_UP_LEFT) {
                        row--;
                        while (board[row - 1][col + 1] == TRI_UP_LEFT && count > 0) {
                            count--;
                            row--;
                            col++;
                        }
                        if (count == 0) return true;
                    }
                }
            }
            return false;
        default:
            return false;
    }
}

int main(int argc, char *argv[]) {
    int height = 0;
    int width = 0;
    int board_height = 0;
    int board_width = 0;
    int **board = NULL;
    int unsolved = 0;
    clock_t start_time = clock();


    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    if (!load_puzzle_board(input_file,
                           &board,
                           &height,
                           &width,
                           &board_height,
                           &board_width,
                           &unsolved)) {
        return 1;
    }

    setlocale(LC_ALL, "");
    printf("Problem Board:\n");
    for (int row = 0; row < board_height; row++) {
        for (int col = 0; col < board_width; col++) {
            if (board[row][col] >= 0 && board[row][col] <= 5) {
                wprintf(L"%lc", L'█');
            } else if (board[row][col] == WHITE) {
                wprintf(L"%lc", L' ');
            } else if (board[row][col] == TRI_DOWN_LEFT) {
                wprintf(L"%lc", L'◣');
            } else if (board[row][col] == TRI_DOWN_RIGHT) {
                wprintf(L"%lc", L'◢');
            } else if (board[row][col] == TRI_UP_LEFT) {
                wprintf(L"%lc", L'◤');
            } else if (board[row][col] == TRI_UP_RIGHT) {
                wprintf(L"%lc", L'◥');
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
    printf("\n");
    printf("unsolved: %d\n", unsolved);
    printf("=====================================================\n");
    printf("\n");

    int iteration = 0;
    bool update = true;
    while(update && unsolved > 0) {
        iteration++;
        update = false;
        for (int row = 1; row <= height; row++) {
            for (int col = 1; col <= width; col++) {
                if (board[row][col] >= BLACK_0 && board[row][col] <= BLACK_4) {
                    if (!(is_candidate_cell(board[row - 1][col]) || 
                          is_candidate_cell(board[row + 1][col]) || 
                          is_candidate_cell(board[row][col - 1]) || 
                          is_candidate_cell(board[row][col + 1]))) continue;
                    int required_triangles = board[row][col];
                    int not_triangles = 0;
                    int triangles = 0;
                    if (board[row - 1][col] >= BLACK_0 && board[row - 1][col] <= WHITE) not_triangles++;
                    else if (board[row - 1][col] >= TRI_UP_LEFT && board[row - 1][col] <= TRI_DOWN_RIGHT) triangles++;
                    else if (!(board[row - 1][col] & CAND_WHITE)) triangles++;
                    
                    if (board[row + 1][col] >= BLACK_0 && board[row + 1][col] <= WHITE) not_triangles++;
                    else if (board[row + 1][col] >= TRI_UP_LEFT && board[row + 1][col] <= TRI_DOWN_RIGHT) triangles++;
                    else if (!(board[row + 1][col] & CAND_WHITE)) triangles++;
                    
                    if (board[row][col - 1] >= BLACK_0 && board[row][col - 1] <= WHITE) not_triangles++;
                    else if (board[row][col - 1] >= TRI_UP_LEFT && board[row][col - 1] <= TRI_DOWN_RIGHT) triangles++;
                    else if (!(board[row][col - 1] & CAND_WHITE)) triangles++;

                    if (board[row][col + 1] >= BLACK_0 && board[row][col + 1] <= WHITE) not_triangles++;
                    else if (board[row][col + 1] >= TRI_UP_LEFT && board[row][col + 1] <= TRI_DOWN_RIGHT) triangles++;
                    else if (!(board[row][col + 1] & CAND_WHITE)) triangles++;

                    if (triangles > required_triangles || not_triangles > 4 - required_triangles) {
                        fprintf(stderr, "Error: puzzle is unsolvable due to contradiction at row %d col %d.\n", row, col);
                        free_board(board, board_height);
                        return 1;
                    }
                    if (triangles == required_triangles) {
                        if (board[row - 1][col] & CAND_WHITE) {
                            board[row - 1][col] = WHITE;
                            unsolved--;
                            update = true;
                        }
                        if (board[row + 1][col] & CAND_WHITE) {
                            board[row + 1][col] = WHITE;
                            unsolved--;
                            update = true;
                        }
                        if (board[row][col - 1] & CAND_WHITE) {
                            board[row][col - 1] = WHITE;
                            unsolved--;
                            update = true;
                        }
                        if (board[row][col + 1] & CAND_WHITE) {
                            board[row][col + 1] = WHITE;
                            unsolved--;
                            update = true;
                        }
                    } else if (not_triangles == 4 - required_triangles) {
                        if (board[row - 1][col] & CAND_WHITE) {
                            board[row - 1][col] &= ~CAND_WHITE;
                            update = true;
                        }
                        if (board[row + 1][col] & CAND_WHITE) {
                            board[row + 1][col] &= ~CAND_WHITE;
                            update = true;
                        }
                        if (board[row][col - 1] & CAND_WHITE) {
                            board[row][col - 1] &= ~CAND_WHITE;
                            update = true;
                        }
                        if (board[row][col + 1] & CAND_WHITE) {
                            board[row][col + 1] &= ~CAND_WHITE;
                            update = true;
                        }
                    }
                }
                else if (is_candidate_cell(board[row][col])) {
                    int previous_state = board[row][col];
                    if (is_candidate_cell(board[row][col + 1])) {
                        if ((board[row - 1][col] == TRI_UP_RIGHT && board[row - 1][col + 1] == TRI_UP_LEFT) ||
                            (board[row + 1][col] == TRI_DOWN_RIGHT && board[row + 1][col + 1] == TRI_DOWN_LEFT)) {
                                board[row][col] &= ~CAND_WHITE;
                                board[row][col + 1] &= ~CAND_WHITE;
                        }
                    }
                    if (is_candidate_cell(board[row + 1][col])) {
                        if ((board[row][col - 1] == TRI_DOWN_LEFT && board[row + 1][col - 1] == TRI_UP_LEFT) ||
                            (board[row][col + 1] == TRI_DOWN_RIGHT && board[row + 1][col + 1] == TRI_UP_RIGHT)) {
                                board[row][col] &= ~CAND_WHITE;
                                board[row + 1][col] &= ~CAND_WHITE;
                        }
                    }
                    if (board[row][col] & CAND_TRI_UP_LEFT) {
                        if ((board[row + 1][col] == TRI_DOWN_LEFT && (has_black_edge_toward(board[row][col - 1], EDGE_RIGHT) || board[row][col - 1] == WHITE)) ||
                            (board[row][col + 1] == TRI_UP_RIGHT && (has_black_edge_toward(board[row - 1][col], EDGE_DOWN) || board[row - 1][col] == WHITE)) ||
                            (board[row - 1][col + 1] == TRI_UP_LEFT && board[row][col + 1] == WHITE) ||
                            (board[row + 1][col - 1] == TRI_UP_LEFT && board[row + 1][col] == WHITE) ||
                            check_triangle_candidate_with_rectangle_completion(board, height, width, row, col, CAND_TRI_UP_LEFT)) {
                            board[row][col] = TRI_UP_LEFT;
                        }
                        else if (has_black_edge_toward(board[row + 1][col], EDGE_UP) ||
                                 has_black_edge_toward(board[row][col + 1], EDGE_LEFT) ||
                                 board[row + 1][col] == TRI_DOWN_RIGHT ||
                                 board[row][col + 1] == TRI_DOWN_RIGHT ||
                                 board[row - 1][col] == TRI_UP_LEFT ||
                                 board[row - 1][col] == TRI_UP_RIGHT ||
                                 board[row][col - 1] == TRI_UP_LEFT ||
                                 board[row][col - 1] == TRI_DOWN_LEFT ||
                                 board[row - 1][col - 1] == TRI_UP_LEFT ||
                                 board[row - 1][col + 1] == TRI_UP_RIGHT ||
                                 board[row + 1][col - 1] == TRI_DOWN_LEFT ||
                                 (board[row + 1][col] == WHITE && !(board[row + 1][col - 1] == TRI_UP_LEFT) && !(board[row + 1][col - 1] & CAND_TRI_UP_LEFT)) ||
                                 (board[row][col + 1] == WHITE && !(board[row - 1][col + 1] == TRI_UP_LEFT) && !(board[row - 1][col + 1] & CAND_TRI_UP_LEFT)) ||
                                 (!is_candidate_cell(board[row + 1][col + 1]) && !(board[row + 1][col + 1] == WHITE || board[row + 1][col + 1] == TRI_DOWN_RIGHT)) ||
                                 (board[row - 1][col] == WHITE && board[row][col - 1] == WHITE && board[row - 1][col - 1] == WHITE)
                                ) {
                            board[row][col] &= ~CAND_TRI_UP_LEFT;
                        }
                    }
                    if (board[row][col] & CAND_TRI_UP_RIGHT) {
                        if ((board[row + 1][col] == TRI_DOWN_RIGHT && (has_black_edge_toward(board[row][col + 1], EDGE_LEFT) || board[row][col + 1] == WHITE)) ||
                            (board[row][col - 1] == TRI_UP_LEFT && (has_black_edge_toward(board[row - 1][col], EDGE_DOWN) || board[row - 1][col] == WHITE)) ||
                            (board[row - 1][col - 1] == TRI_UP_RIGHT && board[row][col - 1] == WHITE) ||
                            (board[row + 1][col + 1] == TRI_UP_RIGHT && board[row + 1][col] == WHITE) ||
                            check_triangle_candidate_with_rectangle_completion(board, height, width, row, col, CAND_TRI_UP_RIGHT)) {
                            board[row][col] = TRI_UP_RIGHT;
                        }
                        else if (has_black_edge_toward(board[row + 1][col], EDGE_UP) ||
                                 has_black_edge_toward(board[row][col - 1], EDGE_RIGHT) ||
                                 board[row + 1][col] == TRI_DOWN_LEFT ||
                                 board[row][col - 1] == TRI_DOWN_LEFT ||
                                 board[row - 1][col] == TRI_UP_LEFT ||
                                 board[row - 1][col] == TRI_UP_RIGHT ||
                                 board[row][col + 1] == TRI_UP_RIGHT ||
                                 board[row][col + 1] == TRI_DOWN_RIGHT ||
                                 board[row - 1][col - 1] == TRI_UP_LEFT ||
                                 board[row - 1][col + 1] == TRI_UP_RIGHT ||
                                 board[row + 1][col + 1] == TRI_DOWN_RIGHT ||
                                 (board[row + 1][col] == WHITE && !(board[row + 1][col + 1] == TRI_UP_RIGHT) && !(board[row + 1][col + 1] & CAND_TRI_UP_RIGHT)) ||
                                 (board[row][col - 1] == WHITE && !(board[row - 1][col - 1] == TRI_UP_RIGHT) && !(board[row - 1][col - 1] & CAND_TRI_UP_RIGHT)) ||
                                 (!is_candidate_cell(board[row + 1][col - 1]) && !(board[row + 1][col - 1] == WHITE || board[row + 1][col - 1] == TRI_DOWN_LEFT)) ||
                                 (board[row - 1][col] == WHITE && board[row][col + 1] == WHITE && board[row - 1][col + 1] == WHITE)
                                ) {
                            board[row][col] &= ~CAND_TRI_UP_RIGHT;
                        }
                    }
                    if (board[row][col] & CAND_TRI_DOWN_LEFT) {
                        if ((board[row - 1][col] == TRI_UP_LEFT && (has_black_edge_toward(board[row][col - 1], EDGE_RIGHT) || board[row][col - 1] == WHITE)) ||
                            (board[row][col + 1] == TRI_DOWN_RIGHT && (has_black_edge_toward(board[row + 1][col], EDGE_UP) || board[row + 1][col] == WHITE)) ||
                            (board[row - 1][col - 1] == TRI_DOWN_LEFT && board[row - 1][col] == WHITE) ||
                            (board[row + 1][col + 1] == TRI_DOWN_LEFT && board[row][col + 1] == WHITE) ||
                            check_triangle_candidate_with_rectangle_completion(board, height, width, row, col, CAND_TRI_DOWN_LEFT)) {
                            board[row][col] = TRI_DOWN_LEFT;
                        }
                        else if (has_black_edge_toward(board[row - 1][col], EDGE_DOWN) ||
                                 has_black_edge_toward(board[row][col + 1], EDGE_LEFT) ||
                                 board[row - 1][col] == TRI_UP_RIGHT ||
                                 board[row][col + 1] == TRI_UP_RIGHT ||
                                 board[row + 1][col] == TRI_DOWN_LEFT ||
                                 board[row + 1][col] == TRI_DOWN_RIGHT ||
                                 board[row][col - 1] == TRI_UP_LEFT ||
                                 board[row][col - 1] == TRI_DOWN_LEFT ||
                                 board[row - 1][col - 1] == TRI_UP_LEFT ||
                                 board[row + 1][col - 1] == TRI_DOWN_LEFT ||
                                 board[row + 1][col + 1] == TRI_DOWN_RIGHT ||
                                 (board[row - 1][col] == WHITE && !(board[row - 1][col - 1] == TRI_DOWN_LEFT) && !(board[row - 1][col - 1] & CAND_TRI_DOWN_LEFT)) ||
                                 (board[row][col + 1] == WHITE && !(board[row + 1][col + 1] == TRI_DOWN_LEFT) && !(board[row + 1][col + 1] & CAND_TRI_DOWN_LEFT)) ||
                                 (!is_candidate_cell(board[row - 1][col + 1]) && !(board[row - 1][col + 1] == WHITE || board[row - 1][col + 1] == TRI_UP_RIGHT)) ||
                                 (board[row + 1][col] == WHITE && board[row][col - 1] == WHITE && board[row + 1][col - 1] == WHITE)
                                ) {
                            board[row][col] &= ~CAND_TRI_DOWN_LEFT;
                        }
                    }
                    if (board[row][col] & CAND_TRI_DOWN_RIGHT) {
                        if ((board[row - 1][col] == TRI_UP_RIGHT && (has_black_edge_toward(board[row][col + 1], EDGE_LEFT) || board[row][col + 1] == WHITE)) ||
                            (board[row][col - 1] == TRI_DOWN_LEFT && (has_black_edge_toward(board[row + 1][col], EDGE_UP) || board[row + 1][col] == WHITE)) ||
                            (board[row - 1][col + 1] == TRI_DOWN_RIGHT && board[row - 1][col] == WHITE) ||
                            (board[row + 1][col - 1] == TRI_DOWN_RIGHT && board[row][col - 1] == WHITE) ||
                            check_triangle_candidate_with_rectangle_completion(board, height, width, row, col, CAND_TRI_DOWN_RIGHT)) {
                            board[row][col] = TRI_DOWN_RIGHT;
                        }
                        else if (has_black_edge_toward(board[row - 1][col], EDGE_DOWN) ||
                                 has_black_edge_toward(board[row][col - 1], EDGE_RIGHT) ||
                                 board[row - 1][col] == TRI_UP_LEFT ||
                                 board[row][col - 1] == TRI_UP_LEFT ||
                                 board[row + 1][col] == TRI_DOWN_LEFT ||
                                 board[row + 1][col] == TRI_DOWN_RIGHT ||
                                 board[row][col + 1] == TRI_UP_RIGHT ||
                                 board[row][col + 1] == TRI_DOWN_RIGHT ||
                                 board[row - 1][col + 1] == TRI_UP_RIGHT ||
                                 board[row + 1][col - 1] == TRI_DOWN_LEFT ||
                                 board[row + 1][col + 1] == TRI_DOWN_RIGHT ||
                                 (board[row - 1][col] == WHITE && !(board[row - 1][col + 1] == TRI_DOWN_RIGHT) && !(board[row - 1][col + 1] & CAND_TRI_DOWN_RIGHT)) ||
                                 (board[row][col - 1] == WHITE && !(board[row + 1][col - 1] == TRI_DOWN_RIGHT) && !(board[row + 1][col - 1] & CAND_TRI_DOWN_RIGHT)) ||
                                 (!is_candidate_cell(board[row - 1][col - 1]) && !(board[row - 1][col - 1] == WHITE || board[row - 1][col - 1] == TRI_UP_LEFT)) ||
                                 (board[row + 1][col] == WHITE && board[row][col + 1] == WHITE && board[row + 1][col + 1] == WHITE)
                                ) {
                            board[row][col] &= ~CAND_TRI_DOWN_RIGHT;
                        }
                    }
                    if (board[row][col] & CAND_WHITE) {
                        if ((board[row - 1][col] == WHITE && board[row][col - 1] == WHITE && (has_black_edge_toward(board[row - 1][col - 1], EDGE_DOWN) || has_black_edge_toward(board[row - 1][col - 1], EDGE_RIGHT))) ||
                            (board[row - 1][col] == WHITE && board[row][col + 1] == WHITE && (has_black_edge_toward(board[row - 1][col + 1], EDGE_DOWN) || has_black_edge_toward(board[row - 1][col + 1], EDGE_LEFT))) ||
                            (board[row + 1][col] == WHITE && board[row][col - 1] == WHITE && (has_black_edge_toward(board[row + 1][col - 1], EDGE_UP) || has_black_edge_toward(board[row + 1][col - 1], EDGE_RIGHT))) ||
                            (board[row + 1][col] == WHITE && board[row][col + 1] == WHITE && (has_black_edge_toward(board[row + 1][col + 1], EDGE_UP) || has_black_edge_toward(board[row + 1][col + 1], EDGE_LEFT))) ||
                            (board[row - 1][col - 1] == WHITE && ((board[row - 1][col] == WHITE && has_black_edge_toward(board[row][col - 1], EDGE_UP)) || (board[row][col - 1] == WHITE && has_black_edge_toward(board[row - 1][col], EDGE_LEFT)))) ||
                            (board[row - 1][col + 1] == WHITE && ((board[row - 1][col] == WHITE && has_black_edge_toward(board[row][col + 1], EDGE_UP)) || (board[row][col + 1] == WHITE && has_black_edge_toward(board[row - 1][col], EDGE_RIGHT)))) ||
                            (board[row + 1][col - 1] == WHITE && ((board[row + 1][col] == WHITE && has_black_edge_toward(board[row][col - 1], EDGE_DOWN)) || (board[row][col - 1] == WHITE && has_black_edge_toward(board[row + 1][col], EDGE_LEFT)))) ||
                            (board[row + 1][col + 1] == WHITE && ((board[row + 1][col] == WHITE && has_black_edge_toward(board[row][col + 1], EDGE_DOWN)) || (board[row][col + 1] == WHITE && has_black_edge_toward(board[row + 1][col], EDGE_RIGHT)))) ||
                            (has_black_edge_toward(board[row - 1][col], EDGE_DOWN) && (board[row + 1][col] == TRI_DOWN_LEFT || board[row + 1][col] == TRI_DOWN_RIGHT)) ||
                            (has_black_edge_toward(board[row + 1][col], EDGE_UP) && (board[row - 1][col] == TRI_UP_LEFT || board[row - 1][col] == TRI_UP_RIGHT)) ||
                            (has_black_edge_toward(board[row][col - 1], EDGE_RIGHT) && (board[row][col + 1] == TRI_UP_RIGHT || board[row][col + 1] == TRI_DOWN_RIGHT)) ||
                            (has_black_edge_toward(board[row][col + 1], EDGE_LEFT) && (board[row][col - 1] == TRI_UP_LEFT || board[row][col - 1] == TRI_DOWN_LEFT))
                           ) {
                            board[row][col] &= ~CAND_WHITE;
                        }
                    }
                    board[row][col] = resolve_single_candidate(board[row][col]);
                    if (board[row][col] >= 6 && board[row][col] <= 10) unsolved--;
                    if (board[row][col] != previous_state) update = true;
                }
            }
        }
    }

    if (!write_solution_file(input_file, board, height, width)) {
        free_board(board, board_height);
        return 1;
    }

    setlocale(LC_ALL, "");
    printf("Logical Solved Board:\n");
    for (int row = 0; row < board_height; row++) {
        for (int col = 0; col < board_width; col++) {
            if (board[row][col] >= 0 && board[row][col] <= 5) {
                wprintf(L"%lc", L'█');
            } else if (board[row][col] == WHITE) {
                wprintf(L"%lc", L' ');
            } else if (board[row][col] == TRI_DOWN_LEFT) {
                wprintf(L"%lc", L'◣');
            } else if (board[row][col] == TRI_DOWN_RIGHT) {
                wprintf(L"%lc", L'◢');
            } else if (board[row][col] == TRI_UP_LEFT) {
                wprintf(L"%lc", L'◤');
            } else if (board[row][col] == TRI_UP_RIGHT) {
                wprintf(L"%lc", L'◥');
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
    printf("\n");
    printf("unsolved: %d\n", unsolved);
    printf("iterations: %d\n", iteration);

    printf("Debug:\n");
    printf("row 4 col 7 is %d\n", board[4][7]);
    printf("row 4 col 8 is %d\n", board[4][8]);
    clock_t end_time = clock();
    if (start_time != (clock_t)-1 && end_time != (clock_t)-1) {
        double elapsed_seconds = (double)(end_time - start_time) / (double)CLOCKS_PER_SEC;
        printf("elapsed time: %.3f sec\n", elapsed_seconds);
    } else {
        printf("elapsed time: unavailable\n");
    }

    free_board(board, board_height);
    return 0;
}