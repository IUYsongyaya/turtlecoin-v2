#include <tools/cli_helper.h>

int main(int argc, char **argv)
{
    auto options = cli_setup_options(argv);

    auto cli = cli_parse_options(argc, argv, options);

    return 0;
}
