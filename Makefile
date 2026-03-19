parse_json: main.c
	gcc main.c --std=c23 -Wall -Wextra -Wpedantic -Wconversion -pedantic-errors -fsanitize=address -o json_parser
