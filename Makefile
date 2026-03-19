parse_json: main.c
	gcc main.c --std=c17 -Wall -Wextra -Wpedantic -Wconversion -pedantic-errors -fsanitize=address -o json_parser
