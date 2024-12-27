#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "lex.h"
#include "utils/debug.h"

int main(int argc, char **argv) {
        uint32_t i;
        const char *input_file = NULL, *output_file = "./a.out";
        char *file_contents;
        bool help = false, debug = false;
argument_parsing: {
        for(i = 1; i < argc; i++) {
                if(argv[i][0] == '-') switch(argv[i][1]) {
                case 'h': help = true; break;
                case 'o': dassert(
                                i < argc-1,
                                ERROR_LEVEL_ERROR,
                                "Opt-Error: ",
                                "-o expected argument "
                                "(--help for more info).\n"
                          );
                          output_file = argv[++i];
                          break;
                case '-':
                          if(!strcmp(argv[i]+2, "help")) help = true;
                          else if(!strcmp(argv[i]+2, "debug")) debug = true;
                          break;
                } else {
                        dassert(
                                !input_file,
                                ERROR_LEVEL_ERROR,
                                "Opt-Error: ",
                                "multiple input files, first is: "
                                "%s, second is: %s.",
                                input_file, argv[i]
                        );
                        input_file = argv[i];
                }
        }
        if(help) {
                printf("--== TAUSM ==--\n"
                       "Usage: %s [OPTIONS] <input file> [OPTIONS]\n"
                       "Options:\n"
                       "\t-h / --help :: open the help menu and exit.\n"
                       "\t-o <output file> :: set the output file to "
                        "<output file>.\n");
                exit(0);
        }
        dassert(
                input_file,
                ERROR_LEVEL_ERROR,
                "Opt-Error: ",
                "expected input file.\n"
        );
}
file_reading: {
        FILE *fp;
        uint32_t length;
        fp = fopen(input_file, "r");
        dassert(
                !!fp,
                ERROR_LEVEL_ERROR,
                "FS-Error: Could not open file `%s` for reading",
                input_file
        );
        fseek(fp, 0L, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        file_contents = calloc(1,length+1);
        fread(file_contents, 1, length, fp);
        fclose(fp);
        if(debug) eprintf(ERROR_LEVEL_INFO, "Debug (file read):\n", "%s\n", file_contents);
}
assembly: {
        TOKENS lexed_tokens;
lexical_analysis: {
        string_lex(&lexed_tokens, file_contents);
        if(debug) {
                print_with_error_level(ERROR_LEVEL_INFO, "Debug (lexed):\n");
                tokens_print(stderr, &lexed_tokens);
        }
}
        tokens_destroy(&lexed_tokens);
}
        return 0;
}
