#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

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
	char *ptr;
	short len;
	token_type_t type;
} token_t;

typedef struct {
	token_t *name;
	uint16_t offset;
} label_t;

char *input = NULL;
size_t input_size = 0;
token_t *tokens = NULL;
int num_tokens = 0;
int tokens_cap = 0;
uint8_t output[0x10000];
size_t output_size = 0;
label_t labels[256];
size_t num_labels = 0;

#define MN_HALT 0
#define MN_NOP  1
#define MN_J    2
#define MN_JT   3
#define MN_JF   4
#define MN_CALL 5

const char *gp_instr_mnemonic[] = {
	"halt", "nop", "j", "jt", "jf", "call", "ret",
	"inc", "dec", "shl", "shr", "ashr", "push", "pop",
};

const char *reg_instr_mnemonics[] = {
	"set", "add", "addc", "sub", "subc", "and", "or", "xor",
};

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
	char *p = input;
	
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
		//printf("token: '%.*s' %u\n", token.len, token.ptr, token.type);
	}
}

token_t end_token = {NULL, 0, T_END};

token_t *get_token(int pos) {
	if (pos >= num_tokens) {
		return &end_token;
	}

	return &tokens[pos];
}

int find_mnemonic_in_table(token_t *mnem, const char *table[], size_t table_size) {
	for (int i = 0; i < table_size; i++) {
		if (strncmp(mnem->name->ptr, table[i], mnem->name->len) == 0)
			return i;
	}

	return -1;
}

void assemble() {
	int token = 0;

	while (get_token(token)->type != T_END) {
		if (get_token(token)->type == T_IDENT && get_token(token + 1)->type == T_COLON) {
			label_t label;
			label.name = get_token(token++);
			token++;

			labels[num_labels++] = label;
			continue;
		}

		find_mnemonic_in_table(get_token(token), gp_instr_mnemonics, sizeof(gp_instr_mnemonics / 

	}

}

int main(int argc, char** argv) {
	if (argc <= 1) {
		printf("usage: asm <input file>\n");
		return 0;
	}

	FILE *file = fopen(argv[1], "r");
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
	assemble();
	
	return 0;
}
