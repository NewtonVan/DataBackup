#include "backend.h"
#include "json_parser.h"
#include "m_server.h"

using namespace std;
int main(int argc, char *argv[])
{
    if (argc > 1){
        chdir(argv[1]);
    }
    MServer &sv = MServer::GetInstance();
    sv.Init(8081, &(BackEnd::Handle));
    cout<<"ok"<<endl;
    sv.StartServer();

    return 0;
}