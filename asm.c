#include <stdio.h>
#include <stdlib.h>

char* input;
size_t input_size;

#define T_NONE    0
#define T_IDENT   1
#define T_COLON   2
#define T_NUM     3
#define T_NEWLINE 4
#define T_COMMA   5
#define T_OBRACK  6
#define T_CBRACK  7
#define T_END     8

typedef short token_type_t;

typedef struct {
	char* ptr;
	short len;
	token_type_t type;
} token_t;

token_t* tokens;
int num_tokens = 0;
int tokens_cap = 0;

void add_token(token_t t) {
	if (num_tokens >= tokens_cap) {
		if (tokens_cap == 0)
			tokens_cap = 256;

		tokens_cap *= 2;
		tokens = realloc(tokens, tokens_cap * sizeof(token_t));
	}

	tokens[num_tokens++] = t;
}

void tokenize() {
	token_t token;
	char* p = input;
	
	while (*p) {
		if (isblank(*p)) {
			p++;
			continue;
		}

		token.ptr = p;
		token.type = T_NONE;

		if (isalpha(*p)) {
			token.type = T_IDENT;
			p++;

			while (isalnum(*p) || *p == '_')
				p++;
		} else if (isdigit(*p)) {
			token.type = T_NUM;
			p++;

			while (isdigit(*p))
				p++;
		} else {
			switch (*p) {
			case ':':
				token.type = T_COLON;
				break;
			case ',':
				token.type = T_COMMA;
				break;
			case '\n':
				token.type = T_NEWLINE;
				break;
			case '[':
				token.type = T_OBRACK;
				break;
			case ']':
				token.type = T_CBRACK;
				break;
			}

			p++;
		}

		token.len = p - token.ptr;
		add_token(token);
		printf("token: '%.*s' %u\n", token.len, token.ptr, token.type);
	}
}

token_t end_token = {NULL, 0, T_END};
token_t* get_token(int pos) {
	if (pos >= num_tokens) {
		return &end_token;
	}

	return &tokens[pos];
}

void assemble() {
	int pos = 0;

	while (pos <= num_tokens) {
		
		if (get_token(0)->type == T_IDENT && get_token(1)->type == T_COLON) {
			
		}

	}

}

int main(int argc, char** argv) {
	FILE* file;
	int pos;
	if (argc <= 1) {
		printf("usage: asm <input file>\n");
		return 0;
	}

	file = fopen(argv[1], "r");
	if (!file) {
		printf("failed to open file '%s'\n", argv[1]);
		return 1;
	}

	fseek(file, 0, SEEK_END);
	input_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	input = malloc(input_size + 1);

	fread(input, 1, input_size, file);
	input[input_size] = 0;

	printf("input: %s\n", input);

	tokenize();
	
	return 0;
}
