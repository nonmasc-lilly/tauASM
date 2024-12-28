default_test="minima"
HELP=false
CLEAN=false
DEBUG="None"
TEST="None"
TFLAGS=""
CFLAGS="-std=c90 -Wpedantic -Wmissing-field-initializers"
DFLAGS="-g"
EDFLAGS=""

for i in $@; do
        [ "$i" =                "-h" ] && HELP=true
        [ "$i" =            "--help" ] && HELP=true
        [ "$i" =           "--clean" ] && CLEAN=true
        [ "$i" =            "--test" ] && TEST="$default_test"
        [ "$i" =     "--test=minima" ] && TEST="minima"
        [ "$i" =       "--test=help" ] && TEST="help"
        [ "$i" = "--test=directives" ] && TEST="directives"
        [ "$i" =     "--test=labels" ] && TEST="labels"
        [ "$i" =           "--edf-s" ] && EDFLAGS="$EDFLAGS -s"
        [ "$i" =      "--test-debug" ] && TFLAGS="$TFLAGS --debug"
        [ "$i" =           "--debug" ] && {
                DEBUG="crash";
                [ "$TEST" = "None" ] && TEST="$default_test";
        }
        [ "$i" = "--debug=crash" ] && {
                DEBUG="crash";
                [ "$TEST" = "None" ] && TEST="$default_test";
        }
        [ "$i" = "--debug=leaks" ] && {
                DEBUG="leaks";
                [ "$TEST" = "None" ] && TEST="$default_test";
        }
done

[ "$HELP" = true ] && {
        echo "Tausm -- make.sh";
        echo "||    Help    ||";
        echo "Usage: make.sh [OPTIONS]";
        echo "Options:";
        echo " --test=<which> :: Run the test corresponding to <which>.";
        echo "   which=["
        echo "     minima       :: Simply run tausm without arguments,";
        echo "     help         :: Run with the help argument,";
        echo "     directives   :: Check the basic directives using the directives file";
        echo "     default = $default_test"
        echo "   ]"
        echo " --debug=<which> :: Run tests with valgrind with the flags corresponding to"
        echo "     <which>, if test is None, then it is set to $default_test.";
        echo "   which=["
        echo "     crash        :: Valgrind without flags,";
        echo "     leaks        :: Valgrind with --leak-check=full"
        echo "   ]"
        echo " --test-debug     :: Add the --debug flag to the Test"
        echo " --edf-s          :: Add the \`-s\` flag to debug (unless --debug=<x>"
        echo "     isnt present)"
        echo " -h / --help :: Open this menu and exit.";
        exit 0;
}

[ -d build ] || mkdir build

[ "$DEBUG" = "crash" ] && gcc -o build/tausm src/*.c src/utils/*.c $CFLAGS $DFLAGS && (
        [ "$TEST" =     "minima" ] && valgrind $EDFLAGS build/tausm $TFLAGS
        [ "$TEST" =       "help" ] && valgrind $EDFLAGS build/tausm --help $TFLAGS
        [ "$TEST" =     "labels" ] && valgrind $EDFLAGS \
                build/tausm -o test/labels.bin test/labels.asm $TFLAGS
        [ "$TEST" = "directives" ] && valgrind $EDFLAGS \
                build/tausm -o test/directives.bin test/directives.asm $TFLAGS
)
[ "$DEBUG" = "leaks" ] && gcc -o build/tausm src/*.c src/utils/*.c $CFLAGS $DFLAGS && (
        [ "$TEST" =     "minima" ] && valgrind --leak-check=full $EDFLAGS \
                build/tausm
        [ "$TEST" =       "help" ] && valgrind --leak-check=full $EDFLAGS \
                build/tausm --help $TFLAGS
        [ "$TEST" =     "labels" ] && valgrind --leak-check=full $EDFLAGS \
                build/tausm -o test/labels.bin test/labels.asm $TFLAGS
        [ "$TEST" = "directives" ] && valgrind --leak-check=full \
                build/tausm -o test/directives.bin test/directives.asm $TFLAGS
)
[ "$DEBUG" = "None" ] && gcc -o build/tausm src/*.c src/utils/*.c $CFLAGS && (
        [ "$TEST" =     "minima" ] && build/tausm $TFLAGS
        [ "$TEST" =       "help" ] && build/tausm --help $TFLAGS
        [ "$TEST" =     "labels" ] && build/tausm -o test/labels.bin test/labels.asm $TFLAGS
        [ "$TEST" = "directives" ] && build/tausm -o test/directives.bin \
                test/directives.asm $TFLAGS
)

[ "$CLEAN" = true ] && compgen -G "vgcore.*" > /dev/null && rm vgcore.*
