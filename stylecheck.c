#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>

#define MAXLEN 120

static void
check_file(int *had_err, const char *fn, int dos)
{
	FILE *fp;
	int ln_nbr;
	char ln[4096];
	int msgs = 0;
	int max_msgs = 20;
	int length;
	int position = 2;
	
	if ((fp = fopen (fn, "r")) == NULL) {
		perror (fn);
		*had_err = 1;
		return;
	}

	ln_nbr = 1;
	while (fgets (ln, sizeof(ln), fp) != NULL) {
		length = strlen(ln);
		/* max line lenght */
		if(length>=MAXLEN) {
			fprintf(stderr, "error: %s:%d: line too long:\n%s\n", fn, ln_nbr, ln);
			*had_err = 1;
			++msgs;
		}
		/* space at the beginning of line */
		if (ln[0] == ' ' && ln[1] != '*') {
			fprintf(stderr, "error: %s:%d: invalid indention (SP at column 1):"
				"\n%s\n", fn, ln_nbr, ln);
			*had_err = 1;
			++msgs;
		}
		/* whitespace at the end of the line */
		if(dos) {
			if(length > position && ln[length-1]=='\n' && ln[length-2]=='\r') {
				position = 3;
			}
			else {
				position = 2;
			}
		}
		if(length > position && isspace(ln[length-position])) {
			fprintf(stderr, "error: %s:%d: trailing whitespace at end of line:"
				"\n%s\n", fn, ln_nbr, ln);
			*had_err = 1;
			++msgs;
		}
		/* line number + msg limit */
		++ln_nbr;
		if(msgs == max_msgs) {
			fprintf(stderr, "reached max %d messages, not reporting more\n",
				max_msgs);
			goto done;
		}
	}

done:
	fclose (fp);
}

int
main(int argc, char *argv[])
{
	int had_err = 0;
	int opt = 0;
	int dos = 0;
	char *ignore = NULL;
	static struct option long_options[] = {
		{"ignore", required_argument, 0, 'i'},
		{"permit-dos-format", no_argument, 0, 'd'},
		{0, 0, 0, 0}
	};
	int long_index = 0;
	while((opt = getopt_long(argc, argv, "i:d", long_options, &long_index)) != -1) {
		switch(opt) {
		case 'i':
			ignore = optarg;
			break;
		case 'd':
			dos = 1;
			break;
		default:
			break;
		}
	}

	for(int i = optind; i < argc ; ++i) {
		if(ignore == NULL || strcmp(basename(argv[i]), ignore) != 0) {
			check_file(&had_err, argv[i], dos);
		}
	}
	return had_err;
}
