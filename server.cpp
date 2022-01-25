#include<iostream>
#include <unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<thread>
#include<vector>

#define SIZE 1024
#define PORT 9000

using namespace std;

struct client
{
    thread th;
    int client_socket;
    string name;
};

vector<client> clients;

void broadcast_message(char* message,int client_socket);
void set_name(char* name,int client_socket);
void end_connection(int client_socket);
void handle_client(int client_socket);

int main()
{
    int server_socket, client_socket, addr_len;

    // Create a socket
    server_socket  = socket(AF_INET, SOCK_STREAM, 0);
    if( (server_socket) == -1 )
    {
        cout<<"ERROR: socket failed";
        return -1;
    }

    //Bind the socket to a IP & port
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr_len = sizeof(addr);
    if( bind(server_socket, (struct sockaddr *)&addr, sizeof(addr) ) == -1 )
    {
        cout<<"Error: can't bind to IP/port";
        return -1;
    }
    
    //Mark the socket for listening in
    if( listen(server_socket, SOMAXCONN) == -1 )
    {
        cout<<"Error: can't listen";
        return -1;
    }

    cout<<"*****Welcome!! Server started*****"<<endl;
    while(true)
    {
        client_socket = accept(server_socket, (sockaddr*)&addr, (socklen_t*)&addr_len);
        if( client_socket == -1 )
        {
            cout<<"Error: can't connect to a client";
            return -1;
        }
        
        thread t(handle_client, client_socket);
        clients.push_back({move(t), client_socket, "Temp"});
        clients[clients.size()-1].th.detach();
    }

    close(server_socket);
    return 0;
}

// Broadcast message to all clients except the sender
void broadcast_message(char* message,int client_socket)
{
    for(int i=0;i<clients.size();i++)
    {
        // cout<<"broadcast"<<endl;
        if(clients[i].client_socket != client_socket)
        {
            // cout<<"broadcast: sender"<<endl;
            send(clients[i].client_socket, message, SIZE, 0);
        }
    }
}

// set name of the sender
void set_name(char* name,int client_socket)
{
    for(int i=0;i<clients.size();i++)
    {
        if(clients[i].client_socket == client_socket)
        {
            clients[i].name = name;
        }
    }
}

void end_connection(int client_socket)
{
    // Close the socket
    close(client_socket);

    for(int i=0;i<clients.size();i++)
    {
        if(clients[i].client_socket == client_socket)
        {
            clients.erase(clients.begin()+i);
            break;
        }
    }
}

void handle_client(int client_socket)
{
    char name[50];
    read(client_socket, name, 50);
    set_name(name, client_socket);

    char join_msg[70];
    memset(join_msg, 0, SIZE);
    strcat(join_msg, name);
    strcat(join_msg, " joined.");
    cout<<join_msg<<endl;
    broadcast_message(join_msg, client_socket);
    
    char buf[SIZE];
    while (true)
    {
        memset(buf, 0, SIZE);
        // Wait for client to send data
        int bytes_received = read(client_socket, buf, SIZE);
        if (bytes_received <= 0)
        {
            break;
        }

        //add name before message
        char msg[SIZE];
        memset(msg, 0, SIZE);
        strcat(msg, name);
        strcat(msg, ": ");
        strcat(msg, buf);

        if(strcmp(buf,"#exit")==0)
        {
            break;
        }
        broadcast_message(msg, client_socket);
        cout<<msg<<endl;
    }

    char leave_msg[70];
    memset(leave_msg, 0, SIZE);
    strcat(leave_msg, name);
    strcat(leave_msg, " left.");
    cout<<leave_msg<<endl;
    broadcast_message(leave_msg, client_socket);

    end_connection(client_socket);
}