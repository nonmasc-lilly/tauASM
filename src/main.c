#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "lex.h"
#include "assemble.h"
#include "utils/debug.h"

struct options {
        const char *input_file;
        const char *output_file;
        bool help, debug;
};
enum file_error {
        FILE_ERROR_SUCCESS,
        FILE_ERROR_NO_OPEN
};


void parse_options(struct options *options, int argc, char **argv) {
        uint32_t i;
        memset(options, 0, sizeof(*options));
        options->output_file = "./a.out";
        for(i = 1; i < argc; i++) {
                if(argv[i][0] != '-') {
                        dassert(
                                !options->input_file,
                                ERROR_LEVEL_ERROR, "Opt-Error: ",
                                "Multiple input files, first being: %s, second: %s.\n",
                                options->input_file,
                                argv[i]
                        );
                        options->input_file = argv[i];
                        continue;
                }
                switch(argv[i][1]) {
                case 'h': options->help = true;
                case 'o':
                        dassert (
                                i < argc - 1,
                                ERROR_LEVEL_ERROR, "Opt-Error: ",
                                "-o expects argument (--help for more info).\n"
                        );
                        options->output_file = argv[++i];
                        break;
                case '-':
                        if(!strcmp(argv[i]+2, "help")) options->help = true;
                        else if(!strcmp(argv[i]+2, "debug")) options->debug = true;
                        break;
                }
        }
        dassert(
                options->input_file,
                ERROR_LEVEL_ERROR, "Opt-Error: ",
                "Expected input file.\n"
        );
}

#define VERSION "0.0.1 -- HEXMOD"
void help_menu(const char *argv0) {
        printf("--== tauASM %s ==--\n"
               "Usage: %s [OPTIONS] <input file> [OPTIONS]\n"
               "Options:\n"
               "\t-h / --help      :: Open this menu and exit.\n"
               "\t-o <output file> :: Set the file to output to.\n"
               "\t-d               :: Enable compiler debug messages.\n",
                VERSION, argv0
        );
        exit(0);
}

enum file_error read_file(const char *path, char **output) {
        FILE *fp;
        uint32_t file_length;
        *output = NULL;
        fp = fopen(path, "r");
        if(!fp) return FILE_ERROR_NO_OPEN;
        fseek(fp, 0L, SEEK_END);
        file_length = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        *output = calloc(1,file_length+1);
        fread(*output, 1, file_length, fp);
        fclose(fp);
        return FILE_ERROR_SUCCESS;
}

enum file_error write_buffer_to_file(const char *path, BYTE_BUFFER to_write) {
        FILE *fp;
        fp = fopen(path, "w");
        if(!fp) return FILE_ERROR_NO_OPEN;
        fwrite(to_write.buffer, 1, to_write.length, fp);
        fclose(fp);
}

/* TODO: CLEANUP CODEBASE FROM SHITTY CODING WRITTEN ON FUMES */
int main(int argc, char **argv) {
        uint32_t i;
        char *file_contents;
        struct options cmd_options;
        TOKENS lexed_tokens;
        BYTE_BUFFER output;
        
        parse_options(&cmd_options, argc, argv);
        if(cmd_options.help) help_menu(*argv);
        dassert(
                !read_file(cmd_options.input_file, &file_contents),
                ERROR_LEVEL_ERROR, "FS-Error: ",
                "Could not read file `%s`.\n",
                cmd_options.input_file
        );
        if(cmd_options.debug) eprintf(
                ERROR_LEVEL_INFO, "Debug (file read):\n",
                "%s\n", file_contents
        );
        string_lex(&lexed_tokens, file_contents);
        if(cmd_options.debug) {
                print_with_error_level(ERROR_LEVEL_INFO, "Debug(lexed):\n");
                tokens_print(stderr, &lexed_tokens);
        }
        tokens_assemble(&output, &lexed_tokens, cmd_options.debug);
        dassert(
                !write_buffer_to_file(cmd_options.output_file, output),
                ERROR_LEVEL_ERROR, "FS-Errror: ",
                "Could not write to `%s`.\n",
                cmd_options.output_file
        );

        free(output.buffer);
        tokens_destroy(&lexed_tokens);
        free(file_contents);
        return 0;
}
