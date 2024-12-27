default_test="minima"
HELP=false
TEST="None"
CFLAGS="-std=c90 -Wpedantic -Wmissing-field-initializers"
DFLAGS="-g"

for i in $@; do
        [ "$i" =                "-h" ] && HELP=true
        [ "$i" =            "--help" ] && HELP=true
        [ "$i" =            "--test" ] && TEST="$default_test"
        [ "$i" =     "--test=minima" ] && TEST="minima"
        [ "$i" =       "--test=help" ] && TEST="help"
        [ "$i" = "--test=directives" ] && TEST="directives"
done

[ "$HELP" = true ] && {
        echo "Tausm -- make.sh";
        echo "||    Help    ||";
        echo "Usage: make.sh [OPTIONS]";
        echo "Options:";
        echo " --test=<which> :: run the test corresponding to <which>";
        echo "   which=["
        echo "     minima       :: Simply run tausm without arguments,";
        echo "     help         :: Run with the help argument,";
        echo "     directives   :: Check the basic directives using the directives file";
        echo "   ]"
        echo " -h / --help :: open this menu and exit";
        exit 0;
}

[ -d build ] || mkdir build

gcc -o build/tausm src/*.c src/utils/*.c $CFLAGS && (
        [ "$TEST" =     "minima" ] && build/tausm
        [ "$TEST" =       "help" ] && build/tausm --help
        [ "$TEST" = "directives" ] && build/tausm -o test/directives.bin \
                test/directives.asm --debug
)
