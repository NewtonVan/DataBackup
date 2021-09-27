#include "backend.h"
#include "json_parser.h"
#include "m_server.h"

using namespace std;
int main()
{
    MServer &sv = MServer::GetInstance();
    sv.Init(9096, &(BackEnd::Handle));
    cout<<"ok"<<endl;
    sv.StartServer();

    return 0;
}