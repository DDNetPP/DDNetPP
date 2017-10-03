//Simple script written by ChillerDragon
//This script is used for updating RIXP memberlist fetching it from ze web
//it makes no sense to use c++ i guess it would more sense to write it in bash directly but yolo...
#include <stdlib.h>
int main()
{
    while (1)
    {
        system("curl http://37.120.168.102/member.txt;sleep 180");
    }
}

