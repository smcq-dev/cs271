/****************************************
 * C-ploration 4 for CS 271
 * 
 * [NAME] SEAN MCQUILLAN
 * [TERM] FALL 2025
 * 
 ****************************************/
#include "parser.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "symtable.h"
#include "error.h"
#include "hack.h"

/* Function: strip
 * -------------
 * remove whitespace and comments from a line
 *
 * s: the char* string to strip
 *
 * returns: the stripped char* string
 */
char *strip(char *s){	

	char s_new[strlen(s) + 1];
	int i = 0;
	
	for (char *s2 = s; *s2; s2++) { 
		if (*s2 == '/' && *(s2 + 1) == '/' ) {
			break;
		}

		else if(!isspace(*s2)){
			s_new[i++] = *s2;
		}
	}
	s_new[i] = '\0';
	strcpy(s, s_new);
	

    return s;	
}

/* Function: parse
 * -------------
 * iterate each line in the file and strip whitespace and comments. 
 *
 * file: pointer to FILE to parse
 *
 * returns: nothing
 */
int parse(FILE * file, instruction *instructions){
	
	char line[200] = {0};
	unsigned int line_num = 0;
	unsigned int instr_num = 0;
	instruction instr;

	add_predefined_symbols();
	while (fgets(line, sizeof(line), file)) {
		line_num++;
		
		if (instr_num > MAX_INSTRUCTIONS) {
    		exit_program(EXIT_TOO_MANY_INSTRUCTIONS, MAX_INSTRUCTIONS + 1);
		}
	
		strip(line);
		if (!*line) {
			continue;
		}
		char inst_type;
		if(is_Atype(line)) {
			//inst_type = 'A';
			if (!parse_A_instruction(line, &instr.inst.a)){
    			exit_program(EXIT_INVALID_A_INSTR, line_num, line);
 			}
 			instr.itype = A_type;
			if (instr.inst.a.is_addr) {
				printf("A: %d\n", instr.inst.a.operand.address);
			}
			else {
				printf("A: %s\n", instr.inst.a.operand.label);
			}
			//printf("%c  %s\n", inst_type, line);
		}

		if(is_label(line)) {
			inst_type = 'L';
			char label[200];
			extract_label(line, label);

			if(!isalpha(label[0])) {
				exit_program(EXIT_INVALID_LABEL, line_num, label);
			}
			if(symtable_find(label) != NULL) {
				exit_program(EXIT_SYMBOL_ALREADY_EXISTS, line_num, label);
			}
			symtable_insert(label, instr_num);
			//printf("%c  %s\n", inst_type, label);
			continue;
		}

		else if(is_Ctype(line)) {
			//inst_type = 'C';
			//printf("%c  %s\n", inst_type, line);
			char tmp_line[MAX_LINE_LENGTH];
			strcpy(tmp_line, line);
			parse_C_instruction(tmp_line, &instr.inst.c);
			if (instr.inst.c.comp == 127) {  
    			exit_program(EXIT_INVALID_C_COMP, line_num, line);
			}
			else if (instr.inst.c.dest == 15) {  
    			exit_program(EXIT_INVALID_C_DEST, line_num, line);
			}
			else if (instr.inst.c.jump == 15) {  
    			exit_program(EXIT_INVALID_C_JUMP, line_num, line);
			}
			instr.itype = C_type;
			printf("C: d=%d, c=%d, j=%d\n", instr.inst.c.dest, instr.inst.c.comp, instr.inst.c.jump);

		}

		//printf("%u: %c  %s\n", instr_num, inst_type, line);
		instructions[instr_num++] = instr;
    
    }
	return instr_num;
}

bool is_Atype(const char *line) {
	if(line[0] == '@'){
		return true;
	}
	else {
		return false;
	}
}

bool is_label(const char *line) {
	if(line[0] == '(' && line[strlen(line) - 1] == ')') {
		return true;
	}
	else {
		return false;
	}
}

bool is_Ctype(const char *line) {
	if(!is_label(line) && !is_Atype(line)) {
		return true;
	}
	else {
		return false;
	}
}

char *extract_label(const char *line, char* label) {
	int j = 0;
	for (int i=1; line[i] != '\0'; i++) {
		if (line[i] != ')') {
			label[j++] = line[i];
		}
		
	}

	label[j] = '\0';
	return label;

}

void add_predefined_symbols() {
	for(int i=0; i < NUM_PREDEFINED_SYMBOLS; i++) {
		predefined_symbol sym = predefined_symbols[i];
		symtable_insert(sym.name, sym.address);
	}
}

bool parse_A_instruction(const char *line, a_instruction *instr) {
	char* s = malloc(strlen(line));
	strcpy(s, line+1);
	char* s_end = NULL;
	long result = strtol(s, &s_end, 10);

	if(s == s_end) {
		instr->operand.label = malloc(strlen(line));
		strcpy(instr->operand.label, s);
		instr->is_addr = false;
		
	}

	else if(*s_end != 0) {
		return false;
	}

	else {
		instr->operand.address = result;
		instr->is_addr = true;
	}
	return true;

}

void parse_C_instruction(char *line, c_instruction *instr) {
	//printf("DEBUG parse_C_instruction: line = '%s'\n", line);
	char* temp1 = strtok(line, ";");
	char temp1_copy[MAX_LINE_LENGTH];
	strcpy(temp1_copy, temp1);
	char* jump_str = strtok(NULL, ";");

	char* temp2 = strtok(temp1_copy, "=");
	char* temp3 = strtok(NULL, "=");

	char* dest_str;
	char* comp_str;
	if (temp3 == NULL) {
    	dest_str = NULL;
    	comp_str = temp2;
	} else {
   
    	dest_str = temp2;
    	comp_str = temp3;
	}
	int a_bit;
	instr->comp = str_to_compid(comp_str, &a_bit);
	instr->a = a_bit;
	instr->jump = (jump_str == NULL) ? JMP_NULL : str_to_jumpid(jump_str);
	instr->dest = (dest_str == NULL) ? DEST_NULL : str_to_destid(dest_str);
	
}

void assemble(const char * file_name, instruction* instructions, int num_instructions){
	char hack_ending[] = ".hack";
	char file_name_temp[256];
	strcpy(file_name_temp, file_name);
	strcat(file_name_temp, hack_ending);

	FILE *fout = fopen(file_name_temp, "w");

	if (fout == NULL) {

	}

	int counter = 16;
	for(int i = 0; i < num_instructions; i++) {

		if(instructions[i].itype == A_type) {

			if (instructions[i].inst.a.is_addr == false) {
				char *label = instructions[i].inst.a.operand.label;
				if (symtable_find(label) == NULL ) {
					symtable_insert(label, counter);
					counter ++;
				}
				instructions[i].inst.a.operand.address = symtable_find(label)->addr;
				free(label);
				
			}

		}
		

		if(instructions[i].itype == C_type) {

		}

	}

	fclose(fout);

}


