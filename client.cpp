#include<iostream>
#include <unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<thread>

#define SIZE 1024
#define PORT 9000

using namespace std;

bool exit_flag = false;
char name[50];
thread th_receive;

void send_message(int client_socket);
void receive_message(int client_socket);

int main()
{
    int client_socket, addr_len;
    // Create a socket
    client_socket  = socket(AF_INET, SOCK_STREAM, 0);
    if( (client_socket) == -1 )
    {
        cout<<"ERROR: socket failed";
        return -1;
    }

    //Bind the socket to a IP & port
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr_len = sizeof(addr);

    if( inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0 )
    {
        cout<<"Error: invalid address";
        return -1;
    }
    
    //connect to the server
    if (connect(client_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cout<<"Error: connection Failed";
        return -1;
    }

    cout<<"Enter your name: ";
    cin.getline(name, '\n');
    send(client_socket, name, sizeof(name) , 0);

    thread t1(send_message, client_socket);
	thread t2(receive_message, client_socket);
    th_receive = move(t2);
    
    if(t1.joinable())
		t1.join();
	if(t2.joinable())
		t2.join();

    // Close the socket
    close(client_socket);
 
    return 0;
}

void send_message(int client_socket)
{
    while(1)
    {
        char buf[SIZE];
        memset(buf, 0, SIZE);
        cout<<"you: ";
        cin.getline(buf,SIZE);
        if(strcmp(buf, "#exit")==0)
        {
            exit_flag = true;
            th_receive.detach();
            close(client_socket);
            cout<<"EXITING...."<<endl;
            return;
        }
        send(client_socket, buf, sizeof(buf), 0);
    }
}

void receive_message(int client_socket)
{
    while(1)
    {
        if(exit_flag)
            return;
        
        char buf[SIZE];
        memset(buf, 0, SIZE);
        // Wait for client to send data
        int bytes_received = read(client_socket, buf, SIZE);
        if (bytes_received <=0)
            continue;

        //remove "you: "
        char back_space=8;
        for(int i=0; i<5; i++)
        {
            cout<<back_space;
        }
 
        cout<<string(buf, 0, bytes_received) << endl;
        cout<<"you: "<<flush;
    }
}