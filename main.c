/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: voparkan <voparkan@student.42prague.cz>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 13:20:05 by voparkan          #+#    #+#             */
/*   Updated: 2026/03/30 21:35:20 by voparkan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

typedef struct map {
	int lines;
	int cols;
	char empty;
	char obstacle;
	char full;
	char ** map_lines;
} bsq_map;

typedef struct main_struct {
	int num_maps;
	char*** solutions;
	struct map * maps;
} bsq_main;

typedef struct square {
	int size;
	int i;
	int j;
} bsq_square;

bool validate_line(bsq_map *map, char * line) {
	int i = 0;
	while (line) {
		if ((int)*line == 0)
			return true;
		if (!((*line == map->obstacle) || (*line == map->full) || (*line == map->empty) || ((int)*line == 10))) {
			return false;
		}
		i++;
		line++;
	}
	return true;
}

bool validate_map(bsq_map *pMap) {
	bool first = true;
	char **tmp = pMap->map_lines;
	while (*tmp) {
		if (first) {
			pMap->cols = strlen(*tmp) - 1;
			first = false;
		}
		if (strlen(*tmp) != pMap->cols + 1) {
			perror("wrong map format");
			return false;
		}
		if (!validate_line(pMap, *tmp)) {
			perror("wrong map character");
			return false;
		}
		tmp++;
	}
	return true;
}

void process_header(struct main_struct * bsq_main, char *line, FILE * fd) {
	fgets(line, BUFSIZ, fd);
	char *first_line = line;
	first_line++;
	while (!isdigit(*first_line) && isascii(*first_line)) {
		first_line++;
	}
	bsq_main->maps = malloc(sizeof (bsq_map));
	int lines_count = atoi((const char *)&line[0]);
	bsq_main->maps->lines = lines_count;
	bsq_main->maps->map_lines = malloc((lines_count + 1) * sizeof (char *));
	if(!bsq_main->maps->map_lines) {
		perror("allocation failed");
		exit(1);
	}
	bsq_main->maps->empty = line[2];
	bsq_main->maps->obstacle = line[4];
	bsq_main->maps->full = line[6];
}

bool parser(struct main_struct * bsq_main, FILE * fd) {
	char * line = malloc(BUFSIZ*sizeof (char));
	process_header(bsq_main, line, fd);
	char **lines = malloc((bsq_main->maps->lines + 1) * sizeof(char *));
	if(!lines) {
		perror("allocation failed");
		exit(1);
	}
	for (int i = 0; i < bsq_main->maps->lines; i++) {
		lines[i] = malloc(BUFSIZ * sizeof(char));
		if(!lines[i]) {
			if (i > 0) {
				while (i > 0) {
					free(lines[i - 1]);
					i--;
				}
			}
			perror("allocation failed");
			exit(1);
		}
	}
	lines[bsq_main->maps->lines] = NULL;
	char ** tmp = NULL;
	tmp = bsq_main->maps->map_lines;
	int linelen = 0;
	bool linelencnt = true;
	while (*lines && fgets(*lines, BUFSIZ, fd)) {
		if (strlen(line) == linelen || linelencnt) {
			if (linelencnt) {
				linelen = strlen(*line);
				linelencnt = false;
			}
			*bsq_main->maps->map_lines = *lines;
			bsq_main->maps->map_lines++;
			lines++;
		}
	}
	*bsq_main->maps->map_lines = NULL;
	bsq_main->maps->map_lines = tmp;
	return  (validate_map(bsq_main->maps));
}

bool parse_stdin(struct main_struct * bsq_main, FILE * fd, char * line) {
	char **lines = malloc((bsq_main->maps->lines + 1) * sizeof(char *));
	if(!lines) {
		perror("allocation failed");
		exit(1);
	}
	for (int i = 0; i < bsq_main->maps->lines; i++) {
		lines[i] = malloc(BUFSIZ * sizeof(char));
		if(!lines[i]) {
			if (i > 0) {
				while (i > 0) {
					free(lines[i - 1]);
					i--;
				}
			}
			perror("allocation failed");
			exit(1);
		}
	}
	lines[bsq_main->maps->lines] = NULL;
	char ** tmp = NULL;
	tmp = bsq_main->maps->map_lines;
	int linelen = 0;
	bool linelencnt = true;
	while (*lines && fgets(*lines, BUFSIZ, fd)) {
		if (strlen(line) == linelen || linelencnt) {
			if (linelencnt) {
				linelen = strlen(*lines);
				linelencnt = false;
			}
			*bsq_main->maps->map_lines = *lines;
			bsq_main->maps->map_lines++;
			lines++;
		}
	}
	*bsq_main->maps->map_lines = NULL;
	bsq_main->maps->map_lines = tmp;
	return  (validate_map(bsq_main->maps));
}

int find_min(int n1, int n2, int n3) {
	int min = n1;
	if (n2 < min)
		min = n2;
	if (n3 < min)
		min = n3;
	return min;
}

void process_map(bsq_main * m_bsq, bsq_square * square) {
	printf("map size: w: %d, h: %d\n", m_bsq->maps->cols, m_bsq->maps->lines);
	int height = m_bsq->maps->lines;
	int width = m_bsq->maps->cols;
	char **  map = m_bsq->maps->map_lines;
	int matrix[height][width];
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; ++j) {
			matrix[i][j] = 0;
		}
	}
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (map[i][j] == m_bsq->maps->obstacle) {
				matrix[i][j] = 0;
			}
			else if (i==0 || j==0)
				matrix[i][j] = 1;
			else {
				int min = find_min(matrix[i - 1][j], matrix[i - 1][j - 1], matrix[i][j-1]);
				matrix[i][j] = min + 1;
			}
			if (matrix[i][j] > square->size) {
				square->size = matrix[i][j];
				square->i = i - matrix[i][j] + 1;
				square->j = j - matrix[i][j] + 1;
			}
		}
	}
	for (int i = square->i; i < square->i + square->size; ++i) {
		for (int j = square->j; j < square->j + square->size; ++j) {
			if ((i < m_bsq->maps->lines) && (j < m_bsq->maps->cols))
				map[i][j] = m_bsq->maps->full;
		}
	}
//	for (int i = 0; i < m_bsq->maps->lines; ++i) {
//		for (int j = 0; j < m_bsq->maps->cols; ++j) {
//			printf("%d", matrix[i][j]);
//		}
//		printf("\n");
//	}
//	printf("squar size: %d, i: %d, j: %d\n", square->size, square->i, square->j);
	for (int i = 0; i < m_bsq->maps->lines; ++i) {
		printf("%s", map[i]);

	}
}

int main(int argc, char **argv) {
	printf("Hello, World!\n");
	bsq_main * m_bsq = malloc(sizeof (bsq_main));
	if (!m_bsq)
		perror("allocation failed");
	int i = 1;
	m_bsq->num_maps = -1;
	if (argc == 1) {
		//TODO finish STDIN handling
		m_bsq->num_maps++;
//		char * line = malloc(BUFSIZ*sizeof (char));
//		process_header(m_bsq, line, stdin);
//		printf("STDIN_FILENO: \n%d, %c, %c, %c\n", m_bsq->maps->lines, m_bsq->maps->empty, m_bsq->maps->obstacle, m_bsq->maps->full);
		if (m_bsq->maps->lines > 1 && parser(m_bsq, stdin)) {
			bsq_square square;
			square.size = 0; square.i = 0; square.j = 0;
			process_map(m_bsq, &square);
			printf("\n\n");
		} else {
			perror("provide map");
			free(m_bsq);
			return 1;
		}
	}
	while (i < argc) {
		m_bsq->num_maps++;
		int i = 0;
		FILE * fd = fopen(argv[i++], "r");
		if (parser(m_bsq, fd)) {
			bsq_square square;
			square.size = 0; square.i = 0; square.j = 0;
			process_map(m_bsq, &square);
			printf("\n\n");
		}
	}
	free(m_bsq);
	return 0;
}
